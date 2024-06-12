/*******************************************************************************
  WINCS Host Assisted MQTT Service Header file 

  File Name:
    sys_wincs_mqtt_service.h

  Summary:
    Header file for the WINCS Host Assisted MQTT Service implementation.

  Description:
    This file contains the header file for the WINCS Host Assisted MQTT Service
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.


 * Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */


// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SYS_WINCS_MQTT_SERVICE_H
#define	SYS_WINCS_MQTT_SERVICE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "system/wifi/sys_wincs_wifi_service.h"
#include "driver/wifi/wincs02/include/wdrv_winc_client_api.h"


#define SYS_WINCS_MQTT_DBG_MSG(args, ...)      SYS_CONSOLE_PRINT("[MQTT]:"args, ##__VA_ARGS__)

/*MQTT buffer max length*/
#define SYS_WINCS_MQTT_BUF_LEN_MAX	4096


#define SYS_WINCS_MQTT_PORT_NO_ECN_NO_AUTH        1883
#define SYS_WINCS_MQTT_PORT_NO_ECN_AUTH           1884
#define SYS_WINCS_MQTT_PORT_ECN_NO_AUTH           8883

/*MQTT Service max callback service*/
#define SYS_WINCS_MQTT_SERVICE_CB_MAX        2
#define SYS_WINCS_MQTT_PUBLISH_TOPIC      "MCHP/Sample01"
#define SYS_WINCS_MQTT_PUBLISH_MSG      "Hi MQTT"

/**<Publish message is not saved at broker */
#define SYS_WINCS_MQTT_PUBLISH_MSG_RETAIN   SYS_WINCS_NO_RETAIN
#define SYS_WINCS_MQTT_PUBLISH_MSG_QoS   SYS_WINCS_MQTT_QOS0
#define SYS_WINCS_MQTT_PUBLISH_MSG_TYPE   SYS_WINCS_NEW_MSG

#define SYS_WINCS_MQTT_PROTO_VERSION      WDRV_WINC_MQTT_PROTO_VER_3
#define SYS_WINCS_MQTT_KEEPALIVE_TIME      60


#define SYS_WINCS_MQTT_SUBSCRIBE_TOPIC    "$MCHP/Sample06"
#define SYS_WINCS_MQTT_DPS_SUBSCRIBE_TOPIC    "$dps/registrations/res/#"


#define SYS_WINCS_MQTT_DPS_TOP_SET_REG        "$dps/registrations/PUT/iotdps-register/?$rid=1"
#define SYS_WINCS_MQTT_DPS_MSG_SET_REQ        "{\\\"payload\\\": {\\\"modelId\\\": \\\"dtmi:com:Microchip:AVR128DB48_CNANO;1\\\"}}"


#define SYS_WINCS_MQTT_DPS_TOP_DPS_GET_STAT   "$dps/registrations/GET/iotdps-get-operationstatus/?$rid=2&operationId=%s"
#define SYS_WINCS_MQTT_DPS_MSG_DPS_GET_STAT   ""

//#define SYS_WINCS_MQTT_DPS_SUBSCRIBE_TOPIC    "$dps/registrations/res/#"


#define SYS_WINCS_MQTT_DPS_HUB_ID_STR         "\\\"assignedHub\\\":\\"""
#define SYS_WINCS_MQTT_DPS_DEV_ID_STR         "\\\"deviceId\\\":\\"""        
#define SYS_WINCS_MQTT_DPS_OP_ID_STR          "\\\"operationId\\\":\\"""
#define SYS_WINCS_MQTT_DPS_END_ID_STR         "\\\" \\\""


#define SYS_WINCS_MQTT_IOT_HUB_USERNAME       "%s/%s/?api-version=2021-04-12"

/**
 @defgroup MQTT_GRP MQTT Cloud API
 @{
 */

/**
 @brief Network and Socket service List
 
 */
typedef enum 
{
    /**<Configure the MQTT Broker parameters*/
    SYS_WINCS_MQTT_CONFIG,
    /**<Connect to the MQTT Broker */        
    SYS_WINCS_MQTT_CONNECT,              
            
    /**<Request reconnect to the MQTT Cloud*/        
    SYS_WINCS_MQTT_RECONNECT,        
            
    /**<Trigger Disconnect from MQTT Broker*/        
    SYS_WINCS_MQTT_DISCONNECT,      
    /**<Subscribe to QoS0 Topics */        
    SYS_WINCS_MQTT_SUBS_TOPIC,  
                         
    /**<UNSubscribe to Topic */        
    SYS_WINCS_MQTT_UNSUBSCRIBE, 
    /**<Configure the MQTT Application Callback*/            
    SYS_WINCS_MQTT_SET_CALLBACK,                
            
    /**<Configure the MQTT Application Callback*/
    SYS_WINCS_MQTT_SET_SRVC_CALLBACK,   
            
    /*< Get Callback Function data*/
    SYS_WINCS_MQTT_GET_CALLBACK,
            
}SYS_WINCS_MQTT_SERVICE_t;


/**
 @brief MQTT Application callback events
 
 */
typedef enum
{
    /**<Connected to MQTT broker event */
    SYS_WINCS_MQTT_CONNECTED,    
            
    /**<Disconnected from MQTT broker event*/   
    SYS_WINCS_MQTT_DISCONNECTED, 
            
    /**<Event to report received MQTT message*/   
    SYS_WINCS_MQTT_SUBCRIBE_MSG,  
            
    /*Subscribe MQTT ACK*/        
    SYS_WINCS_MQTT_SUBCRIBE_ACK,
            
    /*MQTT Publish ACK*/
    SYS_WINCS_MQTT_PUBLISH_ACK,  
       
    /*MQTT Publish acknowledgement and completion received. */
    SYS_WINCS_MQTT_PUBLISH_MSG_RECV,
            
    /*MQTT A topic has been un-subscribed.*/
    SYS_WINCS_MQTT_UNSUBSCRIBED,        
            
    /*MQTT DPS Status*/
    SYS_WINCS_MQTT_DPS_STATUS,
            
    /*MQTT ERROR*/
    SYS_WINCS_MQTT_ERROR,
	   
}SYS_WINCS_MQTT_EVENT_t;

/**
 @brief Network and Socket service List
 
 */
typedef struct 
{    
    /**<MQTT Broker/Server URL */    
    const char *url;          
    
    /**<MQTT Service client ID*/
    const char *clientid;
    
    /**<MQTT User Name Credential */
    const char *username;       
    
    /**<MQTT Password Credential */ 
    const char *password;       
    
    /**<MQTT Broker/Server Port */
    uint16_t port;          
    
    /*MQTT TLS Index*/
    uint8_t     tls_idx;
    
    /*Azure DPS*/
    uint8_t     azure_dps;
    
    /*TLS Configuration*/
    uint8_t     *tls_conf; 
    
    /* Protocol version */
    WDRV_WINC_MQTT_PROTO_VER protoVer;
    
    /* MQTT Clean session flag*/
    bool clean_session;
    
    /* MQTT keep alive time*/
    int keep_alive_time;
    
}SYS_WINCS_MQTT_CFG_t;



/**
 @brief Network and Socket service List
 
 */
typedef enum
{
    /**New message*/
    SYS_WINCS_NEW_MSG,         
            
    /**Duplicate message*/         
    SYS_WINCS_DUP_MSG      

}SYS_WINCS_MQTT_MSG_t;

/**
 @brief MQTT Message QoS Type
 
 */
typedef enum
{
    /**<No-Ack, Best effort delivery(No Guarantee)*/          
    SYS_WINCS_MQTT_QOS0,      
            
    /**<Pub-Ack, sent untill PUBACK from broker(possible duplicates) */
    SYS_WINCS_MQTT_QOS1,      
            
    /**<Highest service, no duplicate with guarantee */            
    SYS_WINCS_MQTT_QOS2,   

}SYS_WINCS_MQTT_QOS_t;

/**
 @brief MQTT Message Retain flag
 */
typedef enum
{
    /**<Publish message is not saved at broker */
    SYS_WINCS_NO_RETAIN,          
            
    /**<Publish message is saved at broker */        
    SYS_WINCS_RETAIN,  

}SYS_WINCS_MQTT_RETAIN_t;

/**
 @brief MQTT Publish Frame format
 
 */
typedef struct
{
    /**<Indicates message is new or duplicate */
    SYS_WINCS_MQTT_MSG_t isNew;          
    
    /**<QoS type for the message ::SYS_WINCS_MQTT_QOS_t */
    SYS_WINCS_MQTT_QOS_t qos;         
    
    /**<Retain flag for the publish message */
    SYS_WINCS_MQTT_RETAIN_t isRetain;    
    
    /**<Publish topic for the message */
    const char *topic;           
    
    /**<Indicates message is new or duplicate */
    const char *message; 
                       
}SYS_WINCS_MQTT_FRAME_t;



typedef struct
{
    const char *topic_name; 
    
    const char  *message;
    
    WDRV_WINC_MQTT_MSG_INFO msg_info;
    
}SYS_WINCS_MQTT_LWT_CFG_t;

/**
 @brief MQTT Callback Function definition
 
 */
typedef SYS_WINCS_RESULT_t (*SYS_WINCS_MQTT_CALLBACK_t)(SYS_WINCS_MQTT_EVENT_t, uint8_t *);


/**
 * @brief MQTT Service Layer API to handle system operations.
 * 
 *
 * @param[in] request       Requested service ::SYS_WINCS_MQTT_SERVICE_t
 * @param[in] input         Input/Output data for the requested service 
 * 
 * @return SYS_WINCS_PASS Requested service is handled successfully
 * @return SYS_WINCS_PASS Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_MQTT_SrvCtrl( SYS_WINCS_MQTT_SERVICE_t request, void *input);

#endif	/* XC_HEADER_TEMPLATE_H */

/** @}*/