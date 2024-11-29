/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                 // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes
#include "app_rnwf02.h"
#include "configuration.h"
#include "system/debug/sys_debug.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/mqtt/sys_rnwf_mqtt_service.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/sys_rnwf_system_service.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

static volatile bool isUARTTxComplete = true,isUART0TxComplete = true;

uint8_t app_buf[SYS_RNWF_BUF_LEN_MAX];

SYS_RNWF_MQTT_CFG_t mqtt_cfg = {
    .url = SYS_RNWF_MQTT_CLOUD_URL,        
    .clientid = SYS_RNWF_MQTT_CLIENT_ID,    
    .username = SYS_RNWF_MQTT_CLOUD_USER_NAME,    
    .password = SYS_RNWF_MQTT_PASSWORD,
    .port = SYS_RNWF_MQTT_CLOUD_PORT,    
    .tls_idx = 0,  
};


SYS_RNWF_MQTT_SUB_FRAME_t sub_cfg = {
    .topic = SYS_RNWF_MQTT_SUB_TOPIC_0,
    .qos = SYS_RNWF_MQTT_SUB_TOPIC_0_QOS,
};
/* Keeps the device IP address */
//static char g_DevIp[16];


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

static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        isUARTTxComplete = true;
    }
}

/* Application MQTT Callback Handler function */
SYS_RNWF_RESULT_t APP_MQTT_Callback(SYS_RNWF_MQTT_EVENT_t event,SYS_RNWF_MQTT_HANDLE_t mqttHandle )
{
    uint8_t *p_str = (uint8_t *)mqttHandle;
    switch(event)
    {
        case SYS_RNWF_MQTT_CONNECTED:
        {    
            SYS_CONSOLE_PRINT("MQTT : Connected\r\n");
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS, (void *)&sub_cfg);
        }
        break;
        
        case SYS_RNWF_MQTT_SUBCRIBE_ACK:
        {
            SYS_CONSOLE_PRINT("Subscribed to topic %s\r\n\r\n",SYS_RNWF_MQTT_SUB_TOPIC_0);
        }
        break;
        
        case SYS_RNWF_MQTT_SUBCRIBE_MSG:
        {   
            SYS_CONSOLE_PRINT("RNWF_MQTT_SUBCRIBE_MSG <- %s\r\n", p_str);
        }
        break;
        
        case SYS_RNWF_MQTT_DISCONNECTED:
        {            
            SYS_CONSOLE_PRINT("MQTT - Reconnecting...\r\n");
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);            
        }
        break; 
        
        default:
        break;
    }
    return SYS_RNWF_PASS;
}

/* Application Wi-fi Callback Handler function */
void APP_WIFI_Callback(SYS_RNWF_WIFI_EVENT_t event, SYS_RNWF_WIFI_HANDLE_t wifiHandle)
{
    uint8_t *p_str = (uint8_t *)wifiHandle;
            
    switch(event)
    {
        case SYS_RNWF_WIFI_SNTP_UP:
        {            
            static uint8_t flag =1;
            if(flag==1)
            {
                SYS_CONSOLE_PRINT("SNTP UP:%s\r\n", &p_str[2]);
                SYS_CONSOLE_PRINT("Connecting to the MQTT Server\r\n");
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SET_CALLBACK, APP_MQTT_Callback);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONFIG, (void *)&mqtt_cfg);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);
                flag=0;
            }
        }
        break;
        
        case SYS_RNWF_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected    \r\n");
        
        }
        break;
        
        case SYS_RNWF_WIFI_DISCONNECTED:
        {
           SYS_CONSOLE_PRINT("Wi-Fi Disconnected\nReconnecting... \r\n");
           SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_STA_CONNECT, NULL); 
        }
        break;
            
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV4_COMPLETE:
        {
            SYS_CONSOLE_PRINT("DHCP Done...%s \r\n",&p_str[2]);
            break;
        }
        
        /* Wi-Fi IPv6 Local DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Local DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Local IPv6 address code*/     
            break;
        }
        
        /* Wi-Fi IPv6 Global DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Global DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Global IPv6 address code*/     
            break;
        }
        
        case SYS_RNWF_WIFI_SCAN_INDICATION:
            break;
            
        case SYS_RNWF_WIFI_SCAN_DONE:
            break;
            
        default:
            break;
                    
    }    
}



/* TODO:  Add any necessary local functions.
*/


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

void APP_RNWF02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INITIALIZE;

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

void APP_RNWF02_Tasks ( void )
{

    /* Check the application's current state. */
    switch(appData.state)
    {
        case APP_STATE_INITIALIZE:
        {
            SYS_CONSOLE_PRINT("########################################\r\n");
            SYS_CONSOLE_PRINT("        RNWF02 Basic MQTT demo\r\n");
            SYS_CONSOLE_PRINT("########################################\r\n");
            
            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
            SYS_RNWF_IF_Init();
            
            appData.state = APP_STATE_REGISTER_CALLBACK;
            SYS_CONSOLE_PRINT("APP_STATE_INITIALIZE\r\n");
            break;
        }
        case APP_STATE_REGISTER_CALLBACK:
        {
                
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RWWF_SYSTEM_GET_WIFI_INFO, app_buf);    
            SYS_CONSOLE_PRINT("Wi-Fi Info:- \r\n%s\r\n\r\n", app_buf);
            
            char sntp_url[] =  SYS_RNWF_SNTP_ADDRESS;
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SET_SNTP, sntp_url);
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SW_REV, app_buf);    
            SYS_CONSOLE_PRINT("Software Revision:- %s\r\n", app_buf);
            
              
            /* RNWF Application Callback register */
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, APP_WIFI_Callback);
          
            /* Set Regulatory domain/Country Code */
            const char *regDomain = SYS_RNWF_COUNTRYCODE;
            SYS_CONSOLE_PRINT("\r\nSetting regulatory domain : %s\r\n",regDomain);
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_REGULATORY_DOMAIN, (void *)regDomain);
            
            /* Wi-Fi Connectivity */
            SYS_CONSOLE_PRINT("Connecting to : %s\r\n",SYS_RNWF_WIFI_STA_SSID);
            SYS_RNWF_WIFI_PARAM_t wifi_sta_cfg = {SYS_RNWF_WIFI_MODE_STA, SYS_RNWF_WIFI_STA_SSID, SYS_RNWF_WIFI_STA_PWD, SYS_RNWF_STA_SECURITY, SYS_RNWF_WIFI_STA_AUTOCONNECT};        
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_SET_WIFI_PARAMS, &wifi_sta_cfg);

            appData.state = APP_STATE_TASK;
            break;
        }
        case APP_STATE_TASK:
        {
            SYS_RNWF_IF_EventHandler();
            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
