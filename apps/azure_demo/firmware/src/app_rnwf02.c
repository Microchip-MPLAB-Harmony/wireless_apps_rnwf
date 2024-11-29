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

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h> 
#include <stdlib.h> 

/* This section lists the other files that are included in this file.*/
#include "app_rnwf02.h"
#include "user.h"
#include "definitions.h"
#include "configuration.h"
#include "system/debug/sys_debug.h"
#include "peripheral/port/plib_port.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/sys_rnwf_system_service.h"
#include "system/mqtt/sys_rnwf_mqtt_service.h"
#include "peripheral/eic/plib_eic.h"

#define CLOUD_STATE_MACHINE()         APP_RNWF_AZURE_Task()
#define CLOUD_SUBACK_HANDLER()        APP_RNWF_azureSubackHandler()
#define CLOUD_SUBMSG_HANDLER(msg)     SYS_RNWF_APP_AZURE_SUB_Handler(msg)


/* Variable to check the UART transfer */
static volatile bool g_isUARTTxComplete = true,isUART0TxComplete = true;;

/*Shows the he application's current state*/
static APP_STATE_t g_appState = APP_SYS_INIT;

/* Keeps the device IP address */
static uint8_t g_devIp[64];

/*Shows the he application's current state*/
static APP_DATA1 g_appData;

/*Application buffer to store data*/
static uint8_t g_appBuf[SYS_RNWF_BUF_LEN_MAX];


/**TLS Configuration for the Azure */
static const char *g_tlsCfgAzure[] = 
                                {
                                    SYS_MQTT_ENABLE_PEER_AUTH,
                                    SYS_RNWF_MQTT_SERVER_CERT, 
                                    SYS_RNWF_MQTT_DEVICE_CERT, 
                                    SYS_RNWF_MQTT_DEVICE_KEY,                                     
                                    SYS_RNWF_MQTT_DEVICE_KEY_PSK,                                    
                                    SYS_RNWF_MQTT_TLS_SERVER_NAME,
                                    SYS_MQTT_DOMAIN_NAME_VERIFY,
                                    SYS_RNWF_MQTT_TLS_DOMAIN_NAME
                                };
         

/*MQTT Configurations for azure*/
SYS_RNWF_MQTT_CFG_t mqtt_cfg = {
    .url      = SYS_RNWF_MQTT_CLOUD_URL,
    .username = SYS_RNWF_MQTT_CLOUD_USER_NAME,
    .clientid = SYS_RNWF_MQTT_CLIENT_ID,    
    .password = SYS_RNWF_MQTT_PASSWORD,
    .port     = SYS_RNWF_MQTT_CLOUD_PORT,
    .tls_conf = (uint8_t *) g_tlsCfgAzure,
    .tls_idx  = SYS_RNWF_NET_TLS_CONFIG_2,
    .protoVer = SYS_RNWF_MQTT_VERSION,
    .keep_alive_time = SYS_RNWF_MQTT_KEEP_ALIVE_INT,
};

/*Azure app buffer*/
uint8_t azure_app_buf[SYS_RNWF_BUF_LEN_MAX];

/* Button Press Event */
static bool    g_buttonPress = false;

/*MQTT data publish function*/
static SYS_RNWF_RESULT_t APP_RNWF_mqttPublish(const char *top, const char *msg)
{    
    SYS_RNWF_MQTT_FRAME_t mqtt_pub;    
    mqtt_pub.isNew    = SYS_RNWF_NEW_MSG;
    mqtt_pub.qos      = SYS_RNWF_MQTT_PUB_QOS_TYPE;
    mqtt_pub.isRetain = SYS_RNWF_NO_RETAIN;
    mqtt_pub.topic    = top;
    mqtt_pub.message  = msg;
    return SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_PUBLISH, (void *)&mqtt_pub);              
}

/**Azure IoT HUB subscribe list */
//static const char *g_subscribeList[] = {SYS_RNWF_MQTT_SUB_TOPIC_0, SYS_RNWF_MQTT_SUB_TOPIC_1, SYS_RNWF_MQTT_SUB_TOPIC_2,  NULL};

///* System Tick Counter for 1mSec*/
static uint32_t g_sysTickCount;

/*Function to send the button press count data*/
static void APP_RNWF_azureButtonTelemetry(uint32_t press_count)
{            
    snprintf((char *)azure_app_buf, sizeof(azure_app_buf), (const char *) AZURE_FMT_BUTTON_TEL, press_count);
    SYS_CONSOLE_PRINT("Telemetry ->> buttonEvent count %d\r\n", press_count);
    APP_RNWF_mqttPublish((const char *)SYS_RNWF_MQTT_PUB_TOPIC_NAME,(const char *) azure_app_buf);
}

/*Function to send the counter data*/
static void APP_RNWF_azureCounterTelemetry(uint32_t counter)
{            
    snprintf((char *)azure_app_buf, sizeof(azure_app_buf),(const char *) AZURE_FMT_COUNTER_TEL, counter);
    SYS_CONSOLE_PRINT("Telemetry ->> counter count %d\r\n", counter);
    APP_RNWF_mqttPublish((const char *)SYS_RNWF_MQTT_PUB_TOPIC_NAME,(const char *) azure_app_buf);
}


/*Azure subscribe acknowledgement handler*/
static void APP_RNWF_azureSubackHandler()
{
        APP_RNWF_mqttPublish(AZURE_PUB_TWIN_GET, "");
}


/*EIC user handler function*/
static void APP_RNWF_eicUserHandler(uintptr_t context)
{
    g_buttonPress = 1;
}

/*Azure components initialization function*/
static void APP_RNWF_AZURE_INIT()
{
    EIC_CallbackRegister(EIC_PIN_15,APP_RNWF_eicUserHandler, 0);
}

/*Function to handle azure tasks*/
static void APP_RNWF_AZURE_Task(void)
{
    static uint32_t press_count = 0;
    static uint32_t counter = 0;
    uint32_t totalTickCount = g_sysTickCount + SYS_TIME_MSToCount(APP_CLOUD_REPORT_INTERVAL);
    
    if(totalTickCount < SYS_TIME_CounterGet())
    {
        APP_RNWF_azureCounterTelemetry(++counter);
        APP_RNWF_azureButtonTelemetry(press_count); 
        
        g_sysTickCount = SYS_TIME_CounterGet();
    }
       
    if(g_buttonPress)
    {
        APP_RNWF_azureCounterTelemetry(++counter);
        APP_RNWF_azureButtonTelemetry(++press_count);
        g_buttonPress = 0;
    }
}

/*Azure subscribe handler function*/
void SYS_RNWF_APP_AZURE_SUB_Handler(char *p_str)
{       
    SYS_CONSOLE_PRINT("Message from azure cloud:\r\n");
    
    for(int i=0;p_str[i];i++)
        SYS_CONSOLE_PRINT("%c",p_str[i]);
    
    SYS_CONSOLE_PRINT("\r\n");
}



/* DMAC Channel Handler Function */
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        g_isUARTTxComplete = true;
    }
}

/*function to get IP address*/
uint8_t *APP_GET_IP_Address(void)
{
    return g_devIp;
}

/* Application MQTT Callback Handler function */
SYS_RNWF_RESULT_t APP_MQTT_Callback(SYS_RNWF_MQTT_EVENT_t event,SYS_RNWF_MQTT_HANDLE_t mqttHandle )
{
    uint8_t *p_str = (uint8_t *)mqttHandle;
    switch(event)
    {
        /* MQTT connected event code*/
        case SYS_RNWF_MQTT_CONNECTED:
        {    
            SYS_CONSOLE_PRINT("MQTT : Connected\r\n");
            SYS_RNWF_MQTT_SUB_FRAME_t sub_cfg ={ 
                .topic = SYS_RNWF_MQTT_SUB_TOPIC_0,
                .qos = SYS_RNWF_MQTT_SUB_TOPIC_0_QOS,
            };
            SYS_RNWF_MQTT_DBG_MSG("Azure IOT Hub Connection Successful!\r\n");
            SYS_RNWF_MQTT_DBG_MSG("Subscribing to topic...\r\n");
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS, (void *)&sub_cfg);
            
            g_appState = APP_CLOUD_UP;
            break;
        }
        
        /* MQTT Subscribe acknowledge event code*/
        case SYS_RNWF_MQTT_SUBCRIBE_ACK:
        {
            CLOUD_SUBACK_HANDLER();
            break;
        }
        
        /* MQTT Subscribe message event code*/
        case SYS_RNWF_MQTT_SUBCRIBE_MSG:
        { 
            CLOUD_SUBMSG_HANDLER((char *)p_str);
            break;
        }
        
        /*MQTT Disconnected event code*/
        case SYS_RNWF_MQTT_DISCONNECTED:
        {            
            SYS_CONSOLE_PRINT("MQTT - Reconnecting...\r\n");
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL); 
            break;
        }
               
        /* MQTT DPS status event code*/
        case SYS_RNWF_MQTT_DPS_STATUS:
        {
            if(*p_str == 1)
            {
                SYS_CONSOLE_PRINT("DPS Successful! Connecting to Azure IoT Hub\r\n");
            }
            else
            {   
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONFIG, (void *)&mqtt_cfg);                                                           
            }
            SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);                
            break;
        }    
        default:
        {
            break;
        }
    }
    return SYS_RNWF_PASS;
}

/* Application Wi-fi Callback Handler function */
void APP_WIFI_Callback(SYS_RNWF_WIFI_EVENT_t event, SYS_RNWF_WIFI_HANDLE_t wifiHandle)
{
    uint8_t *p_str =(uint8_t *)wifiHandle;
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_RNWF_WIFI_SNTP_UP:
        {            
                
                static uint8_t flag =1;
                if(flag==1)
                {
                    SYS_CONSOLE_PRINT("SNTP UP:%s\n", &p_str[0]);  
                    SYS_CONSOLE_PRINT("Connecting to the Cloud\r\n");
                    SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SET_CALLBACK, APP_MQTT_Callback);
                    SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONFIG, (void *)&mqtt_cfg);
                    SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);
                    flag=0;
                }
                break;
        }
        
        /* Wi-Fi connected event code*/
        case SYS_RNWF_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected\r\n");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_RNWF_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV4_COMPLETE:
        {
            SYS_CONSOLE_PRINT("DHCP Done...%s \r\n",&p_str[2]); 
            strncpy((char *)g_devIp,(const char *) &p_str[3], strlen((const char *)(&p_str[3]))-1);
            break;
        }
        
         /* Wi-Fi IPv6 DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Local DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Local IPv6 address code*/     
            break;
        }
        case SYS_RNWF_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Global DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Global IPv6 address code*/     
            break;
        }
        
        /* Wi-Fi scan indication event code*/
        case SYS_RNWF_WIFI_SCAN_INDICATION:
        {
            break;
        }
        
        /* Wi-Fi scan complete event code*/
        case SYS_RNWF_WIFI_SCAN_DONE:
        {
            break;
        }
        
        default:
        {
            break;
        }
                    
    }    
}

/*Application initialization function*/
void APP_RNWF02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    g_appData.state = APP_STATE_INITIALIZE;
}


/* Maintain the application's state machine.*/
void APP_RNWF02_Tasks ( void )
{
    switch(g_appData.state)
    {
        /* Application's state machine's initial state. */
        case APP_STATE_INITIALIZE:
        {
            SYS_CONSOLE_PRINT("########################################\r\n");
            SYS_CONSOLE_PRINT("   Welcome RNWF Azure Cloud Demo \r\n");
            SYS_CONSOLE_PRINT("########################################\r\n");

            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
            SYS_RNWF_IF_Init();
            
            APP_RNWF_AZURE_INIT();
            
            g_appData.state = APP_STATE_REGISTER_CALLBACK;
            SYS_CONSOLE_PRINT("APP_STATE_INITIALIZE\r\n");
            break;
        }
        
        /* Register the necessary callbacks */
        case APP_STATE_REGISTER_CALLBACK:
        {
            SYS_CONSOLE_Tasks(sysObj.sysConsole0);
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RWWF_SYSTEM_GET_WIFI_INFO, g_appBuf);    
            SYS_CONSOLE_PRINT("Wi-Fi Info:- \r\n%s\r\n\r\n", g_appBuf);
            
            SYS_CONSOLE_Tasks(sysObj.sysConsole0);
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_GET_CERT_LIST, g_appBuf);    
            SYS_CONSOLE_PRINT("Certs on RNWF:- \r\n%s\r\n\r\n", g_appBuf);
            
            SYS_CONSOLE_Tasks(sysObj.sysConsole0);
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_GET_KEY_LIST, g_appBuf);    
            SYS_CONSOLE_PRINT("Keys on RNWF:- \r\n%s\r\n\r\n", g_appBuf);
            
            char sntp_url[] =  SYS_RNWF_SNTP_ADDRESS;
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SET_SNTP,sntp_url);
             
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SW_REV, g_appBuf);    
            SYS_CONSOLE_PRINT("Software Revision:- %s\r\n", g_appBuf);
            
              
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

            g_appData.state = APP_STATE_TASK;
            break;
        }
        
        /* Run Event handler */
        case APP_STATE_TASK:
        {
            if(g_appState == APP_CLOUD_UP)
            {
                CLOUD_STATE_MACHINE();
            }
            
            break;
        }
        
        default:
        {
            break;
        }
    }
    
    SYS_CONSOLE_Tasks(sysObj.sysConsole0);
            
    SYS_RNWF_IF_EventHandler();
    
}