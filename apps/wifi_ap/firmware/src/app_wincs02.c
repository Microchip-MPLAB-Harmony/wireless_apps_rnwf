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

#include "webpages/index_page.h"
#include "webpages/success_image.h"


#define MAX_PACKET_LEN  1452    /* ipv4 packet len */

/*! Standard GET response. */
#define webHTTP_OK	"HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"
#define webImageHTTP_OK	"HTTP/1.0 200 OK\r\nContent-type: image/gif\r\n\r\n"

#define FILE_TYPE_PNG   0
#define FILE_TYPE_HTML  1


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

/** Webpage to send to connected STA. */

static int remaining_size = 0;
static uint8_t *   webpage_contents_ptr;

static int close_socket = 0;
static int send_counter = 0;
static uint32_t webpage_socket;
static bool isConnect = false;

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

static int http_request_webpage_contents(uint32_t socket, uint8_t contents[], int size_of_contents, int file_type)
{
    /* Generate page header. */
    if ( FILE_TYPE_PNG == file_type )
    {
        SYS_WINCS_NET_TcpSockWrite(socket, (uint32_t)strlen(webImageHTTP_OK), (uint8_t *) webImageHTTP_OK);
    }
    else    /* FILE_TYPE_HTML */
    {
        SYS_WINCS_NET_TcpSockWrite(socket, (uint32_t)strlen(webHTTP_OK), (uint8_t *) webHTTP_OK);
    }
    send_counter++;

    SYS_CONSOLE_PRINT("[APP] : Send webpage contents(%d)\r\n", size_of_contents);
    
    close_socket = 0;
    webpage_socket = socket;
    webpage_contents_ptr = contents;
    remaining_size = size_of_contents;
    appData.state = APP_STATE_WINCS_SEND_WEBPAGE;

    return 0;
}

static int http_request_index(uint32_t socket)
{
    /* Generate page header. */
    SYS_WINCS_NET_TcpSockWrite(socket, (uint32_t)strlen(webHTTP_OK), (uint8_t *) webHTTP_OK); 
    send_counter++;
    
    SYS_CONSOLE_PRINT("[APP] : Send index.htm (%d)\r\n", sizeof(index_page));
    
    close_socket = 0;
    webpage_socket = socket;
    webpage_contents_ptr = index_page;
    remaining_size = sizeof(index_page);
    appData.state = APP_STATE_WINCS_SEND_WEBPAGE;
    
    return 0;
}
#if 0
static void delay_before_close_socket()
{
    int delay_cnt = 1000000;
    while ( delay_cnt-- );
}
#endif
/* Application NET socket Callback Handler function */
void SYS_WINCS_NET_SockCallbackHandler(uint32_t socket, SYS_WINCS_NET_SOCK_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        /* Net socket connected event code*/
        case SYS_WINCS_NET_SOCK_EVENT_CONNECTED:    
        {
            SYS_CONSOLE_PRINT("[APP] : Connected to Server!\r\n" );
            break;
        }
          
        case SYS_WINCS_NET_SOCK_EVENT_CLIENT_CONNECTED:
        {
            SYS_CONSOLE_PRINT("\r\n[APP] : Client Connected!\r\n" );
            break;
        }
        /* Net socket disconnected event code*/
        case SYS_WINCS_NET_SOCK_EVENT_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("[APP] : DisConnected!\r\n");
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
            break;
        }
         
        /* Net socket error event code*/
        case SYS_WINCS_NET_SOCK_EVENT_ERROR:
        {
            SYS_CONSOLE_PRINT("ERROR : Socket\r\n");
            break;
        }
            
        /* Net socket send event code*/
        case SYS_WINCS_NET_SOCK_EVENT_SEND_COMPLETE:
        {       
            send_counter --;
 
            if (close_socket && send_counter <= 0)
            {
                //delay_before_close_socket();
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
                close_socket = 0;                
            }

            break;
        }
        /* Net socket read event code*/
        case SYS_WINCS_NET_SOCK_EVENT_READ:
        {         
            uint8_t rx_data[SYS_WINCS_NET_SOCK_RCV_BUF_SIZE];
            int16_t rcvd_len = SYS_WINCS_NET_SOCK_RCV_BUF_SIZE;
            
            memset(rx_data,0,SYS_WINCS_NET_SOCK_RCV_BUF_SIZE);
            if((rcvd_len = SYS_WINCS_NET_TcpSockRead(socket, SYS_WINCS_NET_SOCK_RCV_BUF_SIZE, rx_data)) > 0)
            {
                rcvd_len = strlen((char *)rx_data);
                rx_data[rcvd_len] = '\n';
                
                SYS_CONSOLE_PRINT("[APP] : Received (%d)->%s\r\n", rcvd_len, rx_data);
                
                if (strstr((char*) rx_data, "GET /Success_image.gif") != NULL)
                {
                    http_request_webpage_contents(socket, success_image, sizeof(success_image), FILE_TYPE_PNG);
                    
                }
                else if (strstr((char*) rx_data, "GET / HTTP/1.1") != NULL)
                {
                    http_request_index(socket);
                }
                else
                {
                    SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
                }
            }    

            break; 
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_CLOSED:
        {
            SYS_CONSOLE_PRINT("[APP] : Socket CLOSED\r\n");
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
            break;
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_TLS_DONE:    
        {
            SYS_CONSOLE_PRINT("[APP] : TLS ->Connected to Server!\r\n" );
            break;
        }
        
        default:
            break;                  
    }    
    
}


void SYS_WINCS_WIFI_CallbackHandler(SYS_WINCS_WIFI_EVENT_t event, uint8_t *p_str)
{
            
    switch(event)
    {
        case SYS_WINCS_WIFI_REG_DOMAIN_SET_ACK:
        {
            SYS_CONSOLE_PRINT("Set Reg Domain -> SUCCESS\r\n\r\n");
            appData.state = APP_STATE_WINCS_SET_WIFI_PARAMS;
            break;
        }  
        
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
            SYS_CONSOLE_PRINT("[APP] : Wi-Fi Connected    \r\n");
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
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Local : %s\r\n", p_str);
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv6 Global: %s\r\n", p_str);
            break;
        }
        
        
        
        default:
        {
            break;
        }
    }    
}

/* TCP Socket Configurations */
SYS_WINCS_NET_SOCKET_t g_HTTPServerSocket = {
    /*Socket bind type*/
    .bindType = SYS_WINCS_NET_BIND_TYPE0,
    
    /*Socket port number */
    .sockPort = SYS_WINCS_NET_SOCK_PORT0,
    
    /*Socket type(TCP/UDP)*/
    .sockType = SYS_WINCS_NET_SOCK_TYPE0,
    
    .ipType = SYS_WINCS_NET_IPV4,
};

int server_socket_init = 0;
void SYS_WINCS_NET_CallbackHandler(SYS_WINCS_NET_DHCP_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        case SYS_WINCS_NET_STA_DHCP_DONE:
        {
            SYS_WINCS_WIFI_DBG_MSG("STA Connected -> DHCP IP : %s\r\n", p_str);
            
            if (server_socket_init == 0)
            {
                server_socket_init = 1;
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_TCP_OPEN, &g_HTTPServerSocket);
            }
            break;
        }
    }
    return;
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
            SYS_WINCS_WIFI_PARAM_t wifi_sta_cfg = {
                .mode = SYS_WINCS_WIFI_MODE_AP, 
                .ssid = SYS_WINCS_WIFI_AP_SSID, 
                .security = SYS_WINCS_SOFT_AP_SECURITY,
                .channel  = SYS_WINCS_WIFI_AP_CHANNEL,
                .passphrase = SYS_WINCS_WIFI_AP_PWD,
                .ssidVisibility = SYS_WINCS_WIFI_AP_SSID_VISIBILITY,
            };
            
            /* WINCS Application Callback register */
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, SYS_WINCS_NET_CallbackHandler);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_CALLBACK, SYS_WINCS_NET_SockCallbackHandler);
            
            const char *dhcps_cfg[] = {SYS_WINCS_WIFI_AP_IP_ADDR, SYS_WINCS_WIFI_AP_IP_POOL_START};
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_ENABLE, dhcps_cfg); 
            
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &wifi_sta_cfg);
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_ENABLE, NULL);
            
            SYS_CONSOLE_PRINT("\r\n\r\n[APP] : Wi-Fi [AP] connect to : %s ,password : %s\r\n",SYS_WINCS_WIFI_AP_SSID, SYS_WINCS_WIFI_AP_PWD);
            
            appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_WINCS_SEND_WEBPAGE:
        {           
            if (remaining_size >= MAX_TCP_SOCK_PAYLOAD_SZ)
            {
                if (SYS_WINCS_PASS != SYS_WINCS_NET_TcpSockWrite(webpage_socket, MAX_TCP_SOCK_PAYLOAD_SZ, webpage_contents_ptr))
                {
                    break;
                }
                send_counter ++;
                webpage_contents_ptr += MAX_TCP_SOCK_PAYLOAD_SZ;
                remaining_size -= MAX_TCP_SOCK_PAYLOAD_SZ;
            }
            else
            {
                if (SYS_WINCS_PASS != SYS_WINCS_NET_TcpSockWrite(webpage_socket, remaining_size, webpage_contents_ptr))
                {
                    break;
                }
                send_counter ++;
                webpage_contents_ptr += remaining_size;
                remaining_size = 0;
                close_socket = 1;
                appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            }
            break;
        }
        
        case APP_STATE_WINCS_SOCKET_OPEN:
        {

            isConnect = true;
            
            appData.state = APP_STATE_SERVICE_TASKS;

            break;
        }
        
        case APP_STATE_SERVICE_TASKS:
        {
            
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


/*******************************************************************************
 End of File
 */
