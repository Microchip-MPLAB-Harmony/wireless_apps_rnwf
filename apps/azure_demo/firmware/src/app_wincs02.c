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
#include "config/sam_e54_xpro_wincs02/driver/driver_common.h"


/** Azure IoT HUB MQTT subscribe list */
static char *g_subscribeList[] = {
    SYS_WINCS_MQTT_SUB_TOPIC_0,
    SYS_WINCS_MQTT_SUB_TOPIC_1,
    SYS_WINCS_MQTT_SUB_TOPIC_2,
    NULL
};

/** Azure subscribe count */
static uint8_t                g_subCnt;

/** Shows the application's current state */
static APP_STATE_t            g_appState = APP_SYS_INIT;

/** Button Press Event */
static bool                   g_buttonPress = false;

/** System Tick Counter for 1mSec */
static uint32_t               g_sysTickCount;

/** MQTT frame */
static SYS_WINCS_MQTT_FRAME_t g_mqttFrame;

/** Azure app buffer */
static uint8_t                g_azureAppBuf[1024];

/** Application data */
APP_WINCS02_DATA               g_appData;




/** TLS configuration */
SYS_WINCS_NET_TLS_SOC_PARAMS g_tlsCfg = 
{
    .tlsPeerAuth          = SYS_WINCS_MQTT_PEER_AUTH_ENABLE,
    .tlsCACertificate     = SYS_WINCS_MQTT_SERVER_CERT, 
    .tlsCertificate       = SYS_WINCS_MQTT_DEVICE_CERT, 
    .tlsKeyName           = SYS_WINCS_MQTT_DEVICE_KEY,                                     
    .tlsKeyPassword       = SYS_WINCS_MQTT_DEVICE_KEY_PSK,                                    
    .tlsServerName        = SYS_WINCS_MQTT_TLS_SERVER_NAME,
    .tlsDomainName        = SYS_WINCS_MQTT_TLS_DOMAIN_NAME,
    .tlsDomainNameVerify  = SYS_WINCS_MQTT_TLS_DOMAIN_NAME_VERIFY
};

/** MQTT Configurations for Azure */
SYS_WINCS_MQTT_CFG_t g_mqttCfg = 
{
    .url              = SYS_WINCS_MQTT_CLOUD_URL,
    .username         = SYS_WINCS_MQTT_CLOUD_USER_NAME,
    .cleanSession     = SYS_WINCS_MQTT_CLEAN_SESSION,
    .clientId         = SYS_WINCS_MQTT_CLIENT_ID,    
    .password         = SYS_WINCS_MQTT_PASSWORD,
    .port             = SYS_WINCS_MQTT_CLOUD_PORT,
    .tlsConf          = &g_tlsCfg,
    .tlsIdx           = SYS_WINCS_MQTT_TLS_ENABLE,
    .protoVer         = SYS_WINCS_MQTT_PROTO_VERSION,
    .keepAliveTime    = SYS_WINCS_MQTT_KEEP_ALIVE_TIME,
};



// *****************************************************************************
/**
 * @brief Publish an MQTT message
 *
 * Summary:
 *    This function publishes an MQTT message to a specified topic with a given QoS level.
 *
 * Description:
 *    The function initializes an MQTT publish frame structure with the provided topic, 
 *    message, and QoS level. It sets the duplicate flag to false, the retain flag based 
 *    on a predefined constant, and the protocol version. Finally, it calls the service 
 *    control function to publish the message.
 *
 * Parameters:
 *    @param top  The topic to which the message will be published.
 *    @param msg  The message to be published.
 *    @param qos  The Quality of Service level for the message.
 *
 * Returns:
 *    SYS_WINCS_RESULT_t The result of the publish operation.
 */
static SYS_WINCS_RESULT_t APP_WINCS_APP_mqttPublish
(
    char                 *top, 
    const char           *msg, 
    SYS_WINCS_MQTT_QOS_t qos
)
{    
    SYS_WINCS_MQTT_FRAME_t mqttPub;    
    mqttPub.isDuplicate = false;
    mqttPub.qos         = qos;
    mqttPub.retain      = SYS_WINCS_MQTT_PUB_MSG_RETAIN;
    mqttPub.topic       = top;
    mqttPub.message     = msg;
    mqttPub.protoVer    = SYS_WINCS_MQTT_PROTO_VERSION;

    // Call the service control function to publish the message
    return SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_PUBLISH, (void *)&mqttPub);              
}


// *****************************************************************************
/**
 * @brief Azure MQTT Subscription Message Handler
 *
 * Summary:
 *    This function handles messages received from the Azure cloud.
 *
 * Description:
 *    The function prints a message indicating that a message has been received 
 *    from the Azure cloud. It then iterates through the received message (pointed 
 *    to by mqttHandle) and prints each character to the console.
 *
 * Parameters:
 *    @param mqttHandle  The handle to the received MQTT message.
 *
 * Returns:
 *    None
 */
static void APP_WINCS_AzureSubMsgHandler
(
    char *mqttHandle
)
{       
    SYS_CONSOLE_PRINT("Message from azure cloud:\r\n");
    
    // Iterate through the received message and print each character
    for(int i=0;mqttHandle[i];i++)
        SYS_CONSOLE_PRINT("%c",mqttHandle[i]);
    
    SYS_CONSOLE_PRINT("\r\n");
}


// *****************************************************************************
/**
 * @brief Azure MQTT Subscription Acknowledgment Handler
 *
 * Summary:
 *    This function handles the acknowledgment of MQTT subscription requests.
 *
 * Description:
 *    The function checks if there are more topics to subscribe to in the 
 *    subscription list. If there are, it formats the next topic into the 
 *    application buffer, updates the MQTT frame with the topic and protocol 
 *    version, and sends a subscription request. If there are no more topics 
 *    to subscribe to, it publishes a request to get the device twin.
 *
 * Parameters:
 *    None
 *
 * Returns:
 *    None
 */

static void APP_WINCS_azureSubAckHandler
(
    void
)
{
    // Check if there are more topics to subscribe to
    if(g_subscribeList[g_subCnt] != NULL)
    {
        // Format the next topic into the application buffer
        sprintf((char *)g_azureAppBuf, "%s", (const char *)g_subscribeList[g_subCnt++]);
        
        // Update the MQTT frame with the topic and protocol version
        g_mqttFrame.topic    = (char *)g_azureAppBuf;
        g_mqttFrame.protoVer = SYS_WINCS_MQTT_PROTO_VERSION;
        
        // Send a subscription request for the next topic
        SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SUBS_TOPIC, &g_mqttFrame);            
    }
    else
    {
        // If no more topics to subscribe to, publish a request to get the device twin
        APP_WINCS_APP_mqttPublish(AZURE_PUB_TWIN_GET, "", SYS_WINCS_MQTT_QOS0);
    }
}


// *****************************************************************************
/**
 * @brief Azure Button Telemetry Handler
 *
 * Summary:
 *    This function handles the telemetry data for button press events.
 *
 * Description:
 *    The function formats the button press count into a telemetry message 
 *    using a predefined format. It then updates the MQTT frame with the 
 *    formatted message and prints the telemetry data to the console. Finally, 
 *    it publishes the telemetry message to a predefined MQTT topic.
 *
 * Parameters:
 *    @param press_count  The count of button presses to be sent as telemetry.
 *
 * Returns:
 *    None
 */
static void APP_RNWF_azureButtonTelemetry
(
    uint32_t press_count
)
{            
    // Format the button press count into the telemetry message
    snprintf((char *)g_azureAppBuf, sizeof(g_azureAppBuf), (const char *) AZURE_FMT_BUTTON_TEL, press_count);
    
    // Update the MQTT frame with the formatted message
    g_mqttFrame.message = (const char *)g_azureAppBuf;
    SYS_CONSOLE_PRINT(TERM_CYAN"Telemetry ->> buttonEvent count %d\r\n"TERM_RESET, press_count);
    
    // Publish the telemetry message to the predefined MQTT topic
    APP_WINCS_APP_mqttPublish((char *)SYS_WINCS_MQTT_PUB_TOPIC_NAME,(const char *) g_azureAppBuf,SYS_WINCS_MQTT_QOS0);
}


// *****************************************************************************
/**
 * @brief Azure Counter Telemetry Handler
 *
 * Summary:
 *    This function handles the telemetry data for a counter.
 *
 * Description:
 *    The function formats the counter value into a telemetry message 
 *    using a predefined format. It then updates the MQTT frame with the 
 *    formatted message and prints the telemetry data to the console. Finally, 
 *    it publishes the telemetry message to a predefined MQTT topic.
 *
 * Parameters:
 *    @param counter  The counter value to be sent as telemetry.
 *
 * Returns:
 *    None
 */
static void APP_RNWF_azureCounterTelemetry
(
    uint32_t counter
)
{            
    // Format the counter value into the telemetry message
    snprintf((char *)g_azureAppBuf, sizeof(g_azureAppBuf),(const char *) AZURE_FMT_COUNTER_TEL, counter);
    
    // Update the MQTT frame with the formatted message
    g_mqttFrame.message = (const char *)g_azureAppBuf;
    SYS_CONSOLE_PRINT(TERM_YELLOW"Telemetry ->> counter count %d\r\n"TERM_RESET, counter);
    
    // Publish the telemetry message to the predefined MQTT topic
    APP_WINCS_APP_mqttPublish((char *)SYS_WINCS_MQTT_PUB_TOPIC_NAME,(const char *) g_azureAppBuf,SYS_WINCS_MQTT_QOS0);
}



// *****************************************************************************
/**
 * @brief Azure Task Handler
 *
 * Summary:
 *    This function handles periodic tasks and events related to Azure cloud 
 *    communication, such as sending telemetry data and managing subscriptions.
 *
 * Description:
 *    The function performs the following tasks:
 *    - Periodically sends counter telemetry data.
 *    - Sends button press telemetry data when a button press event is detected.
 *    - Manages MQTT topic subscriptions.
 *
 * Parameters:
 *    None
 *
 * Returns:
 *    None
 */
static void APP_WINCS_AzureTask
(
    void
)
{
    static uint32_t press_count = 0;  // Counter for button presses
    static uint32_t counter = 0;      // General counter for telemetry
    uint32_t totalTickCount = g_sysTickCount + SYS_TIME_MSToCount(APP_CLOUD_REPORT_INTERVAL);

    // Check if it's time to send periodic telemetry data
    if (totalTickCount < SYS_TIME_CounterGet())
    {
        // Send counter telemetry data
        APP_RNWF_azureCounterTelemetry(++counter);

        // Send button press telemetry data
        APP_RNWF_azureButtonTelemetry(press_count); 

        // Update the system tick count
        g_sysTickCount = SYS_TIME_CounterGet();
    }

    // Check if a button press event has occurred
    if (g_buttonPress)
    {
        // Send counter telemetry data
        APP_RNWF_azureCounterTelemetry(++counter);

        // Send button press telemetry data
        APP_RNWF_azureButtonTelemetry(++press_count);

        // Reset the button press flag
        g_buttonPress = 0;
    }

    // Check if there are more topics to subscribe to
    if (!g_subCnt && g_subscribeList[g_subCnt] != NULL)
    {
        // Format the next topic into the application buffer
        sprintf((char *)g_azureAppBuf, "%s", (const char *)g_subscribeList[g_subCnt++]);

        // Update the MQTT frame with the topic and protocol version
        g_mqttFrame.topic = (char *)g_azureAppBuf;
        g_mqttFrame.protoVer = SYS_WINCS_MQTT_PROTO_VERSION;

        // Send a subscription request for the next topic
        SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SUBS_TOPIC, &g_mqttFrame);            
    }
}


// *****************************************************************************
/**
 * @brief MQTT callback function
 *
 * Summary:
 *    This function is called when an MQTT event occurs.
 *
 * Description:
 *    This function handles various MQTT events such as connection, 
 *    disconnection, message received, etc. It takes an event type and 
 *    an MQTT handle as parameters and processes the event accordingly.
 *
 * Parameters:
 *    @param event       The MQTT event that occurred.
 *    @param mqttHandle  The handle to the MQTT instance.
 *
 * Returns:
 *    SYS_WINCS_RESULT_t The result of the callback processing.
 */
SYS_WINCS_RESULT_t APP_MQTT_Callback
(
    SYS_WINCS_MQTT_EVENT_t  event,
    SYS_WINCS_MQTT_HANDLE_t mqttHandle
)
{
    switch(event)
    {
        case SYS_WINCS_MQTT_CONNECTED:
        {    
            SYS_CONSOLE_PRINT(TERM_GREEN"\r\n[APP] : MQTT Connected\r\n");
            SYS_CONSOLE_PRINT("[APP] : Azure IOT Hub Connection Successful!\r\n"TERM_RESET);
            
            // Set up the MQTT frame with the topic and QoS for subscription
            g_mqttFrame.qos         = SYS_WINCS_MQTT_SUB_TOPIC_3_QOS;
            g_mqttFrame.topic       = SYS_WINCS_MQTT_SUB_TOPIC_3;
            g_mqttFrame.protoVer    = SYS_WINCS_MQTT_PROTO_VERSION;
            
            // Subscribe to the specified MQTT topic
            SYS_CONSOLE_PRINT("[APP] : Subscribing to topic : %s\r\n",g_mqttFrame.topic);
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SUBS_TOPIC, (void *)&g_mqttFrame);
            
            // Update the application state to indicate that the cloud connection is up
            g_appState = APP_CLOUD_UP;
            break;
        }
        
        case SYS_WINCS_MQTT_SUBCRIBE_ACK:
        {
            // Call the handler for processing the subscription acknowledgment
            CLOUD_SUBACK_HANDLER();
            break;
        }
        
        case SYS_WINCS_MQTT_SUBCRIBE_MSG:
        {   
            // Call the handler for processing the received subscription message
            CLOUD_SUBMSG_HANDLER((char *)mqttHandle);
            break;
        }
        
        case SYS_WINCS_MQTT_PUBLISH_ACK:
        {
            break;
        }
        
        
        case SYS_WINCS_MQTT_DISCONNECTED:
        {            
            SYS_CONSOLE_PRINT(TERM_RED"[APP] : MQTT - Reconnecting...\r\n"TERM_RESET);
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONNECT, (void *)&g_mqttCfg); 
            break;
        }
         
        default:
            break;
    }
    return SYS_WINCS_PASS;
}




// *****************************************************************************
// Application NET Socket Callback Handler
//
// Summary:
//    Handles NET socket events.
//
// Description:
//    This function handles various NET socket events and performs appropriate actions.
//
// Parameters:
//    socket - The socket identifier
//    event - The type of socket event
//    netHandle - Additional data or message associated with the event
//
// Returns:
//    None.
//
// Remarks:
//    None.
// *****************************************************************************
void SYS_WINCS_NET_SockCallbackHandler
(
    uint32_t socket,                    // The socket identifier
    SYS_WINCS_NET_SOCK_EVENT_t event,   // The type of socket event
    SYS_WINCS_NET_HANDLE_t netHandle    // Additional data or message associated with the event
) 
{
    switch(event)
    {
        /* Net socket connected event code*/
        case SYS_WINCS_NET_SOCK_EVENT_CONNECTED:    
        {
            SYS_CONSOLE_PRINT("Socket -> Connected to Server!\r\n" );
            break;
        }
          
        /* Net socket disconnected event code*/
        case SYS_WINCS_NET_SOCK_EVENT_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("Socket -> DisConnected!\r\n");
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
            break;
        }
         
        /* Net socket error event code*/
        case SYS_WINCS_NET_SOCK_EVENT_ERROR:
        {
            SYS_CONSOLE_PRINT("ERROR : Socket\r\n");
            break;
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_SEND_COMPLETE:
        {
            SYS_CONSOLE_PRINT("Socket -> Send Completed\r\n");
            break;
        }
            
        /* Net socket read event code*/
        case SYS_WINCS_NET_SOCK_EVENT_READ:
        {         
            SYS_CONSOLE_PRINT("Socket -> Received data \r\n");
            break; 
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_CLOSED:
        {
            SYS_CONSOLE_PRINT("Socket -> CLOSED : socketID: %d\r\n",socket);
            break;
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_TLS_DONE:    
        {
            SYS_CONSOLE_PRINT("TLS : Connected to Server!\r\n" );
            break;
        }
        
        default:
            break;                  
    }    
    
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
                SYS_CONSOLE_PRINT("Set Reg Domain -> SUCCESS\r\n");
                g_appData.state = APP_STATE_WINCS_PRINT_CERTS_KEYS;
                domainFlag = true;
            }
            
            break;
        }  
        
        /* SNTP UP event code*/
        case SYS_WINCS_WIFI_SNTP_UP:
        {            
            SYS_CONSOLE_PRINT(TERM_YELLOW"[APP] : SNTP UP \r\n"TERM_RESET);
            SYS_CONSOLE_PRINT("[APP] : Connecting to the Cloud\r\n");
            g_appData.state = APP_STATE_WINCS_CONNECT_AZURE;
            break;
            
        }
        break;

        /* Wi-Fi connected event code*/
        case SYS_WINCS_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT(TERM_GREEN"[APP] : Wi-Fi Connected    \r\n"TERM_RESET);
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_WINCS_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[APP] : Wi-Fi Disconnected\nReconnecting... \r\n"TERM_RESET);
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP IPv4 complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV4_COMPLETE:
        {         
            SYS_CONSOLE_PRINT("[APP] : DHCP IPv4 : %s\r\n", (uint8_t *)wifiHandle);
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
            
            // Request the current time from the Wi-Fi service controller
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_TIME, NULL);
            break;
        }
        
       case SYS_WINCS_WIFI_CONNECT_FAILED:
        {
            SYS_CONSOLE_PRINT("[APP] : Wi-Fi Connection Failed\nRetrying\r\n");
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            break;
        } 
        
        default:
        {
            break;
        }
    }    
}

/*EIC user handler function*/
static void APP_RNWF_eicUserHandler(uintptr_t context)
{
    g_buttonPress = 1;
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
    g_appData.state = APP_STATE_WINCS_PRINT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
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
    switch ( g_appData.state )
    {
        case APP_STATE_WINCS_PRINT:
        {
            SYS_CONSOLE_PRINT(TERM_YELLOW"########################################\r\n"TERM_RESET);
            SYS_CONSOLE_PRINT(TERM_CYAN"         WINCS02 Azure demo\r\n"TERM_RESET);
            SYS_CONSOLE_PRINT(TERM_YELLOW"########################################\r\n"TERM_RESET);
            
            g_appData.state = APP_STATE_WINCS_INIT;
            break;
        }
        
        /* Application's initial state. */
        case APP_STATE_WINCS_INIT:
        {
            SYS_STATUS status;
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_STATUS, &status);

            if (SYS_STATUS_READY == status)
            {
                g_appData.state = APP_STATE_WINCS_OPEN_DRIVER;
            }
            
            break;
        }
        
        case APP_STATE_WINCS_OPEN_DRIVER:
        {
            DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;
            // Open the Wi-Fi driver
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_OPEN_DRIVER, &wdrvHandle))
            {
                g_appData.state = APP_STATE_WINCS_ERROR;
                break;
            }

            // Get the driver handle
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE, &wdrvHandle);
            
            SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_DEBUG_UART_SET, NULL);
            
            EIC_CallbackRegister(EIC_PIN_15,APP_RNWF_eicUserHandler, 0);
            g_appData.state = APP_STATE_WINCS_DEVICE_INFO;
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
                
                g_appData.state = APP_STATE_WINCS_SET_REG_DOMAIN;
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
                g_appData.state = APP_STATE_WINCS_ERROR;
                break;
            }
            else if(SYS_WINCS_BUSY == status)
            {
                g_appData.state = APP_STATE_WINCS_SET_REG_DOMAIN;
                break;
            }
            else
            {
                
                if(g_appData.state != APP_STATE_WINCS_PRINT_CERTS_KEYS)
                {
                    g_appData.state = APP_STATE_WINCS_PRINT_CERTS_KEYS;
                    break;
                }
            }
                break;
        }
        
        case APP_STATE_WINCS_PRINT_CERTS_KEYS:
        {
            SYS_CONSOLE_PRINT("[APP] : Certificates on Device :-\r\n"TERM_YELLOW);
            if (SYS_WINCS_FAIL == SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_GET_CERT_LIST,NULL))
            {
                g_appData.state = APP_STATE_WINCS_ERROR;
                break;
            }
            g_appData.state = APP_STATE_WINCS_SET_WIFI_PARAMS;
            break;
        }
        
        case APP_STATE_WINCS_SET_WIFI_PARAMS:
        {
            if (SYS_WINCS_SYSTEM_getTaskStatus() == false)
            {
                break;
            }
            SYS_CONSOLE_PRINT(TERM_RESET);
            
            char sntp_url[] =  SYS_WINCS_WIFI_SNTP_ADDRESS;
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_SNTP, sntp_url))
            {
                g_appData.state = APP_STATE_WINCS_ERROR;
                break;
            }
            
            // Configuration parameters for Wi-Fi station mode
            SYS_WINCS_WIFI_PARAM_t wifi_sta_cfg = {
                .mode        = SYS_WINCS_WIFI_MODE_STA,        // Set Wi-Fi mode to Station (STA)
                .ssid        = SYS_WINCS_WIFI_STA_SSID,        // Set the SSID (network name) for the Wi-Fi connection
                .passphrase  = SYS_WINCS_WIFI_STA_PWD,         // Set the passphrase (password) for the Wi-Fi connection
                .security    = SYS_WINCS_WIFI_STA_SECURITY,    // Set the security type (e.g., WPA2) for the Wi-Fi connection
                .autoConnect = SYS_WINCS_WIFI_STA_AUTOCONNECT  // Enable or disable auto-connect to the Wi-Fi network
            }; 

            // Set the Wi-Fi parameters
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &wifi_sta_cfg))
            {
                g_appData.state = APP_STATE_WINCS_ERROR;
                break;
            }
            SYS_CONSOLE_PRINT("\r\n\r\n[APP] : Wi-Fi Connecting to : %s\r\n", SYS_WINCS_WIFI_STA_SSID);
            g_appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        
        case APP_STATE_WINCS_CONNECT_AZURE:
        {
            // Set the callback function for MQTT events
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SET_CALLBACK, APP_MQTT_Callback);
            
            // Configure the MQTT service with the provided configuration
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONFIG, (void *)&g_mqttCfg);
            // Connect to the MQTT broker using the specified configuration
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONNECT,(void *)&g_mqttCfg);
            
            g_appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }
        
        case APP_STATE_WINCS_SERVICE_TASKS:
        {
             if(g_appState == APP_CLOUD_UP)
            {
                CLOUD_STATE_MACHINE();
            }
            break;
        }
        
        case APP_STATE_WINCS_ERROR:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[APP_ERROR] : ERROR in Application "TERM_RESET);
            g_appData.state = APP_STATE_WINCS_SERVICE_TASKS;
            break;
        }

    }
}
/* ************************************************************************** */
/* *****************************************************************************
 End of File
 */
