/*******************************************************************************
  WINCS Host Assisted Wifi Service Header file 

  File Name:
    sys_wincs_wifi_service.h

  Summary:
    Header file for the WINCS Host Assisted Wifi Service implementation.

  Description:
    This file contains the header file for the WINCS Host Assisted Wifi Service
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
#ifndef SYS_WINCS_WIFI_SERVICE_H
#define	SYS_WINCS_WIFI_SERVICE_H

#include <xc.h> // include processor files - each processor file is guarded.
#include <stdbool.h>
#include "driver/driver_common.h"
#include "driver/wifi/wincs02/include/wdrv_winc_common.h"


#define SYS_WINCS_WIFI_DBG_MSG(args, ...)      SYS_CONSOLE_PRINT("[WIFI]:"args, ##__VA_ARGS__)

#define SYS_WINCS_SSID_LEN_MAX       33
#define SYS_WINCS_BSSID_LEN_MAX      32
#define SYS_WINCS_PWD_LEN_MAX        128

/*Wi-Fi max Callback service*/
#define SYS_WINCS_WIFI_SERVICE_CB_MAX     2
#define SYS_WINCS_PING_TYPE 4


typedef enum
{      
    /**<Success*/
    SYS_WINCS_PASS =  0x0000, 
            
    /**<Failure*/
    SYS_WINCS_FAIL =  -1, 
            
    /**<RAW mode*/
    SYS_WINCS_RAW = -2,  
            
    /**<Retry*/ 
    SYS_WINCS_COTN =  -3, 
            
    /**<Busy*/
    SYS_WINCS_BUSY = -4, 
            
    /**<Timeout*/
    SYS_WINCS_TIMEOUT = -5,
            
}SYS_WINCS_RESULT_t;


/**
 * @brief Wi-Fi Service List
 * 
 */
typedef enum 
{
    /**<Request Driver version */ 
    SYS_WINCS_WIFI_GET_DRV_STATUS,
            
    /**<Request OPen Driver */ 
    SYS_WINCS_WIFI_OPEN_DRIVER,
            
    /**<Request Driver Handle */ 
    SYS_WINCS_WIFI_GET_DRV_HANDLE,
      
    /* Get Time */
    SYS_WINCS_WIFI_GET_TIME,
      
    /* Set SNTP Conf */
    SYS_WINCS_WIFI_SET_SNTP,
    /**<Request/Trigger Wi-Fi connect */
    SYS_WINCS_WIFI_STA_CONNECT,          
            
    /**<Request/Trigger Wi-Fi disconnect */         
    SYS_WINCS_WIFI_STA_DISCONNECT,       
    /**<Request/Trigger SoftAP disable */        
    SYS_WINCS_WIFI_AP_DISABLE,
            
    /**<Configure the Wi-Fi parameters */          
    SYS_WINCS_WIFI_SET_PARAMS,          
            
    /**<Configure the Wi-Fi channel */        
    SYS_WINCS_WIFI_SET_CHANNEL_AP,   
            
    /**<Configure the Access point's BSSID */        
    SYS_WINCS_WIFI_SET_BSSID,          
            
    /**<Configure Hidden mode SSID in SoftAP mode*/        
    SYS_WINCS_WIFI_SET_HIDDEN,       
            
    /**<Request/Trigger Wi-Fi passive scan */       
    SYS_WINCS_WIFI_PASSIVE_SCAN,     
            
    /**<Request/Trigger Wi-Fi active scan */
    SYS_WINCS_WIFI_ACTIVE_SCAN, 
       
    /* Set Reg Domain */
    SYS_WINCS_WIFI_SET_REG_DOMAIN,
     
    /* Get Reg Domain */
    SYS_WINCS_WIFI_GET_REG_DOMAIN,
    /**<Regester the call back for async events */        
    SYS_WINCS_WIFI_SET_CALLBACK,     
            
    /**<Get Callback functions data*/        
    SYS_WINCS_WIFI_GET_CALLBACK,
            
    /**<Regester the call back for async events */
    SYS_WINCS_WIFI_SET_SRVC_CALLBACK, 
            
    /**<Regester the call back for async events */
    SYS_WINCS_WIFI_AP_ENABLE,
      
    /* DNS Resolve */
    SYS_WINCS_WIFI_DNS_RESOLVE,
            
}SYS_WINCS_WIFI_SERVICE_t;

/**
 * @brief Wi-Fi Event Code List
 * 
 */
typedef enum 
{
    /**<Wi-Fi connected event code*/
    SYS_WINCS_CONNECTED,
            
    /**<Wi-Fi disconnected event code*/
    SYS_WINCS_DISCONNECTED,          
            
    /**<Wi-Fi connection failure event code*/       
    SYS_WINCS_CONNECT_FAILED,        
     
    /**<Wi-Fi DHCP complete event code*/
    SYS_WINCS_DHCP_DONE, 

    /**<Wi-Fi DHCP complete event code*/
    SYS_WINCS_DHCP_IPV6_LOCAL_DONE,
    
	    /**<Wi-Fi DHCP complete event code*/
    SYS_WINCS_DHCP_IPV6_GLOBAL_DONE,
	
    /**<Scan indication event to report each scan list */
    SYS_WINCS_SCAN_INDICATION, 
            
    /**<Scan complete event code*/
    SYS_WINCS_SCAN_DONE,      
    
    /**<SNTP up event*/
    SYS_WINCS_SNTP_UP,
            
    SYS_WINCS_DNS_RESOLVED,
            
    SYS_WINCS_WIFI_FAILED,

}SYS_WINCS_WIFI_EVENT_t;

/**
 * @brief Wi-Fi security modes
 * 
 */
typedef enum 
{
    /**<OPEN mode, no security*/
    SYS_WINCS_OPEN,              
           
    /**<RSVD mode*/
    SYS_WINCS_RSVD,                  
            
    /**<WPA2 Mixed mode TKIP/CCMP*/
    SYS_WINCS_WPA2_MIXED,            
            
    /**<WPA2 CCMP*/
    SYS_WINCS_WPA2,                  
            
    /**<WPA3 Transition Mode*/
    SYS_WINCS_WPA3_TRANS,            
            
    /**<WPA3 mode*/
    SYS_WINCS_WPA3,   

    /**<WPA2/WPA3 mode*/
    SYS_WINCS_WPA2WPA3_PERSONAL,
}SYS_WINCS_WIFI_SECURITY_t;

/**
 * @brief Wi-Fi operation modes
 * 
 */
typedef enum 
{
    /**<Station (STA) mode of WiFi operation*/
    SYS_WINCS_WIFI_MODE_STA, 
            
    /**<Software Access Point (SoftAP) mode of WiFi operation*/
    SYS_WINCS_WIFI_MODE_AP,

}SYS_WINCS_WIFI_MODE_t;

/**
 * @brief       Wi-Fi operation modes
 * 
 */
typedef struct
{
    /**<Wi-Fi operation mode ::SYS_WINCS_WIFI_MODE_t either STA or SoftAP */
    SYS_WINCS_WIFI_MODE_t mode;      
    
    /**<Wi-Fi SSID of Home AP or SoftAP */
    const char  *ssid;              
    
    /**<Wi-Fi Passphrase of Home AP or SoftAP*/
    const char  *passphrase;   
    
    /**<Wi-Fi Secrity mode ::SYS_WINCS_WIFI_SECURITY_t */
    SYS_WINCS_WIFI_SECURITY_t security;  
    
    /**<Wi-Fi autoconnect, SoftAP */
    uint8_t autoconnect;   
    
    uint8_t channel;
    
    bool ssidVisibility;
             
}SYS_WINCS_WIFI_PARAM_t;


/* Wifi Scan Prams*/
typedef struct 
{
    uint8_t channel;
    
    uint16_t scanTime;
    
}SYS_WIFI_WINC_SCAN_PARAM_t;

/**
 * @brief       Wi-Fi callback function type
 * 
 * @param[out]  event One of the ::SYS_WINCS_WIFI_EVENT_t event   
 * @param[out]  msg Received data related to the passed event   
 * 
 */
typedef void (*SYS_WINCS_WIFI_CALLBACK_t)(SYS_WINCS_WIFI_EVENT_t event, uint8_t *msg);

/**
 * @brief Wi-Fi Service Layer API to handle STA and SoftAP mode operations.
 * 
 * @note The asynchronous events are reported through callback, make sure that
 *  the application registers the callback using the ::SYS_WINCS_WIFI_SET_CALLBACK
 *  service request
 *
 * @param[in] request   Service request ::SYS_WINCS_WIFI_SERVICE_t
 * @param[in] input     Parameters for the requested service
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_WIFI_SrvCtrl( SYS_WINCS_WIFI_SERVICE_t request, void *input);

SYS_WINCS_RESULT_t SYS_WINCS_WIFI_GetWincsStatus( WDRV_WINC_STATUS status);

#endif	/* WINCS_WIFI_SERVICE_H */

/** @}*/
