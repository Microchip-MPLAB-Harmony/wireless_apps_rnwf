# Cloud Service

The Cloud service provides an Application Programming Interface \(API\) to manage MQTT functionalities. These functionalities include, configuring the MQTT settings, connecting, disconnecting and reconnecting to the MQTT broker, publishing, subscribing and setting callbacks. The MQTT also provides support for the Azure Device Provisioning Service \(DPS\). The Azure DPS implementation in the service layer simplifies device connectivity with Azure IoT HUB. The user application only provides the IoT central connection parameters and the final IoT HUB connection status is reported through the registered callback function.

**MQTT System Service Configuration in MCC**

![](images\GUID-488B7383-795F-4030-9D62-F712476FC053-low.png "Cloud Service Configuration")

This section allows MQTT service basic configuration as mentioned below:

-   **Cloud URL:** Configure Cloud provider endpoint / MQTT Broker URL.
-   **Cloud Port** : Configure Cloud/MQTT port.
-   **Azure DPS:** Select to enable Azure DPS option \(applicable only when Azure endpoint is used\)
    -   **Scope ID:** Provide Scope ID from Azure IoT Central portal for registered device
    -   **Device Template:** Configure Azure DPS specific device template

-   **User Name and Password:** Configure cloud client credentials.
-   **Client ID:** Device ID registered with cloud provider.
-   **Publish:** Select to enable/ MQTT Publish option. If enabled, it offers related configurations such as Publish Topic Name, Pub QoS, Retain Flag.

-   **Keep Alive:** Select to enable Keep Alive MQTT specific option.
    -   **Keep Alive Interval:** Configure the field in the range of 1-1000 \(in seconds\)
-   **Subscribe:** Select to enable MQTT Subscribe option. If enabled, it provides subscribe specific configurations such as Total Subscribe Topics, Table for Subscribe Topics, Sub. QoS
-   **TLS:** Select to enable TLS Configuration option. If enabled, it will further prompt to enter details as below:
    -   **Server Certificate**
    -   **Device Certificate**
    -   **Device Key**
    -   **Device Key Password**
    -   **Server Name**
    -   **Domain Name**

The MQTT service API example is as follows:<br />

``` {#CODEBLOCK_OLC_5TV_XYB .language-c}
SYS_RNWF_RESULT_t SYS_RNWF_MQTT_SrvCtrl( SYS_RNWF_MQTT_SERVICE_t request, void *input)
```

It handles following services and reports the result to application over the return code or through the registered callback:

|Service|Input|Description|
|-------|-----|-----------|
|`SYS_RNWF_MQTT_CONFIG`|[Broker URL, Port, Client ID, Username, TLS configuration](GUID-C83AB35B-80E7-47A9-BE31-AA8721DEFC14.md)|Configures the MQTT server details along with the corresponding<br /> TLS configurations|
|`SYS_RNWF_MQTT_CONNECT`|None|Initiates the MQTT connection to the configured MQTT<br /> broker|
|`SYS_RNWF_MQTT_RECONNECT`|None|Triggers the re-connection to the configured MQTT broker|
|`SYS_RNWF_MQTT_DISCONNECT`|None|Disconnects from the connected MQTT broker|
|`SYS_RNWF_MQTT_SUBSCRIBE_QOS0`|Subscribe topic \(String\)|Subscribes to the given subscribe topic with QoS0|
|`SYS_RNWF_MQTT_SUBSCRIBE_QOS1`|Subscribe topic \(String\)|Subscribes to the given subscribe topic with QoS1|
|`SYS_RNWF_MQTT_SUBSCRIBE_QOS2`|Subscribe topic \(String\)|Subscribes to the given subscribe topic with QoS2|
|`SYS_RNWF_MQTT_PUBLISH`|[New, QOS, Retain, topic, message](GUID-C83AB35B-80E7-47A9-BE31-AA8721DEFC14.md)|Publish the message on given publish topic and<br /> configuration|
|`SYS_RNWF_MQTT_SET_CALLBACK`|Callback Function Handler|Registers the MQTT callback to report the status to user<br /> application|

The following list captures the MQTT callback event codes and their arguments

|Event|Response Components|Comments|
|-----|-------------------|--------|
|`SYS_RNWF_MQTT_CONNECTED`|None|Reported once connected to MQTT broker|
|`SYS_RNWF_MQTT_DISCONNECTED`|None|Event to report the MQTT broker disconnection|
|`SYS_RNWF_MQTT_SUBCRIBE_MSG`|[dup, QoS, retain, topic, payload](GUID-C83AB35B-80E7-47A9-BE31-AA8721DEFC14.md)|Reports the received payload for the subscribed topic|
|`SYS_RNWF_MQTT_SUBCRIBE_ACK`|Integer string|Subscribe ack return code|
|`SYS_RNWF_MQTT_DPS_STATUS`|Integer|Azure DPS status :-  1 for success and 0 for failure |

The sequence chart below explains this process.

![](images\GUID-A740DF32-02DA-489A-A6E7-79A9E77118BA-low.png "MQTT (Azure) Connection Sequence")

MQTT Publish

<br />

User application can publish to the MQTT broker by creating the MQTT frame and then sending the frame using the API. The sequence chart is illustrated below.

``` {#CODEBLOCK_QX4_WDX_MZB .language-c}
SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_PUBLISH, (void *)&mqtt_pub)
```

<br />

![](images\GUID-D08134E8-FE53-431E-8A6B-81DE74A90FFA-low.png "MQTT Publish Sequence")

<br />

<br />

MQTT Subscribe

The sequence for subscribing to a topic from the MQTT Broker is illustrated below. The user application needs to use the API to subscribe to the topic with the appropriate QoS value.

``` {#CODEBLOCK_LWS_F2X_MZB .language-c}
SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS0, buffer) 
```

<br />

![](images\GUID-2D3B0685-B597-465B-83DD-93AC83012252-low.png "MQTT Subscribe Sequence")

<br />

An example of the MQTT application provided below showcases the use of MQTT service<br /> API's:

Some of the configurations con be configured by the user by MCC.<br />

``` {#CODEBLOCK_LSY_D4K_JYB .language-c}
/*
    MQTT application
*/
#define CLOUD_STATE_MACHINE()         APP_RNWF_AZURE_Task()
#define CLOUD_SUBACK_HANDLER()        APP_RNWF_azureSubackHandler()
#define CLOUD_SUBMSG_HANDLER(msg)     SYS_RNWF_APP_AZURE_SUB_Handler(msg)

/* Variable to check the UART transfer */
static volatile bool g_isUARTTxComplete = true,isUART0TxComplete = true;;
    
/*Shows the he application's current state*/
static APP_STATE_t g_appState = APP_SYS_INIT;

/* Keeps the device IP address */
static uint8_t g_devIp[16];

/*Shows the he application's current state*/
static APP_DATA1 g_appData;

/*Application buffer to store data*/
static uint8_t g_appBuf[SYS_RNWF_BUF_LEN_MAX];


/**TLS Configuration for the Azure */
const char *g_tlsCfgAzure[] = {SYS_RNWF_NET_ROOT_CERT, SYS_RNWF_NET_DEVICE_CERTIFICATE, SYS_RNWF_NET_DEVICE_KEY, SYS_RNWD_NET_DEVICE_KEY_PWD, SYS_RNWF_NET_SERVER_NAME};
         

/*MQTT Configurations for azure*/
SYS_RNWF_MQTT_CFG_t mqtt_cfg = {
    .url = SYS_RNWF_MQTT_CLOUD_URL,
    .username = SYS_RNWF_MQTT_CLOUD_USER_NAME,    
    .clientid = SYS_RNWF_MQTT_CLIENT_ID,
    .password = SYS_RNWF_MQTT_PASSWORD,
    .port = SYS_RNWF_MQTT_CLOUD_PORT,
    .tls_conf = (uint8_t *) g_tlsCfgAzure,
    .tls_idx = SYS_RNWF_NET_TLS_CONFIG_2,
    .azure_dps = SYS_RNWF_MQTT_AZURE_DPS_ENABLE
};

/* APP Cloud Telemetry Rate in seconds */
static uint16_t g_reportRate = APP_CLOUD_REPORT_INTERVAL;

/*Azure app buffer*/
uint8_t azure_app_buf[SYS_RNWF_BUF_LEN_MAX];

/**Azure IoT HUB subscribe list */
static const char *g_subscribeList[] = {AZURE_SUB_TWIN_RES, AZURE_SUB_METHODS_POST, AZURE_SUB_TWIN_PATCH,  NULL};

/* System Tick Counter for 1mSec*/
static uint32_t g_sysTickCount;

/*Azure subscribe count*/
static uint8_t g_subCnt;

/* Button Press Event */
static bool    g_buttonPress = false;

/*MQTT data publish function*/
static SYS_RNWF_RESULT_t APP_RNWF_mqttPublish(const char *top, const char *msg)
{    
    SYS_RNWF_MQTT_FRAME_t mqtt_pub;    
    mqtt_pub.isNew = SYS_RNWF_NEW_MSG;
    mqtt_pub.qos = SYS_RNWF_MQTT_QOS0;
    mqtt_pub.isRetain = SYS_RNWF_NO_RETAIN;
    mqtt_pub.topic = top;
    mqtt_pub.message = msg;        
    return SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_PUBLISH, (void *)&mqtt_pub);              
} 

/*Function to send the button press count data*/
static void APP_RNWF_azureButtonTelemetry(uint32_t press_count)
{            
    snprintf((char *)azure_app_buf, sizeof(azure_app_buf), (const char *) AZURE_FMT_BUTTON_TEL, press_count);
    SYS_CONSOLE_PRINT("Telemetry ->> buttonEvent count %d\r\n", press_count);
    APP_RNWF_mqttPublish((const char *)SYS_RNWF_MQTT_PUB_TELEMETRY,(const char *) azure_app_buf);
}


/*Function to send the counter data*/
static void APP_RNWF_azureCounterTelemetry(uint32_t counter)
{            
    snprintf((char *)azure_app_buf, sizeof(azure_app_buf),(const char *) AZURE_FMT_COUNTER_TEL, counter);
    SYS_CONSOLE_PRINT("Telemetry ->> counter count %d\r\n", counter);
    APP_RNWF_mqttPublish((const char *)SYS_RNWF_MQTT_PUB_TELEMETRY,(const char *) azure_app_buf);
}

/*Azure subscribe acknowledgement handler*/
static void APP_RNWF_azureSubackHandler()
{    
    if(g_subscribeList[g_subCnt] != NULL)
    {
        sprintf((char *)azure_app_buf, "%s", (const char *)g_subscribeList[g_subCnt++]);
        SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS0, azure_app_buf);            
    }
    else
    {        
        // get device twin
        APP_RNWF_mqttPublish(AZURE_PUB_TWIN_GET, "");
    }
}


static void APP_RNWF_eicUserHandler(uintptr_t context)
{     
    g_buttonPress = 1;
}

/*Azure initialization function*/
static void APP_RNWF_AZURE_INIT()
{
    EIC_CallbackRegister(EIC_PIN_15,APP_RNWF_eicUserHandler, 0);
}


/*Function to handle azure tasks*/
static void APP_RNWF_AZURE_Task(void)
{
    static uint32_t press_count = 0;
    static uint32_t counter = 0;
   
    if(!(g_sysTickCount % APP_SYS_TICK_COUNT_1SEC))
    {     
        if(!(g_sysTickCount % g_reportRate))
        {
            APP_RNWF_azureCounterTelemetry(++counter);
            
            APP_RNWF_azureButtonTelemetry(press_count);
        }                   
    }  
    
    if(g_buttonPress)
    {        
        APP_RNWF_azureButtonTelemetry(++press_count);
        g_buttonPress = 0;
    }
         
    
    if(!g_subCnt && g_subscribeList[g_subCnt] != NULL)
    {
        sprintf((char *)azure_app_buf, "%s", (const char *)g_subscribeList[g_subCnt++]);
        SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SUBSCRIBE_QOS0, azure_app_buf);            
    }        
                 
}

/*Azure subscribe handler function*/
void SYS_RNWF_APP_AZURE_SUB_Handler(char *p_str)
{
    char *end_ptr = NULL;
    if(strstr(p_str, "twin/res/200"))
    {
        sprintf((char *)azure_app_buf, "{"AZURE_MSG_IPADDRESS"}",(const char *) APP_GET_IP_Address());
        APP_RNWF_mqttPublish((const char *)AZURE_PUB_PROPERTY,(const char *) azure_app_buf);

    }                        

    if(strstr(p_str, "POST") != NULL)
    {
        char *echo_ptr = (char *)strstr(p_str, AZURE_ECHO_TAG);   
        char *rid_ptr = (char *)strstr(p_str, "rid="); 
        if(rid_ptr != NULL)
        {
            end_ptr = (char *)strstr(rid_ptr, "\" \"");
            *end_ptr = '\0';
            //+1 for null character
            uint16_t pubLen = sprintf((char *)azure_app_buf, AZURE_PUB_CMD_RESP,(const char *) rid_ptr) + 1;             
            if(echo_ptr != NULL)
            {                 
                echo_ptr += strlen(AZURE_ECHO_TAG);
                end_ptr = (char *)strstr(echo_ptr, "\\\"}");
                *end_ptr = '\0';            
                SYS_CONSOLE_PRINT("Echo = %s\r\n", echo_ptr);
                sprintf(( char *)azure_app_buf+pubLen, AZURE_FMT_ECHO_RSP,(const char *) echo_ptr);                
                APP_RNWF_mqttPublish((const char *)azure_app_buf, (const char *)azure_app_buf+pubLen);
            }
        }                
    }    
}


/* DMAC Channel Handler Function */
static void APP_RNWF_usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
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
SYS_RNWF_RESULT_t SYS_RNWF_MqttCallbackHandler(SYS_RNWF_MQTT_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        /* MQTT connected event code*/
        case SYS_RNWF_MQTT_CONNECTED:
        {    
            SYS_CONSOLE_PRINT("MQTT : Connected\r\n");
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
void SYS_RNWF_WIFI_CallbackHandler(SYS_RNWF_WIFI_EVENT_t event, uint8_t *p_str)
{
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_RNWF_SNTP_UP:
        {            
                SYS_CONSOLE_PRINT("SNTP UP:%s\n", &p_str[2]);  
                SYS_CONSOLE_PRINT("Connecting to the Cloud\r\n");
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_SET_CALLBACK, SYS_RNWF_MqttCallbackHandler);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONFIG, (void *)&mqtt_cfg);
                SYS_RNWF_MQTT_SrvCtrl(SYS_RNWF_MQTT_CONNECT, NULL);
                break;
        }
        
        /* Wi-Fi connected event code*/
        case SYS_RNWF_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected\r\n");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_RNWF_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_DHCP_DONE:
        {
            SYS_CONSOLE_PRINT("DHCP IP:%s\r\n", &p_str[2]); 
            strncpy((char *)g_devIp,(const char *) &p_str[3], strlen((const char *)(&p_str[3]))-1);
            break;
        }
        
        /* Wi-Fi scan indication event code*/
        case SYS_RNWF_SCAN_INDICATION:
        {
            break;
        }
        
        /* Wi-Fi scan complete event code*/
        case SYS_RNWF_SCAN_DONE:
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
void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    g_appData.state = APP_STATE_INITIALIZE;
}


/* Maintain the application's state machine.*/
void APP_Tasks ( void )
{
    switch(g_appData.state)
    {
        /* Application's state machine's initial state. */
        case APP_STATE_INITIALIZE:
        {
            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, APP_RNWF_usartDmaChannelHandler, 0);
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
            SYS_CONSOLE_PRINT("Certs on RNWF02:- \r\n%s\r\n\r\n", g_appBuf);
            
            SYS_CONSOLE_Tasks(sysObj.sysConsole0);
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_GET_KEY_LIST, g_appBuf);    
            SYS_CONSOLE_PRINT("Keys on RNWF02:- \r\n%s\r\n\r\n", g_appBuf);
            
            char sntp_url[] =  "0.in.pool.ntp.org";    
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SET_SNTP,sntp_url);
             
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SW_REV, g_appBuf);    
            SYS_CONSOLE_PRINT("Software Revision:- %s\r\n", g_appBuf);
            
              
            /* RNWF Application Callback register */
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, SYS_RNWF_WIFI_CallbackHandler);
          
            /* Wi-Fi Connectivity */
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
```
