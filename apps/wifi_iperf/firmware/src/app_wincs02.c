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
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <time.h>

#include "app_wincs02.h"
#include "system/system_module.h"
#include "system/console/sys_console.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "system/sys_wincs_system_service.h"
#include "system/net/sys_wincs_net_service.h"
#include "iperf/iperf.h"

#define MAX_PACKET_LEN  1452    /* ipv4 packet len */
static bool isConnect = false;
static WDRV_WINC_ASSOC_HANDLE assoc_handler;
static void IPERFCMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void RSSIPrint(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
static const SYS_CMD_DESCRIPTOR    IPERFCmdTbl[]=
{
    {"iperf",     IPERFCMDProcessing,              ": WLAN MAC commands processing"},
    {"rssi",     RSSIPrint,              ": get rssi value"},
};
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

DRV_HANDLE wdrvHandle;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
static void _AssociationRSSICallback(DRV_HANDLE handle, WDRV_WINC_ASSOC_HANDLE assocHandle, int8_t rssi)
{
    SYS_CONSOLE_PRINT("ch rssi %d\r\n", rssi);

}

void SYS_WINCS_WIFI_CallbackHandler(SYS_WINCS_WIFI_EVENT_t event, uint8_t *p_str)
{
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_WINCS_WIFI_SNTP_UP:
        {            
            SYS_CONSOLE_PRINT("[APP] : SNTP UP \r\n"); 
            break;
        }
        break;

        /* Wi-Fi connected event code*/
        case SYS_WINCS_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT("[APP] : Wi-Fi Connected \r\n");
            assoc_handler = (WDRV_WINC_ASSOC_HANDLE)p_str;
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_WINCS_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("[APP] : Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV4_COMPLETE:
        {         
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv4 : %s\r\n", p_str);
            appData.state = APP_STATE_WINCS_SOCKET_OPEN;
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            //SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Local : %s\r\n", p_str);
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            //SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Global: %s\r\n", p_str);
            break;
        }
        
        
        default:
        {
            break;
        }
    }    
}



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_WINCS02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_WINCS_INIT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    
    if (!SYS_CMD_ADDGRP(IPERFCmdTbl, sizeof(IPERFCmdTbl)/sizeof(*IPERFCmdTbl), "iperf", ": iperf commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create IPERF Commands\r\n");
    }
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_WINCS02_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_WINCS_INIT:
        {
            SYS_STATUS status;
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_STATUS, &status);

            if (SYS_STATUS_READY == status)
            {
                appData.state = APP_STATE_WINCS_OPEN_DRIVER;
            }
            
            break;
        }
        
        case APP_STATE_WINCS_OPEN_DRIVER:
        {
            SYS_CONSOLE_PRINT("WINC: APP_STATE_WINCS_OPEN_DRIVER\r\n");
            wdrvHandle = DRV_HANDLE_INVALID;
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_OPEN_DRIVER, &wdrvHandle);
            
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE, &wdrvHandle);
            appData.state = APP_STATE_WINCS_DEVICE_INFO;
            break;
        }
        
        case APP_STATE_WINCS_DEVICE_INFO:
        {
            
            APP_DRIVER_VERSION_INFO drvVersion;
            APP_FIRMWARE_VERSION_INFO fwVersion;
            APP_DEVICE_INFO devInfo;
            SYS_WINCS_RESULT_t status = SYS_WINCS_BUSY;
            
            status = SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_DEV_INFO, &devInfo);
            
            if(status == SYS_WINCS_PASS)
            {
                SYS_CONSOLE_PRINT("WINC: SYS_WINCS_SYSTEM_SW_REV\r\n");
                status = SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_SW_REV,&fwVersion);
            }
            
            if(status == SYS_WINCS_PASS)
            {
                status = SYS_WINCS_SYSTEM_SrvCtrl (SYS_WINCS_SYSTEM_DRIVER_VER, &drvVersion);
            }
            
            if(status == SYS_WINCS_PASS)
            {
                char buff[30];
                SYS_CONSOLE_PRINT("WINC: Device ID = %08x\r\n", devInfo.id);
                for (int i=0; i<devInfo.numImages; i++)
                {
                    SYS_CONSOLE_PRINT("%d: Seq No = %08x, Version = %08x, Source Address = %08x\r\n", i, devInfo.image[i].seqNum, devInfo.image[i].version, devInfo.image[i].srcAddr);
                }
                
                SYS_CONSOLE_PRINT("Firmware Version: %d.%d.%d ", fwVersion.version.major, fwVersion.version.minor, fwVersion.version.patch);
                strftime(buff, sizeof(buff), "%X %b %d %Y", localtime((time_t*)&fwVersion.build.timeUTC));
                SYS_CONSOLE_PRINT(" [%s]\r\n", buff);
                SYS_CONSOLE_PRINT("Driver Version: %d.%d.%d\r\n", drvVersion.version.major, drvVersion.version.minor, drvVersion.version.patch);
                SYS_CONSOLE_PRINT("WINC: Device ID = %08x\r\n", devInfo.id);
                appData.state = APP_STATE_WINCS_SET_WIFI_PARAMS;
            }
            break;
        }
        case APP_STATE_WINCS_SET_WIFI_PARAMS:
        {
            
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, NULL);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_CALLBACK, NULL);
            
            SYS_WINCS_WIFI_PARAM_t wifi_sta_cfg = {
                .mode = SYS_WINCS_WIFI_MODE_STA, 
                .ssid = SYS_WINCS_WIFI_STA_SSID, 
                .security = SYS_WINCS_WIFI_STA_SECURITY,
                .passphrase = SYS_WINCS_WIFI_STA_PWD,
                .ssidVisibility = false,
                //.autoconnect = SYS_WINCS_WIFI_STA_AUTOCONNECT,
            };
            
            //SYS_WINCS_WIFI_PARAM_t wifi_sta_cfg = {SYS_WINCS_WIFI_MODE_STA, SYS_WINCS_WIFI_STA_SSID, 
            //SYS_WINCS_WIFI_STA_PWD, SYS_WINCS_STA_SECURITY, SYS_WINCS_WIFI_STA_AUTOCONNECT, 0};  

            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &wifi_sta_cfg);
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_CALLBACK, SYS_WINCS_WIFI_CallbackHandler); 
            SYS_CONSOLE_PRINT("\r\n\r\n[APP] : Wi-Fi Connecting to : %s\r\n",SYS_WINCS_WIFI_STA_SSID);
            
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            
            
            appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        
        case APP_STATE_WINCS_SOCKET_OPEN:
        {

            isConnect = true;
            IPERF_Init();
            
            appData.state = APP_STATE_SERVICE_TASKS;

            break;
        }
        
        case APP_STATE_SERVICE_TASKS:
        {
            IPERF_Update();
            
            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

static void RSSIPrint(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    
    if (WDRV_WINC_STATUS_RETRY_REQUEST != WDRV_WINC_AssocRSSIGet(assoc_handler, NULL, _AssociationRSSICallback))
    {
        return;
    }

}
static void IPERFCMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    if (argc < 2)
    {
        SYS_CONSOLE_MESSAGE("usage: iperf server <tcp | udp> <port_number> \r\n");
        SYS_CONSOLE_MESSAGE("usage: iperf client tcp <ip_address> <port_number> <number_of_packet> \r\n");
        SYS_CONSOLE_MESSAGE("usage: iperf client udp <ip_address> <port_number> <packet_len> <number_of_packet> <bps rate>\r\n");
        SYS_CONSOLE_MESSAGE("usage: iperf stop\r\n");
        //SYS_CONSOLE_MESSAGE("Notice: set <number_of_packet> to 0 for continuous packet transfer\r\n");
        return;
    }
    
    if (!strcmp("server", argv[1]))
    {
        if (argc != 4)
        {
            SYS_CONSOLE_MESSAGE("usage: iperf server <tcp | udp> <port_number> \r\n");
            return;
        }
        else
        {
            char *iperf_mode = argv[2];
            unsigned int port = strtoul(argv[3],0,10);
            
            if (isConnect)
            {
                if (!strcmp(iperf_mode, "tcp"))
                {
                    IPERF_INIT_DATA iperfConfig;

                    if (IPERF_GetNumActiveSockets() > 1)
                    {
                        SYS_CONSOLE_MESSAGE("more than 1 iperf socket is running, use command \"iperf stop\" to stop it before create another iperf stream...\r\n");
                        return;
                    }
                    IPERF_Start();

                    memset(&iperfConfig.localIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    memset(&iperfConfig.remoteIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));

                    iperfConfig.opMode = MODE_TCP_SERVER;
                    iperfConfig.localIPAddress.sin_port = htons(port);
                    iperfConfig.localIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.localIPAddress.v4.sin_addr.s_addr = htonl(INADDR_ANY);


                    if (false == IPERF_Create(&iperfConfig, false))
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) fail..\r\n");
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) Success..\r\n");
                    }
                }
                else if (!strcmp(iperf_mode, "udp"))
                {
                    IPERF_INIT_DATA iperfConfig;
                    
                    if (IPERF_GetNumActiveSockets() > 1)
                    {
                        SYS_CONSOLE_MESSAGE("more than 1 iperf socket is running, use command \"iperf stop\" to stop it before create another iperf stream...\r\n");
                        return;
                    }
                    IPERF_Start();

                    memset(&iperfConfig.localIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    memset(&iperfConfig.remoteIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));

                    iperfConfig.opMode = MODE_UDP_SERVER;
                    iperfConfig.localIPAddress.sin_port = htons(port);
                    iperfConfig.localIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.localIPAddress.v4.sin_addr.s_addr = htonl(INADDR_ANY);


                    if (false == IPERF_Create(&iperfConfig, false))
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) fail..\r\n");
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) Success..\r\n");
                    }
                }
            }
        }
    }
    if (!strcmp("client", argv[1]))
    {
        if (argc < 5)
        {
            SYS_CONSOLE_MESSAGE("usage: iperf client tcp <ip_address> <port_number> <number_of_packet> \r\n");
            SYS_CONSOLE_MESSAGE("usage: iperf client udp <ip_address> <port_number> <packet_len> <number_of_packet> <bps rate> \r\n");
            SYS_CONSOLE_MESSAGE("Notice: set <number_of_packet> to 0 for continuous packet transfer\r\n");
            return;
        }
        else
        {
            WDRV_WINC_IP_MULTI_ADDRESS remoteIPAddress;
            char *iperf_mode = argv[2];
            
            uint16_t port = strtol(argv[4],NULL,0);
            if (false == WDRV_WINC_UtilsStringToIPAddress(argv[3], &remoteIPAddress.v4)){
                SYS_CONSOLE_MESSAGE("ip address format is wrong.. \r\n");
                return;
            }

       
            if (isConnect)
            {
                if (!strcmp(iperf_mode, "tcp"))
                {
                    if (argc < 6)
                    {
                        SYS_CONSOLE_MESSAGE("usage: iperf client tcp <ip_address> <port_number> <number_of_packet> \r\n");
                        SYS_CONSOLE_MESSAGE("Notice: set <number_of_packet> to 0 for continuous packet transfer\r\n");
                        return;
                    }
                    
                    IPERF_INIT_DATA iperfConfig;

                    if (IPERF_GetNumActiveSockets() > 1)
                    {
                        SYS_CONSOLE_MESSAGE("more than 1 iperf socket is running, use command \"iperf stop\" to stop it before create another iperf stream...\r\n");
                        return;
                    }
                    IPERF_Start();
                    
                    memset(&iperfConfig.localIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    memset(&iperfConfig.remoteIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    
                    iperfConfig.opMode = MODE_TCP_CLIENT;
                    iperfConfig.remoteIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.remoteIPAddress.v4.sin_port        = htons(port);
                    iperfConfig.remoteIPAddress.v4.sin_addr.s_addr = remoteIPAddress.v4.Val;
                    iperfConfig.packetLength = MAX_PACKET_LEN;
                    iperfConfig.packetToSend = strtol(argv[5],NULL,0);
                    iperfConfig.dataRateBps = 0;
                    
                    iperfConfig.localIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.localIPAddress.v4.sin_addr.s_addr = htonl(INADDR_ANY);
        
                    
                    
                    if (false == IPERF_Create(&iperfConfig, false))
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) fail..\r\n");
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) Success..\r\n");
                    }
                }
                else if (!strcmp(iperf_mode, "udp"))
                {
                    if (argc < 8)
                    {
                        SYS_CONSOLE_MESSAGE("usage: iperf client udp <ip_address> <port_number> <packet_len> <number_of_packet> <bps rate> \r\n");
                        SYS_CONSOLE_MESSAGE("Notice: set <number_of_packet> to 0 for continuous packet transfer\r\n");
                        return;
                    }
                    IPERF_INIT_DATA iperfConfig;

                    if (IPERF_GetNumActiveSockets() > 1)
                    {
                        SYS_CONSOLE_MESSAGE("more than 1 iperf socket is running, use command \"iperf stop\" to stop it before create another iperf stream...\r\n");
                        return;
                    }
                    IPERF_Start();
                    
                    memset(&iperfConfig.localIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    memset(&iperfConfig.remoteIPAddress, 0, sizeof(APP_SOCK_ADDR_TYPE));
                    
                    iperfConfig.opMode = MODE_UDP_CLIENT;
                    iperfConfig.remoteIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.remoteIPAddress.v4.sin_port        = htons(port);
                    iperfConfig.remoteIPAddress.v4.sin_addr.s_addr = remoteIPAddress.v4.Val;
                    iperfConfig.packetLength = strtol(argv[5],NULL,0);
                    iperfConfig.packetToSend = strtol(argv[6],NULL,0);
                    iperfConfig.dataRateBps = strtol(argv[7],NULL,0);
                    
                    iperfConfig.localIPAddress.v4.sin_family      = AF_INET;
                    iperfConfig.localIPAddress.v4.sin_addr.s_addr = htonl(INADDR_ANY);

                    if ((0 == iperfConfig.packetLength) || (iperfConfig.packetLength > MAX_PACKET_LEN))
                    {
                        iperfConfig.packetLength = MAX_PACKET_LEN;
                    }
                    
                    if (false == IPERF_Create(&iperfConfig, false))
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) fail..\r\n");
                    }
                    else
                    {
                        SYS_CONSOLE_PRINT("IPERF_Create(...) Success..\r\n");
                    }
                }
            }
        }
    }
    else if (!strcmp("stop", argv[1]))
    {
        if (2 != argc)
        {
            SYS_CONSOLE_MESSAGE("usage: iperf stop \r\n");
            return;
        }
        IPERF_Stop(-1); /* stop all iperf stream */
    }
    
}

/*******************************************************************************
 End of File
 */
