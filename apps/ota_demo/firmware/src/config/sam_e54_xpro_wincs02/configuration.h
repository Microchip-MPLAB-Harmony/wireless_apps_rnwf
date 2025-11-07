/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************
/*----------------- WINCS Net System Service Configuration -----------------*/
#define SYS_WINCS_NET_BIND_TYPE0                SYS_WINCS_NET_BIND_LOCAL
#define SYS_WINCS_NET_SOCK_TYPE0                SYS_WINCS_NET_SOCK_TYPE_TCP 
#define SYS_WINCS_NET_SOCK_TYPE_IPv4_0          4
#define SYS_WINCS_NET_SOCK_TYPE_IPv6_LOCAL0     0
#define SYS_WINCS_NET_SOCK_TYPE_IPv6_GLOBAL0    0
#define SYS_WINCS_NET_SOCK_PORT0                80
#define SYS_WINCS_TLS_ENABLE0                   1
#define SYS_WINCS_NET_PEER_AUTH0                true
#define SYS_WINCS_NET_ROOT_CERT0                "ca-cert"
#define SYS_WINCS_NET_DEVICE_CERTIFICATE0       "mutual-client-cert"
#define SYS_WINCS_NET_DEVICE_KEY0               "mutual-client"
#define SYS_WINCS_NET_DEVICE_KEY_PWD0           NULL
#define SYS_WINCS_NET_SERVER_NAME0              "192.168.99.67"
#define SYS_WINCS_NET_DOMAIN_NAME_VERIFY0       0
#define SYS_WINCS_NET_DOMAIN_NAME0              ""
/*----------------------------------------------------------------------------*/

/* -----------------WINCS02 MQTT System Service Configuration ----------------- */

#define SYS_WINCS_MQTT_PROTO_VERSION             SYS_WINCS_MQTT_PROTO_VER_3

#define SYS_WINCS_MQTT_CLOUD_URL                 "WIFI-SYS-APPS.azure-devices.net"
#define SYS_WINCS_MQTT_CLOUD_PORT                8883
#define SYS_WINCS_MQTT_CLIENT_ID                 "wincs02_device3_1"
#define SYS_WINCS_MQTT_CLOUD_USER_NAME           "WIFI-SYS-APPS.azure-devices.net/wincs02_device3_1/?api-version=2021-04-12"
#define SYS_WINCS_MQTT_PASSWORD                  ""
#define SYS_WINCS_MQTT_CLEAN_SESSION             true
#define SYS_WINCS_MQTT_KEEP_ALIVE_TIME           60
#define SYS_WINCS_MQTT_SUB_TOPIC_0               "$iothub/twin/res/#"
#define SYS_WINCS_MQTT_SUB_TOPIC_0_QOS           SYS_WINCS_MQTT_QOS0
#define SYS_WINCS_MQTT_SUB_TOPIC_1               "$iothub/methods/POST/#"
#define SYS_WINCS_MQTT_SUB_TOPIC_1_QOS           SYS_WINCS_MQTT_QOS0
#define SYS_WINCS_MQTT_SUB_TOPIC_2               "$iothub/twin/PATCH/properties/desired/#"
#define SYS_WINCS_MQTT_SUB_TOPIC_2_QOS           SYS_WINCS_MQTT_QOS0
#define SYS_WINCS_MQTT_SUB_TOPIC_3               "devices/wincs02_device3_1/messages/devicebound/#"
#define SYS_WINCS_MQTT_SUB_TOPIC_3_QOS           SYS_WINCS_MQTT_QOS0


#define SYS_WINCS_MQTT_PUB_TOPIC_NAME            "devices/wincs02_device3_1/messages/events/"
#define SYS_WINCS_MQTT_PUB_MSG                   "Hi. It's MCHP Wireless Device"
#define SYS_WINCS_MQTT_PUB_MSG_QOS_TYPE          SYS_WINCS_MQTT_QOS0
#define SYS_WINCS_MQTT_PUB_MSG_RETAIN            false
#define SYS_WINCS_MQTT_TLS_ENABLE                true
#define SYS_WINCS_MQTT_PEER_AUTH_ENABLE          true
#define SYS_WINCS_MQTT_SERVER_CERT               "DigiCertGlobalRootG2"
#define SYS_WINCS_MQTT_DEVICE_CERT               "wincs02_device3_1"
#define SYS_WINCS_MQTT_DEVICE_KEY                "wincs02_device3_1"
#define SYS_WINCS_MQTT_DEVICE_KEY_PSK            ""
#define SYS_WINCS_MQTT_TLS_SERVER_NAME           ""
#define SYS_WINCS_MQTT_TLS_DOMAIN_NAME_VERIFY    true
#define SYS_WINCS_MQTT_TLS_DOMAIN_NAME           "*.azure-devices.net"
#define SYS_WINCS_MQTT_AZURE_DPS_ENABLE          false
#define SYS_WINCS_MQTT_CallbackHandler           APP_MQTT_Callback

/*----------------------------------------------------------------------------*/


#define SYS_CMD_ENABLE
#define SYS_CMD_DEVICE_MAX_INSTANCES       SYS_CONSOLE_DEVICE_MAX_INSTANCES
#define SYS_CMD_PRINT_BUFFER_SIZE          1024U
#define SYS_CMD_BUFFER_DMA_READY


/* TIME System Service Configuration Options */
#define SYS_TIME_INDEX_0                            (0)
#define SYS_TIME_MAX_TIMERS                         (5)
#define SYS_TIME_HW_COUNTER_WIDTH                   (24)
#define SYS_TIME_TICK_FREQ_IN_HZ                    (1000)


#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL       SYS_ERROR_INFO
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE


#define SYS_CONSOLE_DEVICE_MAX_INSTANCES   			(1U)
#define SYS_CONSOLE_UART_MAX_INSTANCES 	   			(1U)
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 	   		(0U)
#define SYS_CONSOLE_PRINT_BUFFER_SIZE        		(200U)



/* WINCS02  OTA System Service Configuration Options */

/* OTA- Server Socket Configuration */
#define SYS_WINCS_OTA_SERV_SOCK_BIND_TYPE	          SYS_WINCS_NET_BIND_TYPE0
#define SYS_WINCS_OTA_SERV_SOCK_TYPE	              SYS_WINCS_NET_SOCK_TYPE0  
#define SYS_WINCS_OTA_SERV_SOCK_PORT		          SYS_WINCS_NET_SOCK_PORT0 
#define SYS_WINCS_OTA_SERV_SOCK_TYPE_IPv4             SYS_WINCS_NET_SOCK_TYPE_IPv4_0
#define SYS_WINCS_OTA_SERV_SOCK_TYPE_IPv6_LOCAL       SYS_WINCS_NET_SOCK_TYPE_IPv6_LOCAL0
#define SYS_WINCS_OTA_SERV_SOCK_TYPE_IPv6_GLOBAL      SYS_WINCS_NET_SOCK_TYPE_IPv6_GLOBAL0
#define SYS_WINCS_OTA_SERV_SOCK_TLS_ENABLE            SYS_WINCS_TLS_ENABLE0

#define SYS_WINCS_OTA_FILE_NAME                       "wincs_ota.bin"
#define SYS_WINCS_OTA_TIMEOUT                         20
#define SYS_WINCS_OTA_SERV_SOCK_PEER_AUTH	          SYS_WINCS_NET_PEER_AUTH0
#define SYS_WINCS_OTA_SERV_SOCK_ROOT_CERT	          SYS_WINCS_NET_ROOT_CERT0
#define SYS_WINCS_OTA_SERV_SOCK_DEV_CERT	          SYS_WINCS_NET_DEVICE_CERTIFICATE0
#define SYS_WINCS_OTA_SERV_SOCK_DEV_KEY	              SYS_WINCS_NET_DEVICE_KEY0
#define SYS_WINCS_OTA_SERV_SOCK_DEV_KEY_PWD	          SYS_WINCS_NET_DEVICE_KEY_PWD0
#define SYS_WINCS_OTA_SERV_SOCK_SERVER_NAME           SYS_WINCS_NET_SERVER_NAME0
#define SYS_WINCS_OTA_SERV_SOCK_DOMAIN_NAME_VERIFY	  SYS_WINCS_NET_DOMAIN_NAME_VERIFY0
#define SYS_WINCS_OTA_SERV_SOCK_DOMAIN_NAME	          SYS_WINCS_NET_DOMAIN_NAME0
#define SYS_WINCS_OTA_TLS_ENABLE                      true    
#define SYS_WINCS_TLS_OTA_URL                         "https://%s:%d/%s"   

#define SYS_WINCS_OTA_CallbackHandler   		   	      APP_OTA_Callback

#define SYS_CONSOLE_INDEX_0                       0






// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/*** WiFi WINC Driver Configuration ***/
#define WDRV_WINC_EIC_SOURCE
#define WDRV_WINC_DEVICE_USE_SYS_DEBUG
#define WDRV_WINC_DEV_RX_BUFF_SZ            2048
#define WINC_SOCK_SLAB_ALLOC_MODE           1
#define WDRV_WINC_DEV_SOCK_SLAB_NUM         50
#define WDRV_WINC_DEV_SOCK_SLAB_SZ          1472
#define WINC_SOCK_NUM_SOCKETS               10
#define WINC_SOCK_BUF_RX_SZ                 7360
#define WINC_SOCK_BUF_TX_SZ                 4416
#define WINC_SOCK_BUF_RX_PKT_BUF_NUM        5
#define WINC_SOCK_BUF_TX_PKT_BUF_NUM        5
#define WDRV_WINC_MOD_DISABLE_SYSLOG



// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************
/* WINCS02  WIFIPROV System Service Configuration Options */

#define SYS_WINCS_WIFIPROV_CallbackHandler		APP_WIFIPROV_Callback

/* WINCS02  WIFI System Service Configuration Options */

#define SYS_WINCS_WIFI_DEVMODE        		SYS_WINCS_WIFI_MODE_AP



#define SYS_WINCS_WIFI_PROV_SSID			"DEMO_AP_SOFTAP"
#define SYS_WINCS_WIFI_PROV_PWD        		"password"
#define SYS_WINCS_WIFI_PROV_SSID_VISIBILITY       false
#define SYS_WINCS_WIFI_PROV_SECURITY		SYS_WINCS_WIFI_SECURITY_WPA2
#define SYS_WINCS_WIFI_PROV_MOBILE_APP			1
#define SYS_WINCS_WIFI_PROV_CHANNEL          1
#define SYS_WINCS_WIFI_AP_IP_ADDR            "192.168.1.1"
#define SYS_WINCS_WIFI_AP_IP_POOL_START      "192.168.1.50"
#define SYS_WINCS_WIFI_COUNTRYCODE          "GEN"
#define SYS_WINCS_WIFI_SNTP_ADDRESS          "0.in.pool.ntp.org"


#define SYS_WINCS_WIFI_CallbackHandler	     APP_WIFI_Callback


// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/
