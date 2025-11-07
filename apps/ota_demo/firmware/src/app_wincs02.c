/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wincs02.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files
 ************************************************************************** */

/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

/* This section lists the other files that are included in this file.
 */
#include "app_wincs02.h"
#include "configuration.h"
#include "system/system_module.h"
#include "system/console/sys_console.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "system/sys_wincs_system_service.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/mqtt/sys_wincs_mqtt_service.h"
#include "system/wifiprov/sys_wincs_provision_service.h"
#include "config/sam_e54_xpro_wincs02/driver/driver_common.h"
#include "app_provision.h"
#include "app_azure.h"
#include "app_ota.h"



#define APP_WINCS02_MAX_SSID_SIZE 30
#define APP_WINCS02_MAX_PASS_SIZE 30
#define APP_WINCS02_MAX_REGDOMAIN_SIZE 5


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

typedef struct
{
    /* The application's current state */
    APP_WINCS02_STATES              state;
    
    char                            regDomain[APP_WINCS02_MAX_REGDOMAIN_SIZE];

    DRV_HANDLE                      wdrvHandle;
    
    WDRV_WINC_TLS_HANDLE            tlsHandle;
    
    char                            ssid[APP_WINCS02_MAX_SSID_SIZE];
    
    char                            passphrase[APP_WINCS02_MAX_PASS_SIZE]; 
    
 } APP_WINCS02_CONTEXT;
 
 
 typedef struct
{
    bool                            wifiConnected;
    
    bool                            sntpUp;
    
    bool                            mqttConnected;
    
    bool                            regDomainSet;
    
 } APP_WINCS02_DEV_STATUS;

 static APP_WINCS02_DEV_STATUS g_appDevStatus = 
{
    .wifiConnected    = false,
    .mqttConnected    = false,
    .sntpUp           = false,
    .regDomainSet     = false,
};

 static APP_WINCS02_CONTEXT g_appWincsCtx;
// Application data

// Wi-Fi station configuration parameters
static SYS_WINCS_WIFI_PARAM_t g_wifiStaCfg;


bool APP_WINCS02_GetWifiStatus( void ) {
    return g_appDevStatus.wifiConnected;
}

bool APP_WINCS02_GetMqttStatus ( void ) {
    return g_appDevStatus.mqttConnected;
}

bool APP_WINCS02_GetSntpStatus ( void ) {
    return g_appDevStatus.sntpUp;
}

bool APP_WINCS02_GetRegDomainStatus ( void ) {
    return g_appDevStatus.regDomainSet;
}

char * APP_WINCS02_GetRegDomain ( void ) {
    return g_appWincsCtx.regDomain;
}

void APP_WINCS02_SetRegDomain ( const char *regDomain ) {
    strncpy((char *)g_appWincsCtx.regDomain, regDomain, strlen(regDomain));
}

static void APP_WINCS02_CmdPrintUsage (void)
{
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n*********************** COMMAND HELP ************************************\r\n"TERM_RESET);
    SYS_CONSOLE_PRINT(">> Command     : sta \r\n");
    SYS_CONSOLE_PRINT(">> Description : Configure, connect and disconnect of AP \r\n");
    SYS_CONSOLE_PRINT(">> Options     : config     -> Configure the AP Credentials\r\n"
                      "               : connect    -> Connect to AP\r\n"
                      "               : disconnect -> Disconnect from AP\r\n"
                      "               : help       -> Help Command\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : sta config <REG_DOMAIN> <SSID> <SECURITY> \r\n"
                "\t\t\tREG_DOMAIN : GEN, USA, EMEA\r\n "
                "\t\t\tSECURITY   : 0-Open\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : sta config <REG_DOMAIN> <SSID> <SECURITY> <PASSWORD>\r\n"
                "\t\t\tREG_DOMAIN : GEN, USA, EMEA\r\n "
                "\t\t\tSECURITY   : 2-WPA2 Mixed, 3-WPA2, 4-WPA3 Trans, 5-WPA3\r\n"
                "\t\t\tPASSWORD   : Password \r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : sta connect\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : sta disconnect\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : sta help\r\n");
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n**************************************************************************\r\n"TERM_RESET);
    return;
}
// *****************************************************************************
// *****************************************************************************
// Command Table for STA
//
// Summary:
//    Defines the command table for the STA commands.
//
// Description:
//    This table defines the commands that can be processed by the STA
//    application.
//
// Remarks:
//    None
// *****************************************************************************
static const SYS_CMD_DESCRIPTOR staCmdTbl[] =
{
    {"sta",    APP_WINCS02_StaCmdProcessing,             ": sta commands processing"},
};

// *****************************************************************************
// Function: APP_WINCS02_StaCmdProcessing
//
// Summary:
//    Processes STA commands.
//
// Description:
//    This function processes commands related to the STA application. It takes
//    the command input, parses it, and performs the necessary actions.
//
// Parameters:
//    pCmdIO - Pointer to the command device node.
//    argc - Number of arguments.
//    argv - Array of arguments.
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_WINCS02_StaCmdProcessing (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    SYS_CONSOLE_MESSAGE(TERM_RESET "\r\nCommand received : sta\r\n");

    if ((argc < 2) && (argc > 6))
    {
        SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Invalid Number of parameters. Check \"sta help\" command\r\n" TERM_RESET);
        return;
    }
    if (strcmp(argv[1], "config") == 0)
    {
        if (argc < 5)
        {
            SYS_CONSOLE_MESSAGE(TERM_RED "Invalid Number of parameters. Check \"sta help\" command\r\n" TERM_RESET);
            return;
        }
        
        if ((0 == strcmp(argv[2], "GEN")) || (0 == strcmp(argv[2], "USA")) ||
                (0 == strcmp(argv[2], "EMEA")))
        {
            strncpy((char *)g_appWincsCtx.regDomain, argv[2], strlen(argv[2]));
            strncpy((char *)g_appWincsCtx.ssid, argv[3], strlen(argv[3]) );
            g_wifiStaCfg.ssid = &g_appWincsCtx.ssid[0];
            
            int security = atoi(argv[4]);
            
            switch (security){
                case 0:
                    g_wifiStaCfg.security = SYS_WINCS_WIFI_SECURITY_OPEN;
                    break;
                
                case 2:
                    g_wifiStaCfg.security = SYS_WINCS_WIFI_SECURITY_WPA2_MIXED;
                    break;
                
                case 3:
                    g_wifiStaCfg.security = SYS_WINCS_WIFI_SECURITY_WPA2;
                    break;
                
                case 4:
                    g_wifiStaCfg.security = SYS_WINCS_WIFI_SECURITY_WPA3_TRANS;
                    break;
                    
                case 5:
                    g_wifiStaCfg.security = SYS_WINCS_WIFI_SECURITY_WPA3;
                    break;
                    
                default:
                    SYS_CONSOLE_PRINT(TERM_RED "Invalid Command parameter <SECURITY>. "
                            "Check \"sta help\" command\r\n\r\n" TERM_RESET);
                    return;
            }
            
            if (0 != g_wifiStaCfg.security){
                if (argc < 6){
                    SYS_CONSOLE_PRINT(TERM_RED "Invalid Number of parameter. Check \"sta help\" command\r\n\r\n" TERM_RESET);
                    return;
                }
                strncpy(g_appWincsCtx.passphrase, argv[5], strlen(argv[5]) );
                g_wifiStaCfg.passphrase = &g_appWincsCtx.passphrase[0];
            }
            
            g_appWincsCtx.state = APP_STATE_WINCS_SET_REG_DOMAIN;
        }
        else{
            g_appWincsCtx.regDomain[0] = '\0';
            SYS_CONSOLE_PRINT(TERM_RED "Invalid Command parameter <REGDOMAIN>. Check \"sta help\" command\r\n\r\n" TERM_RESET);
        }
    }
    else if (strcmp(argv[1], "connect") == 0){
        g_appWincsCtx.state = APP_STATE_WINCS_WIFI_CONNECT;
    }
    else if (strcmp(argv[1], "disconnect") == 0){
        g_appWincsCtx.state = APP_STATE_WINCS_WIFI_DISCONNECT;
    }
#if STA_EXTENDED_CONFIG_ENABLE
    else if (strcmp(argv[1], "cert") == 0)
    {
        g_appWincsCtx.state = APP_STATE_WINCS_PRINT_CERTS;
    }
    else if (strcmp(argv[1], "key") == 0)
    {
        g_appWincsCtx.state = APP_STATE_WINCS_PRINT_KEYS;
    }
#endif
    else if (strcmp(argv[1], "help") == 0){
        APP_WINCS02_CmdPrintUsage();
    }
    else
    {
        SYS_CONSOLE_PRINT(TERM_RED "Invalid Command parameter. Check \"sta help\" command\r\n\r\n" TERM_RESET);
    }
    return;
}

// *****************************************************************************
// Application Wi-Fi Callback Handler
//
// Summary:
//    Handles Wi-Fi events.
//
// Description:
//    This function handles various Wi-Fi events and performs appropriate actions.
//
// Parameters:
//    event - The type of Wi-Fi event
//    wifiHandle - Handle to the Wi-Fi event data
//
// Returns:
//    None.
//
// Remarks:
//    None.
// *****************************************************************************
void SYS_WINCS_WIFI_CallbackHandler
(
    SYS_WINCS_WIFI_EVENT_t event,         // The type of Wi-Fi event
    SYS_WINCS_WIFI_HANDLE_t wifiHandle    // Handle to the Wi-Fi event data
)
{
            
    switch(event)
    {
        /* Set regulatory domain Acknowledgment */
        case SYS_WINCS_WIFI_REG_DOMAIN_SET_ACK:
        {
            // The driver generates this event callback twice, hence the if condition 
            // to ignore one more callback. This will be resolved in the next release.
            static bool domainFlag = false;
            if( domainFlag == false)
            {
                SYS_CONSOLE_PRINT("[APP] : Set Reg Domain ->"TERM_GREEN" SUCCESS\r\n"TERM_RESET);
                if((APP_PROV_STATE_SET_REG_DOMAIN == APP_PROV_GetProvStatus())
                        || (APP_PROV_STATE_WAIT == APP_PROV_GetProvStatus()))
                {
                    g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
                }
                else
                {
                    g_appWincsCtx.state = APP_STATE_WINCS_SET_WIFI_PARAMS;
                }
                domainFlag = true;
                g_appDevStatus.regDomainSet = true;
            }
            
            break;
        }  
        
        /* SNTP UP event code*/
        case SYS_WINCS_WIFI_SNTP_UP:
        {            
            SYS_CONSOLE_PRINT(TERM_YELLOW"[APP] : SNTP UP \r\n"TERM_RESET);
            g_appDevStatus.sntpUp = true;
            break;
        }

        /* Wi-Fi connected event code*/
        case SYS_WINCS_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT(TERM_GREEN"[APP] : Wi-Fi Connected    \r\n"TERM_RESET);
            g_appDevStatus.wifiConnected = true;
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_WINCS_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[APP] : Wi-Fi Disconnected \r\n"TERM_RESET);
            g_appDevStatus.wifiConnected = false;
            break;
        }
        
        /* Wi-Fi DHCP IPv4 complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV4_COMPLETE:
        {         
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv4 : %s\r\n", (uint8_t *)wifiHandle);
            
            // Request the current time from the Wi-Fi service controller
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_TIME, NULL);
            break;
        }
        
        /* Wi-Fi DHCP IPv6 local complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Local : %s\r\n", (uint8_t *)wifiHandle);
            break;
        }
        
        /* Wi-Fi DHCP IPv6 global complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Global: %s\r\n", (uint8_t *)wifiHandle);
            break;
        }
        
        case SYS_WINCS_WIFI_CONNECT_FAILED:
        {
            SYS_CONSOLE_PRINT("[APP] : Wi-Fi Connection Failed\nRetrying\r\n");
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
        }
        
        default:
        {
            break;
        }
    }    
}


void APP_WINCS02_StaCmdInitialize
(
    void
)
{
    if (!SYS_CMD_ADDGRP(staCmdTbl, sizeof(staCmdTbl)/sizeof(*staCmdTbl), "sta", ": sta commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create STA Commands\r\n");
    }
    return;
}
// *****************************************************************************
// Application Initialization Function
//
// Summary:
//    Initializes the application.
//
// Description:
//    This function initializes the application's state machine and other
//    parameters.
//
// Parameters:
//    None.
//
// Returns:
//    None.
//
// Remarks:
//    None.
// *****************************************************************************
void APP_WINCS02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    g_appWincsCtx.state = APP_STATE_WINCS_PRINT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    APP_WINCS02_StaCmdInitialize();
    
    APP_PROV_Initialize(); 

    APP_AZURE_Initialize();
    
    APP_OTA_Initialize();
    
}


// *****************************************************************************
// Application Tasks Function
//
// Summary:
//    Executes the application's tasks.
//
// Description:
//    This function implements the application's state machine and performs
//    the necessary actions based on the current state.
//
// Parameters:
//    None.
//
// Returns:
//    None.
//
// Remarks:
//    None.
// *****************************************************************************
void APP_WINCS02_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( g_appWincsCtx.state )
    {
        case APP_STATE_WINCS_PRINT:
        {
            SYS_CONSOLE_PRINT(TERM_YELLOW"########################################\r\n"TERM_RESET);
            SYS_CONSOLE_PRINT(TERM_CYAN"  WINCS02 RED CS Certification Application\r\n"TERM_RESET);
            SYS_CONSOLE_PRINT(TERM_YELLOW"########################################\r\n"TERM_RESET);
            
            g_appWincsCtx.state = APP_STATE_WINCS_INIT;
            break;
        }
        
        /* Application's initial state. */
        case APP_STATE_WINCS_INIT:
        {
            SYS_STATUS status;
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_STATUS, &status);

            
            if (SYS_STATUS_READY == status)
            {
                g_appWincsCtx.state = APP_STATE_WINCS_OPEN_DRIVER;
            }
            
            break;
        }
        
        case APP_STATE_WINCS_OPEN_DRIVER:
        {
            // Open the Wi-Fi driver
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_OPEN_DRIVER, &g_appWincsCtx.wdrvHandle))
            {
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }

            // Get the driver handle
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE, &g_appWincsCtx.wdrvHandle);
            
            SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_DEBUG_UART_SET, NULL);
            
            g_appWincsCtx.state = APP_STATE_WINCS_DEVICE_INFO;
            break;
        }
        
        case APP_STATE_WINCS_DEVICE_INFO:
        {
            APP_DRIVER_VERSION_INFO drvVersion;
            APP_FIRMWARE_VERSION_INFO fwVersion;
            APP_DEVICE_INFO devInfo;
            SYS_WINCS_RESULT_t status = SYS_WINCS_BUSY;

            // Get the firmware version
            status = SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_SW_REV, &fwVersion);

            if(status == SYS_WINCS_PASS)
            {
                // Get the device information
                status = SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_DEV_INFO, &devInfo);
            }

            if(status == SYS_WINCS_PASS)
            {
                // Get the driver version
                status = SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_DRIVER_VER, &drvVersion);
            }

            if(status == SYS_WINCS_PASS)
            {
                char buff[30];
                // Print device information
                SYS_CONSOLE_PRINT("WINC: Device ID = %08x\r\n", devInfo.id);
                for (int i = 0; i < devInfo.numImages; i++)
                {
                    SYS_CONSOLE_PRINT("%d: Seq No = %08x, Version = %08x, Source Address = %08x\r\n", 
                            i, devInfo.image[i].seqNum, devInfo.image[i].version, devInfo.image[i].srcAddr);
                }

                // Print firmware version
                SYS_CONSOLE_PRINT(TERM_CYAN "Firmware Version: %d.%d.%d ", fwVersion.version.major,
                        fwVersion.version.minor, fwVersion.version.patch);
                strftime(buff, sizeof(buff), "%X %b %d %Y", localtime((time_t*)&fwVersion.build.timeUTC));
                SYS_CONSOLE_PRINT(" [%s]\r\n", buff);

                // Print driver version
                SYS_CONSOLE_PRINT("Driver Version: %d.%d.%d\r\n"TERM_RESET, drvVersion.version.major, 
                        drvVersion.version.minor, drvVersion.version.patch);
                
            
                // Set the callback handler for Wi-Fi events
                SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_CALLBACK, SYS_WINCS_WIFI_CallbackHandler);
            
                SYS_CONSOLE_PRINT("SNTP address : %s\r\n",SYS_WINCS_WIFI_SNTP_ADDRESS);
                char sntp_url[] =  SYS_WINCS_WIFI_SNTP_ADDRESS;
                if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_SNTP, sntp_url))
                {
                    g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                    break;
                }
            
                SYS_CONSOLE_PRINT(TERM_GREEN"\r\nNow you can send a command or use HELP for help.\r\n"TERM_RESET);
                g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_WINCS_SET_REG_DOMAIN:
        {
            static bool flag = false;
            // Set the callback handler for Wi-Fi events
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_CALLBACK, SYS_WINCS_WIFI_CallbackHandler);
            
            if(false == flag)
            {
                SYS_CONSOLE_PRINT(TERM_YELLOW"[APP] : Setting REG domain to " TERM_UL "%s\r\n"TERM_RESET ,SYS_WINCS_WIFI_COUNTRYCODE);
                flag = true;
            }
            // Set the regulatory domain
            SYS_WINCS_RESULT_t status = SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_REG_DOMAIN, SYS_WINCS_WIFI_COUNTRYCODE);
            if (SYS_WINCS_FAIL == status)
            {
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            else if(SYS_WINCS_BUSY == status)
            {
                g_appWincsCtx.state = APP_STATE_WINCS_SET_REG_DOMAIN;
                break;
            }
            else
            {
                
                if(g_appWincsCtx.state != APP_STATE_WINCS_SET_WIFI_PARAMS)
                {
                    SYS_CONSOLE_PRINT("%s %d\r\n",__FUNCTION__,__LINE__);
                    g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
                    break;
                }
            }
                break;
        }
        
        case APP_STATE_WINCS_PRINT_CERTS:
        {
            SYS_CONSOLE_PRINT("[APP] : Certificates on Device :-\r\n"TERM_YELLOW);
//            SYS_WINCS_SYSTEM_setTaskStatus(false);
            if (SYS_WINCS_FAIL == SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_GET_CERT_LIST,NULL))
            {
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            g_appWincsCtx.state = APP_STATE_WINCS_WAIT;
            break;
        }
        
        case APP_STATE_WINCS_PRINT_KEYS:
        {
            SYS_CONSOLE_PRINT("[APP] : Keys on Device :-\r\n"TERM_YELLOW);
//            SYS_WINCS_SYSTEM_setTaskStatus(false);
            
            if (SYS_WINCS_FAIL == SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_GET_KEY_LIST,NULL))
            {
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            g_appWincsCtx.state = APP_STATE_WINCS_WAIT;
            break;
        }
        
        case APP_STATE_WINCS_SET_WIFI_PARAMS:
        {    
           
            
            // Configuration parameters for Wi-Fi station mode
            g_wifiStaCfg.mode        = SYS_WINCS_WIFI_MODE_STA;        // Set Wi-Fi mode to Station (STA)
            g_wifiStaCfg.autoConnect = false;                           // Enable or disable auto-connect to the Wi-Fi network
            
            SYS_CONSOLE_PRINT("\r\n\r\n[APP] : Wi-Fi Configurations Set\r\n"
                              "\t\tREG DOMAIN : %s\r\n"
                              "\t\tSSID       : %s\r\n"
                              "\t\tSECURITY   : %d\r\n"
                              "\t\tPASSWORD   : %s\r\n",g_appWincsCtx.regDomain,
                              g_wifiStaCfg.ssid,g_wifiStaCfg.security,g_wifiStaCfg.passphrase);
            // Set the Wi-Fi parameters
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &g_wifiStaCfg))
            {
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_WINCS_WIFI_CONNECT:
        {
            // Connect to AP
            SYS_CONSOLE_PRINT("\r\n\r\n[APP] : Wi-Fi Connecting to AP : %s \r\n",g_wifiStaCfg.ssid);
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL))
            {
                SYS_CONSOLE_PRINT(TERM_RED"[APP_ERROR] : ERROR connecting to AP\r\n");
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_WINCS_WIFI_DISCONNECT:
        {
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_DISCONNECT,NULL))
            {
                SYS_CONSOLE_PRINT(TERM_RED"[APP_ERROR] : ERROR \r\n");
                g_appWincsCtx.state = APP_STATE_WINCS_ERROR;
            }
            SYS_CONSOLE_PRINT("[APP] : Wifi disconnected\r\n");
            g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_WINCS_SERVICE_TASKS:
        {
            break;
        }
        
        case APP_STATE_WINCS_WAIT:
        {
            break;
        }
        
        case APP_STATE_WINCS_ERROR:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[APP_ERROR] : ERROR in Application "TERM_RESET);
            g_appWincsCtx.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }

    }
    
    APP_PROV_Tasks();
    
    APP_AZURE_Tasks();
    
    APP_OTA_Tasks();
    
    return;
}
/* ************************************************************************** */
/* *****************************************************************************
 End of File
 */
