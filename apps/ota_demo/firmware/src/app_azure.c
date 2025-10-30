/*******************************************************************************
Copyright (C) 2024 released Microchip Technology Inc. All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/

/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */


/* This section lists the other files that are included in this file.
 */
#include "app_azure.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************


// Global data for Azure application
static APP_AZURE_CONTEXT      g_appAzureCtx;

/** Azure subscribe count */
static uint8_t                g_subCnt;

/** Button Press Event */
static bool                   g_buttonPress = false;

/** MQTT frame */
static SYS_WINCS_MQTT_FRAME_t g_mqttFrame;

/** Azure app buffer */
static uint8_t                g_azureAppBuf[1024];


/** TLS configuration */
static SYS_WINCS_NET_TLS_SOC_PARAMS g_tlsCfg = 
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
static SYS_WINCS_MQTT_CFG_t g_mqttCfg =
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


// MQTT Subscription List
static char *g_subscribeList[] = {
    SYS_WINCS_MQTT_SUB_TOPIC_0,
    SYS_WINCS_MQTT_SUB_TOPIC_1,
    SYS_WINCS_MQTT_SUB_TOPIC_2,
    NULL
};

static void APP_AZURE_CmdPrintUsage
(
    void
)
{
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n*********************** COMMAND HELP ************************************\r\n"TERM_RESET);
    SYS_CONSOLE_PRINT(">> Command     : azure \r\n");
    SYS_CONSOLE_PRINT(">> Description : Connect to Azure  \r\n");
    SYS_CONSOLE_PRINT(">> Options     : connect    -> Connect to Azure cloud\r\n"
                      "               : disconnect -> Disconnect from Azure cloud\r\n"
                      "               : help       -> Help Command\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : azure connect\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : azure disconnect\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : azure help\r\n");
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n**************************************************************************\r\n"TERM_RESET);
    
    #ifdef AZURE_EXTENDED_COMMAND_ENABLE    
    SYS_CONSOLE_PRINT("\tusage: ota mqtt config \r\n"
                        "\tota mqtt config 1 <BROKER_ADDR>\r\n"
                        "\tota mqtt config 2 <BROKER_PORT>\r\n"
                        "\tota mqtt config 3 <CLIENT_ID>\r\n"
                        "\tota mqtt config 4 <USERNAME>\r\n"
                        "\tota mqtt config 5 <PASSWORD>>\r\n"
                        "\tota mqtt config 6 <KEEP>\r\n"
                        "\tota mqtt config 7 <PROTO_VER (3/5)>\r\n");
    
    SYS_CONSOLE_PRINT("\tusage: ota tls config \r\n"
                        "\tota tls config 1 <CA_CERT_NAME>\r\n"
                        "\tota tls config 2 <CERT_NAME>\r\n"
                        "\tota tls config 3 <PRI_KEY_NAME>\r\n"
                        "\tota tls config 4 <PRI_KEY_PASSWORD>\r\n"
                        "\tota tls config 5 <SERVER_NAME>\r\n"
                        "\tota tls config 6 <DOMAIN_NAME>\r\n"
                        "\tota tls config 7 <PEER_AUTH (0/1)>\r\n"
                        "\tota tls config 8 <PEER_DOMAIN_VERIFY (0/1)>\r\n");
    #endif
    return;
}
// *****************************************************************************
// *****************************************************************************
// Function: APP_AZURE_CMDProcessing
//
// Summary:
//    Processes Azure commands received from the console.
//
// Description:
//    This function processes the Azure commands received from the console and
//    updates the Azure state accordingly.
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
void APP_AZURE_CMDProcessing
(
    SYS_CMD_DEVICE_NODE* pCmdIO, 
    int argc, 
    char** argv
)
{
    SYS_CONSOLE_PRINT(TERM_RESET"Command Received : azure\r\n");
    if (argc < 2)
    {
        SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Invalid Number of parameters. Check \"azure help\" command\r\n" TERM_RESET);
        return;
    }

    if (strcmp(argv[0], "azure") == 0)
    {
        if (strcmp(argv[1], "connect") == 0)
        {
            g_appAzureCtx.state = APP_AZURE_STATE_START;
        }
        #ifdef AZURE_EXTENDED_COMMAND_ENABLE 
        else if (strcmp(argv[1], "tls") == 0)
        {
            if (strcmp(argv[2], "config") == 0)
            {
                if(argc < 5)
                {
                    SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Usage of Command\r\n"TERM_RESET);
                    APP_AZURE_CmdPrintUsage();
                }
                switch(argv[3][0])
                {
                    case '1':
                        strncpy(g_tlsCfg.tlsCACertificate, argv[4], sizeof(argv[4]) - 1);
                        break;
                            
                    case '2':
                        strncpy(g_tlsCfg.tlsCertificate, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '3':
                        strncpy(g_tlsCfg.tlsKeyName, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '4':
                        strncpy(g_tlsCfg.tlsKeyPassword, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '5':
                        strncpy(g_tlsCfg.tlsServerName, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '6':
                        strncpy(g_tlsCfg.tlsDomainName, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '7':
                        if (strcmp(argv[3], "0") == 0) {
                            g_tlsCfg.tlsPeerAuth = true;
                        } else if (strcmp(argv[3], "1") == 0) {
                            g_tlsCfg.tlsPeerAuth = false;
                        } else {
                            APP_AZURE_CmdPrintUsage();
                            return;
                        }
                        break;
                    case '8':
                        if (strcmp(argv[3], "0") == 0) {
                            g_tlsCfg.tlsDomainNameVerify = true;
                        } else if (strcmp(argv[3], "1") == 0) {
                            g_tlsCfg.tlsDomainNameVerify = false;
                        } else {
                            APP_AZURE_CmdPrintUsage();
                            return;
                        }
                        break;
                    default:
                        APP_AZURE_CmdPrintUsage();
                        return;
                }
            }
        }
        else if (strcmp(argv[1], "mqtt") == 0){
            if (strcmp(argv[2], "config") == 0){
                if(argc < 5){
                    SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Usage of Command\r\n"TERM_RESET);
                    APP_AZURE_CmdPrintUsage();
                }
                switch(argv[3][0]){
                    case '1':
                        strncpy((char *)g_mqttCfg.url, argv[4], sizeof(argv[4]) - 1);
                        break;
                            
                    case '2':
                        g_mqttCfg.port = atoi(argv[4]);
                        break;
                        
                    case '3':
                        strncpy((char *)g_mqttCfg.clientId, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '4':
                        strncpy((char *)g_mqttCfg.username, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '5':
                        strncpy((char *)g_mqttCfg.password, argv[4], sizeof(argv[4]) - 1);
                        break;
                        
                    case '6':
                        g_mqttCfg.keepAliveTime = atoi(argv[4]);
                        break;
                        
                    case '7':
                        if (strcmp(argv[3], "0") == 0) {
                            g_mqttCfg.protoVer = SYS_WINCS_MQTT_PROTO_VER_3;
                        }
                        else if (strcmp(argv[3], "0") == 0) {
                            g_mqttCfg.protoVer = SYS_WINCS_MQTT_PROTO_VER_5;
                        }
                        else{
                            SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Usage of Command\r\n"TERM_RESET);
                        }
                        break;
                        
                    default:
                        APP_AZURE_CmdPrintUsage();
                        return;
                }
            }
            else{
                SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Usage of Command\r\n"TERM_RESET);
                APP_AZURE_CmdPrintUsage();
            }
        }
        #endif
        
        else if (strcmp(argv[1], "disconnect") == 0){
            g_appAzureCtx.state = APP_AZURE_STATE_STOP;
        }
        else if (strcmp(argv[1], "help") == 0){
            APP_AZURE_CmdPrintUsage();
        }
        else{
            SYS_CONSOLE_MESSAGE(TERM_RED "COMMAND ERROR : Wrong Command Usage. Check \"azure help\" command\r\n" TERM_RESET);
        }
    }
    return;
}

// *****************************************************************************
// Azure Command Table
//
// Summary:
//    Defines the command table for Azure commands.
//
// Description:
//    This static constant array defines the command table for Azure commands,
//    mapping the "azure" command to the APP_AZURE_CMDProcessing function.
//
// Remarks:
//    None
// *****************************************************************************
static const SYS_CMD_DESCRIPTOR AZURECmdTbl[] =
{
    {"azure", APP_AZURE_CMDProcessing, ": Azure commands processing"},
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
    SYS_WINCS_MQTT_FRAME_t *msg = (SYS_WINCS_MQTT_FRAME_t *)mqttHandle;
            SYS_CONSOLE_PRINT("%s",msg->message);
    
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
        SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n[APP_AZURE]: Press SW0 to send data to Azure Cloud\r\n"TERM_RESET);
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
static void APP_WINCS_azureButtonTelemetry
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
    if (g_buttonPress)
    {
        // Send button press telemetry data
        APP_WINCS_azureButtonTelemetry(++press_count);

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
            SYS_CONSOLE_PRINT(TERM_GREEN"\r\n[APP_AZURE] : MQTT Connected\r\n");
            SYS_CONSOLE_PRINT("[APP_AZURE] : Azure IOT Hub Connection Successful!\r\n"TERM_RESET);
            
            // Set up the MQTT frame with the topic and QoS for subscription
            g_mqttFrame.qos         = SYS_WINCS_MQTT_SUB_TOPIC_3_QOS;
            g_mqttFrame.topic       = SYS_WINCS_MQTT_SUB_TOPIC_3;
            g_mqttFrame.protoVer    = SYS_WINCS_MQTT_PROTO_VERSION;
            
            // Subscribe to the specified MQTT topic
            SYS_CONSOLE_PRINT("[APP_AZURE] : Subscribing to topic : %s\r\n",g_mqttFrame.topic);
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SUBS_TOPIC, (void *)&g_mqttFrame);
            
            // Update the application state to indicate that the cloud connection is up
            g_appAzureCtx.state = APP_AZURE_STATE_CLOUD_UP;
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
//            SYS_WINCS_MQTT_FRAME_t *temp = (SYS_WINCS_MQTT_FRAME_t *)mqttHandle;
//            SYS_CONSOLE_PRINT("MSG;%s\r\n",temp->message);
            
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
            SYS_CONSOLE_PRINT(TERM_RED"[APP_AZURE] : MQTT - Reconnecting...\r\n"TERM_RESET);
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONNECT, (void *)&g_mqttCfg); 
            break;
        }
         
        default:
            break;
    }
    return SYS_WINCS_PASS;
}


/*EIC user handler function*/
static void APP_WINCS_MQTT_EicUserHandler(uintptr_t context)
{
    g_buttonPress = 1;
}


// *****************************************************************************
// *****************************************************************************
// Function: APP_AZURE_Initialize
//
// Summary:
//    Initializes the Azure application.
//
// Description:
//    This function initializes the Azure application, registers the Azure command
//    group, and sets the initial state of the Azure application.
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
void APP_AZURE_Initialize(void)
{
    // Register the Azure command group
    if (!SYS_CMD_ADDGRP(AZURECmdTbl, sizeof(AZURECmdTbl)/sizeof(*AZURECmdTbl), "azure", ": azure commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create AZURE Commands\r\n");
    }

    // Set the initial state of the Azure application
    g_appAzureCtx.state = APP_AZURE_STATE_INIT;
}


// *****************************************************************************
// Function: APP_AZURE_Tasks
//
// Summary:
//    Manages the Azure application tasks.
//
// Description:
//    This function defines the state machine for the Azure application and
//    manages the tasks based on the current state.
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

void APP_AZURE_Tasks
(
    void
)
{
    switch( g_appAzureCtx.state)
    {
        case APP_AZURE_STATE_INIT:
        {
            EIC_CallbackRegister(EIC_PIN_15,APP_WINCS_MQTT_EicUserHandler, 0);
            break;
        }
        
        case APP_AZURE_STATE_START:
        {
            if(true == APP_WINCS02_GetWifiStatus())
            {
                if(true == APP_WINCS02_GetSntpStatus())
                {
                    // Set the callback function for MQTT events
                    SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_SET_CALLBACK, APP_MQTT_Callback);
                    #ifdef AZURE_EXTENDED_COMMAND_ENABLE
                    if(g_mqttCfg.url == NULL)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT broker not configured. using default %s \r\n",SYS_WINCS_MQTT_CLOUD_URL);
                        g_mqttCfg.url = SYS_WINCS_MQTT_CLOUD_URL;
                    }

                    if(g_mqttCfg.clientId == NULL)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT clientId not configured. using default %d \r\n",SYS_WINCS_MQTT_CLIENT_ID);
                        g_mqttCfg.clientId = SYS_WINCS_MQTT_CLIENT_ID;
                    }

                    if(g_mqttCfg.username == NULL)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT username not configured. using default %s \r\n",SYS_WINCS_MQTT_CLOUD_USER_NAME);
                        g_mqttCfg.username = SYS_WINCS_MQTT_CLOUD_USER_NAME;
                    }

                    if(g_mqttCfg.password == NULL)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT Password not configured. using default %s \r\n",SYS_WINCS_MQTT_PASSWORD);
                        g_mqttCfg.password = SYS_WINCS_MQTT_PASSWORD;
                    }

                    if(g_mqttCfg.port == 0)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT port not configured. using default %d \r\n",SYS_WINCS_OTA_ROOT_CERT);
                        g_mqttCfg.port = SYS_WINCS_MQTT_CLOUD_PORT;
                    }

                    if(g_mqttCfg.protoVer == 0)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT Protocol Version not configured. using default %d \r\n",SYS_WINCS_OTA_ROOT_CERT);
                        g_mqttCfg.protoVer = SYS_WINCS_MQTT_PROTO_VERSION;
                    }
                    
                    if(g_mqttCfg.keepAliveTime == 0)
                    {
                        SYS_CONSOLE_PRINT("[APP_MQTT] :MQTT keepAliveTime not configured. using default %d \r\n",SYS_WINCS_OTA_ROOT_CERT);
                        g_mqttCfg.keepAliveTime = SYS_WINCS_MQTT_KEEP_ALIVE_TIME;
                    }
                    g_mqttCfg.cleanSession = true;
                    #endif
                    // Configure the MQTT service with the provided configuration
                    SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONFIG, (void *)&g_mqttCfg);
                    
                    g_appAzureCtx.state = APP_AZURE_STATE_CONNECT_CLOUD;
                    break;
                }
                else
                {
                    SYS_CONSOLE_PRINT(TERM_RED"[APP_AZURE] : SNTP not UP\r\n"TERM_RESET);
                    SYS_CONSOLE_PRINT(TERM_YELLOW"[APP_AZURE] : Try again after SNTP is UP\r\n"TERM_RESET);
                }
            }
            else
            {
                SYS_CONSOLE_PRINT(TERM_RED"[APP_AZURE] : Wifi Not connected\r\n"TERM_RESET);
                SYS_CONSOLE_PRINT(TERM_YELLOW"[APP_AZURE] : Try again after Wi-Fi is connected\r\n"TERM_RESET);
            }
            
            g_appAzureCtx.state = APP_AZURE_STATE_WAIT;
            break;
        }
        
        case APP_AZURE_STATE_CONNECT_CLOUD:
        {
            SYS_CONSOLE_PRINT(TERM_YELLOW"[APP_AZURE] : Connecting to Azure cloud : %s\r\n"TERM_RESET,g_mqttCfg.url);
            // Connect to the MQTT broker using the specified configuration
            if( SYS_WINCS_PASS != SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_CONNECT,(void *)&g_mqttCfg))
            {
                g_appAzureCtx.state = APP_AZURE_STATE_ERROR;
                break;
            }
            
            g_appAzureCtx.state = APP_AZURE_STATE_WAIT;
            break;
        }
        
        case APP_AZURE_STATE_CLOUD_UP:
        {
            APP_WINCS_AzureTask();
            break;
        }
        
        case APP_AZURE_STATE_STOP:
        {
            SYS_WINCS_MQTT_SrvCtrl(SYS_WINCS_MQTT_DISCONNECT, NULL);
            g_appAzureCtx.state = APP_AZURE_STATE_ERROR;
            break;
        }
        
        case APP_AZURE_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT(TERM_RED"[ERROR_AZURE] :ERROR in Azure tasks\r\n"TERM_RESET);
            g_appAzureCtx.state = APP_AZURE_STATE_WAIT;
            break;
        }
        
        case APP_AZURE_STATE_DONE:
        {
            break;
        }
        
        case APP_AZURE_STATE_WAIT:
        {
            
            break;
        }
    }
}