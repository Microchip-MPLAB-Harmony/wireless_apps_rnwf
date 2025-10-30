/*******************************************************************************
  WINCS02 Internal Ota service Header file

  File Name:
    sys_wincs_ota_service.h

  Summary:
    Header file for the WINCS02 Internal Ota service implementation.

  Description:
    This header file provides a simple APIs to enable Internal Ota service with WINCS02 device 
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
#ifndef SYS_WINCS_OTA_SERVICE_H
#define    SYS_WINCS_OTA_SERVICE_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <xc.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


/* This section lists the other files that are included in this file.
 */

#include "configuration.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "driver/wifi/wincs02/include/wdrv_winc_client_api.h"
#include "driver/wifi/wincs02/include/wdrv_winc.h"


/* Handle for OTA Configurations */
typedef void * SYS_WINCS_OTA_HANDLE_t;

/* Debug print for Ota service */
#define SYS_WINCS_OTA_DBG_MSG(args, ...)      SYS_CONSOLE_PRINT("[OTA]:"args, ##__VA_ARGS__)

/* OTA FW Download URL MAX SIZE*/
#define SYS_WINCS_OTA_URL_MAX_SIZE         100

// *****************************************************************************
// OTA Options
//
// Summary:
//    Structure containing OTA options.
//
// Description:
//    Contains OTA options.
//
// Remarks:
//    None.
// *****************************************************************************
typedef struct
{
  /* OTA timeout: If a data packet is not received within mentioned seconds 
   * or mentioned second gap occurs between packets, then the OTA process will be stopped  */
    uint8_t timeout;

} SYS_WINCS_OTA_OPTIONS;


// *****************************************************************************
// OTA Configuration Parameters
//
// Summary:
//    Identifies OTA Configuration parameters.
//
// Description:
//    This structure defines the configuration parameters for OTA, including URL,
//    TLS enable flag, OTA options, and TLS context handle.
//
// Remarks:
//    None.
// *****************************************************************************
typedef struct 
{
    /* OTA configuration parameter URL */
    char                       url[SYS_WINCS_OTA_URL_MAX_SIZE];

    /* OTA configuration parameter TLS enable flag */  
    bool                       tlsEnable;

    /* OTA options */
    SYS_WINCS_OTA_OPTIONS      options;

    /* TLS context handle */
    WDRV_WINC_TLS_HANDLE       tlsCtxHandle;

} SYS_WINCS_OTA_CFG_t;

// *****************************************************************************
// OTA TLS Configuration Parameters
//
// Summary:
//    Identifies OTA TLS Configuration parameters.
//
// Description:
//    This structure defines the TLS configuration parameters for OTA, including
//    peer authentication, CA certificate, certificate name, key name, key password,
//    server name, domain name, and domain name verification.
//
// Remarks:
//    None.
// *****************************************************************************
typedef struct 
{
    /**< TLS Peer authentication */
    bool        tlsPeerAuth;             

    /**< TLS CA Certificate */
    char        *tlsCACertificate;       

    /**< TLS Certificate Name */         
    char        *tlsCertificate;         

    /**< TLS Key name  */            
    char        *tlsKeyName;             

    /**< TLS Key password  */
    char        *tlsKeyPassword;         

    /**< TLS Server name  */
    char        *tlsServerName;          

    /**< TLS Domain Name */
    char        *tlsDomainName;          

    /**< TLS Domain Name Verify */
    bool        tlsDomainNameVerify;     

} SYS_WINCS_OTA_TLS_CFG_t;

// *****************************************************************************
// OTA Service List
//
// Summary:
//    Identifies OTA Service list.
//
// Description:
//    This enumeration defines the various OTA services that can be requested.
//
// Remarks:
//    None.
// *****************************************************************************
typedef enum
{
    /* Set OTA options */
    SYS_WINCS_OTA_OPTIONS_SET,

    /* Start OTA download */
    SYS_WINCS_OTA_DOWNLOAD_START,               

    /* Activate OTA image */
    SYS_WINCS_OTA_IMG_ACTIVATE,          

    /* Verify OTA image */
    SYS_WINCS_OTA_IMG_VERIFY,           

    /* Invalidate OTA image */
    SYS_WINCS_OTA_IMG_INVALIDATE,    

    /* Register OTA application callback */
    SYS_WINCS_OTA_SET_CALLBACK,          

} SYS_WINCS_OTA_SERVICE_t;


// *****************************************************************************
// OTA Program Event List
//
// Summary:
//    Identifies OTA Program Event list.
//
// Description:
//    This enumeration defines the various events that can occur during the OTA
//    process.
//
// Remarks:
//    None.
// *****************************************************************************
typedef enum
{
    /* OTA download started */
    SYS_WINCS_OTA_DOWNLOAD_STARTED,

    /* OTA download complete */        
    SYS_WINCS_OTA_DOWNLOAD_COMPLETE,
            
    /* OTA Image Verify */
    SYS_WINCS_OTA_IMAGE_VERIFY,
            
    /* New partition active */
    SYS_WINCS_OTA_NEW_PARTITION_ACTIVE,

    /* Invalid URL */
    SYS_WINCS_OTA_INVALID_URL,

    /* Insufficient flash memory */
    SYS_WINCS_OTA_INSUFFICIENT_FLASH,

    /* OTA process busy */
    SYS_WINCS_OTA_BUSY,

    /* OTA error occurred */
    SYS_WINCS_OTA_ERROR

} SYS_WINCS_OTA_EVENT_t;


// *****************************************************************************
// OTA Service Control Function
//
// Summary:
//    Controls the OTA service based on the requested operation.
//
// Description:
//    This function handles various OTA service requests such as setting options,
//    starting downloads, activating images, setting callbacks, verifying images,
//    and invalidating images.
//
// Parameters:
//    request - The OTA service request type
//    otaHandle - The handle to the OTA service request data
//
// Returns:
//    SYS_WINCS_RESULT_t - The result of the OTA service request
//
// Remarks:
//    This function is typically called by the application to control the OTA service.
// *****************************************************************************
SYS_WINCS_RESULT_t SYS_WINCS_OTA_SrvCtrl(SYS_WINCS_OTA_SERVICE_t request, SYS_WINCS_OTA_HANDLE_t otaHandle);


// *****************************************************************************
// OTA Service Callback Function Pointer
//
// Summary:
//    Function pointer to the OTA service callback function.
//
// Description:
//    This typedef defines the function signature for the OTA service callback
//    function, which takes an OTA event and an OTA handle as parameters.
//
// Remarks:
//    None.
// *****************************************************************************
typedef void (*SYS_WINCS_OTA_CALLBACK_t)(SYS_WINCS_OTA_EVENT_t, SYS_WINCS_OTA_HANDLE_t);


#endif    /* SYS_WINCS_OTA_SERVICE_H */
