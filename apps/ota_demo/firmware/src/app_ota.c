/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ota.c

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
#include <stdio.h>

/* This section lists the other files that are included in this file.
 */
#include "app_ota.h"
#include "system/console/sys_console.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/ota/sys_wincs_ota_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "config/sam_e54_xpro_wincs02/configuration.h"


// Global data for OTA application
static APP_OTA_CONTEXT g_appOtaCtx;

static SYS_WINCS_OTA_TLS_CFG_t g_otaTlsCfg =
{
    // Specify the peer authentication method
    .tlsPeerAuth          = SYS_WINCS_OTA_SERV_SOCK_PEER_AUTH,
    // Set the CA certificate for TLS
    .tlsCACertificate     = SYS_WINCS_OTA_SERV_SOCK_ROOT_CERT, 
    // Set the device certificate for TLS
    .tlsCertificate       = SYS_WINCS_OTA_SERV_SOCK_DEV_CERT, 
    // Set the key name for the device
    .tlsKeyName           = SYS_WINCS_OTA_SERV_SOCK_DEV_KEY,                                     
    // Set the password for the device key
    .tlsKeyPassword       = SYS_WINCS_OTA_SERV_SOCK_DEV_KEY_PWD,                                    
    // Set the server name for TLS
    .tlsServerName        = SYS_WINCS_OTA_SERV_SOCK_SERVER_NAME,
    // Set the domain name for TLS
    .tlsDomainName        = SYS_WINCS_OTA_SERV_SOCK_DOMAIN_NAME,
    // Enable or disable domain name verification
    .tlsDomainNameVerify  = SYS_WINCS_OTA_SERV_SOCK_DOMAIN_NAME_VERIFY
};


static void APP_OTA_CmdPrintUsage (void)
{
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n*********************** COMMAND HELP ************************************\r\n"TERM_RESET);
    SYS_CONSOLE_PRINT(">> Command     : ota \r\n");
    SYS_CONSOLE_PRINT(">> Description : Configure, start and stop OTA process \r\n");
    SYS_CONSOLE_PRINT(">> Options     : config     -> Configure the OTA properties\r\n"
                      "               : start      -> Start OTA process\r\n"
                      "               : stop       -> Stop OTA process\r\n"
                      "               : help       -> Help Command\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : ota config <URL> <TLS> \r\n"
            "\t\tURL : OTA FW binary URL \r\n "
            "\t\tTLS : 0- Non TLS Connection\r\n"
            "\t\t      1- TLS Connection\r\n");
    #ifdef OTA_EXTENDED_CONFIG_ENABLE
    SYS_CONSOLE_PRINT(">> Syntax      : ota config <URL> <TLS> <TIMEOUT>\r\n"
            "\t\tURL     : OTA FW binary URL \r\n "
            "\t\tTLS     : 0- Non TLS Connection\r\n"
            "\t\t          1- TLS Connection\r\n"
            "\t\tTIMEOUT : OTA timeout ");
    SYS_CONSOLE_PRINT("\tusage: ota tls config <ID> <VAL> \r\n");
    SYS_CONSOLE_PRINT("\tusage: ota tls config 1 <CA_CERT_NAME>\r\n"
                        "\tota tls config 2 <CERT_NAME>\r\n"
                        "\tota tls config 3 <PRI_KEY_NAME>\r\n"
                        "\tota tls config 4 <PRI_KEY_PASSWORD>\r\n"
                        "\tota tls config 5 <SERVER_NAME>\r\n"
                        "\tota tls config 6 <DOMAIN_NAME>\r\n"
                        "\tota tls config 7 <PEER_AUTH (0/1)>\r\n"
                        "\tota tls config 8 <PEER_DOMAIN_VERIFY (0/1)>\r\n");
    #endif
    SYS_CONSOLE_PRINT(">> Syntax      : ota start  -> To Start download and switch \r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : ota stop   -> To stop OTA process\r\n");
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n**************************************************************************\r\n"TERM_RESET);
    return;
}

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_CMDProcessing
//
// Summary:
//    Processes OTA commands received from the console.
//
// Description:
//    This function processes the OTA commands received from the console and
//    updates the OTA state accordingly.
//
// Parameters:
//    pCmdIO - Pointer to the command device node
//    argc - Argument count
//    argv - Argument vector
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_OTA_CMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    SYS_CONSOLE_PRINT(TERM_RESET"Command Received : ota\r\n");
    if ((argc < 2) && (argc > 4)){
        SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Invalid Number of parameters. Check \"ota help\" command\r\n" TERM_RESET);
        return;
    }

    if (strcmp(argv[1], "config") == 0){
        #ifdef OTA_EXTENDED_CONFIG_ENABLE
        if (argc == 5)
        {
            g_appOtaCtx.otaCfg.options.timeout = atoi(argv[4]);
        }
        else
        #endif
        {
            g_appOtaCtx.otaCfg.options.timeout = SYS_WINCS_OTA_TIMEOUT;
        }
            if (argc < 4){
            SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Invalid Number of parameters. Check \"ota help\" command\r\n" TERM_RESET);
            return;
        }
       
        strncpy(g_appOtaCtx.otaCfg.url, argv[2], strlen(argv[2]));
        if (strcmp(argv[3], "1") == 0){
            g_appOtaCtx.otaCfg.tlsEnable = 1;
        }
        else if (strcmp(argv[3], "0") == 0){
            g_appOtaCtx.otaCfg.tlsEnable = 0;
        }
        else{
            SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Command\r\n"TERM_RESET);
            SYS_CONSOLE_PRINT(TERM_RED "Invalid Command parameter <TLS_ENABLE>. "
                            "Check \"ota help\" command\r\n\r\n" TERM_RESET);
            return;
        }
        g_appOtaCtx.state = APP_OTA_STATE_CONFIG;
    }
#ifdef OTA_EXTENDED_CONFIG   
    else if ((strcmp(argv[1], "tls") == 0) && (strcmp(argv[2], "config") == 0)){
        uint8_t val = (uint8_t)atoi(argv[3]);
        switch(val)
        {
            case 1:
                strncpy(g_otaTlsCfg.tlsCACertificate, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 2:
                strncpy(g_otaTlsCfg.tlsCertificate, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 3:
                strncpy(g_otaTlsCfg.tlsKeyName, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 4:
                strncpy(g_otaTlsCfg.tlsKeyPassword, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 5:
                strncpy(g_otaTlsCfg.tlsServerName, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 6:
                strncpy(g_otaTlsCfg.tlsDomainName, argv[4], sizeof(argv[4]) - 1);
                break;
                
            case 7:
                if (strcmp(argv[3], "0") == 0) {
                    g_otaTlsCfg.tlsPeerAuth = true;
                } else if (strcmp(argv[3], "1") == 0) {
                    g_otaTlsCfg.tlsPeerAuth = false;
                } else {
                    APP_OTA_CmdPrintUsage();
                    return;
                }
                break;
                
            case 8:
                if (strcmp(argv[3], "0") == 0) {
                    g_otaTlsCfg.tlsDomainNameVerify = true;
                } else if (strcmp(argv[3], "1") == 0) {
                    g_otaTlsCfg.tlsDomainNameVerify = false;
                } else {
                    APP_OTA_CmdPrintUsage();
                    return;
                }
                break;
            
            default:
                APP_OTA_CmdPrintUsage();
                return;
        }
        g_appOtaCtx.state  = APP_OTA_STATE_CONFIG;
    }
#endif
    else if (strcmp(argv[1], "start") == 0){
        g_appOtaCtx.state = APP_OTA_STATE_DOWNLOAD_START;
    }
    
    else if (strcmp(argv[1], "stop") == 0){
        g_appOtaCtx.state = APP_OTA_STATE_STOP;
    }
    else if (strcmp(argv[1], "help") == 0){
        APP_OTA_CmdPrintUsage();
    }
    else{
        SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Wrong Command Usage. Check \"ota help\" command\r\n" TERM_RESET);
    }
    return;
}

// *****************************************************************************
// *****************************************************************************
// OTA Command Table
//
// Summary:
//    Defines the command table for OTA commands.
//
// Description:
//    This static constant array defines the command table for OTA commands,
//    mapping the "ota" command to the APP_OTA_CMDProcessing function.
//
// Remarks:
//    None
// *****************************************************************************
static const SYS_CMD_DESCRIPTOR OTACmdTbl[] =
{
    {"ota", APP_OTA_CMDProcessing, ": ota commands processing"},
};


// *****************************************************************************
// *****************************************************************************
// Function: SYS_WINCS_OTA_CallbackHandler
//
// Summary:
//    Handles OTA events and updates the application state.
//
// Description:
//    This function handles the OTA events and updates the application state
//    based on the event received.
//
// Parameters:
//    event - OTA event
//    otaHandle - OTA handle
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
static void SYS_WINCS_OTA_CallbackHandler 
( 
    SYS_WINCS_OTA_EVENT_t event, 
    SYS_WINCS_OTA_HANDLE_t otaHandle
)
{
    switch(event)
    {
        case SYS_WINCS_OTA_DOWNLOAD_STARTED:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA operation %d started\r\n", *(uint8_t *)otaHandle);
            SYS_CONSOLE_PRINT(TERM_YELLOW"[APP_OTA] : Downloading......\r\n"TERM_RESET);
            break;
        }

        case SYS_WINCS_OTA_DOWNLOAD_COMPLETE:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA FW Download Completed\r\n");
            g_appOtaCtx.state = APP_OTA_STATE_DOWNLOAD_DONE;
            break;
        }
        
        case SYS_WINCS_OTA_IMAGE_VERIFY:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : Verification Completed\r\n");
            g_appOtaCtx.state = APP_OTA_STATE_IMG_ACTIVATE;
            break;
        }
        
        case SYS_WINCS_OTA_NEW_PARTITION_ACTIVE:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : New partition active\r\n");
            g_appOtaCtx.state = APP_OTA_STATE_SWITCH_DONE;
            break;
        }

        case SYS_WINCS_OTA_BUSY:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA Busy\r\n");
            break;
        }

        case SYS_WINCS_OTA_INVALID_URL:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA Error! Invalid URL\r\n");
            break;
        }

        case SYS_WINCS_OTA_INSUFFICIENT_FLASH:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA Error! Insufficient Flash\r\n");
            break;
        }

        case SYS_WINCS_OTA_ERROR:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : OTA Error opId: %d\r\n",*(uint8_t *)otaHandle);
            break;
        }
    }
}

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_Initialize
//
// Summary:
//    Initializes the OTA application.
//
// Description:
//    This function initializes the OTA application and registers the OTA command
//    group.
//
// Parameters:
//    None
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_OTA_Initialize
(
    void
)
{
    if (!SYS_CMD_ADDGRP(OTACmdTbl, sizeof(OTACmdTbl)/sizeof(*OTACmdTbl), "ota", ": ota commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create OTA Commands\r\n");
    }
    g_appOtaCtx.state = APP_OTA_STATE_INIT;
}

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_Tasks
//
// Summary:
//    Manages the OTA application tasks.
//
// Description:
//    This function manages the OTA application tasks based on the current state.
//
// Parameters:
//    None
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_OTA_Tasks
(
    void
)
{
    switch( g_appOtaCtx.state)
    {
        // Initialize the OTA process
        case APP_OTA_STATE_INIT:
        {
            
            break;
        }
        
        case APP_OTA_STATE_CONFIG:
        {
            SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_SET_CALLBACK, SYS_WINCS_OTA_CallbackHandler);
            if(g_appOtaCtx.otaCfg.tlsEnable == 1 ) 
            {
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_OPEN_TLS_CTX,NULL);
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_GET_TLS_CTX_HANDLE,(SYS_WINCS_NET_HANDLE_t)&g_appOtaCtx.otaCfg.tlsCtxHandle);
                
                #ifdef OTA_EXTENDED_CONFIG_ENABLE
                if(g_appOtaCtx.otaCfg.tlsCtxHandle == WDRV_WINC_TLS_INVALID_HANDLE)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :ERROR OTA Setting Options \r\n");
                    g_appOtaCtx.state = APP_OTA_STATE_ERROR;
                }
                
                if(g_otaTlsCfg.tlsCACertificate == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :CA Certificate not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsCACertificate = SYS_WINCS_OTA_ROOT_CERT;
                }
                
                if(g_otaTlsCfg.tlsCertificate == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :Device Certificate not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsCertificate = SYS_WINCS_OTA_DEV_CERT;
                }
                
                if(g_otaTlsCfg.tlsKeyName == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :Key not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsKeyName = SYS_WINCS_OTA_DEV_KEY;
                }
                
                if(g_otaTlsCfg.tlsKeyPassword == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :Key Password not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsKeyPassword = SYS_WINCS_OTA_DEV_KEY_PWD;
                }
                
                if(g_otaTlsCfg.tlsServerName == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :Server not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsServerName = SYS_WINCS_OTA_SERVER_NAME;
                }
                
                if(g_otaTlsCfg.tlsDomainName == NULL)
                {
                    SYS_CONSOLE_PRINT("[APP_OTA] :Domain name not configured. using default %s \r\n",SYS_WINCS_OTA_ROOT_CERT);
                    g_otaTlsCfg.tlsDomainName = SYS_WINCS_OTA_DOMAIN_NAME;
                }
                
//                g_otaTlsCfg.tlsPeerAuth = SYS_WINCS_OTA_PEER_AUTH;
//                g_otaTlsCfg.tlsDomainNameVerify = SYS_WINCS_OTA_DOMAIN_NAME_VERIFY;
                #endif
                
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_TLS_CONFIG,(SYS_WINCS_NET_HANDLE_t)&g_otaTlsCfg);
            }
            else
            {
                SYS_CONSOLE_PRINT("[APP_OTA] : g_appOtaCtx.otaCfg.tlsCtxHandle 0\r\n");
                g_appOtaCtx.otaCfg.tlsCtxHandle = 0;
            }
            
            
            if(SYS_WINCS_PASS != SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_OPTIONS_SET, &g_appOtaCtx.otaCfg.options))
            {
                SYS_CONSOLE_PRINT("[APP_OTA] :ERROR OTA Setting Options \r\n");
                g_appOtaCtx.state = APP_OTA_STATE_ERROR;
            }
            SYS_CONSOLE_PRINT(TERM_GREEN"[APP_OTA] : OTA Configurations Set Successfully\r\n"TERM_RESET,g_appOtaCtx.otaCfg.options.timeout);
            g_appOtaCtx.state = APP_OTA_STATE_WAIT;
            break;
        }

        // Start the OTA download process
        case APP_OTA_STATE_DOWNLOAD_START:
        {
            if('\0' != g_appOtaCtx.otaCfg.url[0])
            {
                if(true == APP_WINCS02_GetWifiStatus())
                {
                    if(g_appOtaCtx.otaCfg.tlsEnable == 1 ) 
                    {
                        if(false == APP_WINCS02_GetSntpStatus())
                        {
                            SYS_CONSOLE_PRINT("[APP_OTA] :ERROR SNTP not UP. Please try again after SNTP is UP\r\n \r\n");
                            g_appOtaCtx.state = APP_OTA_STATE_ERROR;
                            break;
                        }

                    }
                    if(SYS_WINCS_PASS != SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_DOWNLOAD_START, &g_appOtaCtx.otaCfg))
                    {
                        SYS_CONSOLE_PRINT("[APP_OTA] :ERROR OTA Download \r\n");
                    }
                }
                else
                {
                    SYS_CONSOLE_PRINT(TERM_RED"[APP_OTA] :ERROR - Wifi Not connected\r\n"TERM_RESET);
                    SYS_CONSOLE_PRINT(TERM_YELLOW"Connect to Wi-Fi using sta command and try again\r\n"TERM_RESET);
                    g_appOtaCtx.state = APP_OTA_STATE_ERROR;
                    break;
                }
            }
            else
            {
                SYS_CONSOLE_PRINT(TERM_RED"[APP_OTA] :ERROR - OTA configurations not set\r\n"TERM_RESET);
                SYS_CONSOLE_PRINT(TERM_YELLOW"Set OTA configuration and try again\r\n"TERM_RESET);
                g_appOtaCtx.state = APP_OTA_STATE_ERROR;
            }
            g_appOtaCtx.state = APP_OTA_STATE_WAIT;
            break;
        }

        // Handle the completion of the OTA download
        case APP_OTA_STATE_DOWNLOAD_DONE:
        {
            SYS_CONSOLE_PRINT("[APP_OTA] : Download Completed\r\n");
            SYS_CONSOLE_PRINT("[APP_OTA] : Now switching to active partition...\r\n");
            
            if(g_appOtaCtx.otaCfg.tlsEnable == 1 ) 
            {
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_CLOSE_TLS_CTX,NULL);
            }
            
            if (SYS_WINCS_PASS != SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_IMG_ACTIVATE, NULL))
            {
                SYS_CONSOLE_PRINT("[APP_OTA] : OTA Error\r\n");
                g_appOtaCtx.state = APP_OTA_STATE_ERROR;
                break;
            }

            g_appOtaCtx.state = APP_OTA_STATE_WAIT;
            break;
        }
        
        // Verify the downloaded image
        case APP_OTA_STATE_IMG_ACTIVATE:
        {
            if (SYS_WINCS_PASS != SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_IMG_ACTIVATE, NULL))
            {
//                SYS_CONSOLE_PRINT("[APP_OTA] : OTA Error\r\n");
                g_appOtaCtx.state = APP_OTA_STATE_IMG_ACTIVATE;
                break;
            }
            g_appOtaCtx.state = APP_OTA_STATE_WAIT;
            break;
        }
        
        // Handle errors in the OTA process
        case APP_OTA_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[ERROR_OTA] :ERROR OTA Update \r\n"TERM_RESET);
            g_appOtaCtx.state = APP_OTA_STATE_WAIT;
            break;
        }

        // Wait state for the OTA process
        case APP_OTA_STATE_WAIT:
        {
            break;
        }

        // Handle the completion of the partition switch
        case APP_OTA_STATE_SWITCH_DONE:
        {
            SYS_CONSOLE_PRINT(TERM_GREEN"[APP_OTA] : OTA Successfully Completed\r\n"TERM_RESET);
            g_appOtaCtx.state = APP_OTA_STATE_STOP;
            break;
        }

        // Stop the OTA process
        case APP_OTA_STATE_STOP:
        {
            break;
        }
    }
    return;
}
