/*******************************************************************************
  WINCS Host Assisted System service Header file

  File Name:
    sys_wincs_system_service.h

  Summary:
    Header file for the WINCS Host Assisted System service implementation.

  Description:
    This header file provides a simple APIs to enable System service with WINCS device 
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
#ifndef SYS_WINCS_SYSTEM_SERVICE_H
#define	SYS_WINCS_SYSTEM_SERVICE_H

#include <xc.h>

#include "system/wifi/sys_wincs_wifi_service.h" // include processor files - each processor file is guarded.  


/**
 @brief System Service List
 
 */
typedef enum
{
    /**<Request/Trigger reset the system */
    SYS_WINCS_SYSTEM_RESET,             
            
    /**<Request Software Revision */
    SYS_WINCS_SYSTEM_SW_REV,            
        
    /**<Request Software Revision */            
    SYS_WINCS_SYSTEM_DEV_INFO,  
            
    /**<Get the available certificate list */
    SYS_WINCS_SYSTEM_GET_CERT_LIST,     
            
    /**<Get the available private key list */
    SYS_WINCS_SYSTEM_GET_KEY_LIST,      
            
    /**<Request Driver version */          
    SYS_WINCS_SYSTEM_DRIVER_VER,
            
    /**<Set Driver system event callback */          
    SYS_WINCS_SYSTEM_SET_SYS_EVENT_CALLBACK, 
          
    /* Debug UART Set*/
    SYS_WINCS_SYSTEM_DEBUG_UART_SET,
    
    /* Debug UART Register callback*/
    SYS_WINCS_SYSTEM_SET_DEBUG_REG_CLBK,
            
}SYS_WINCS_SYSTEM_SERVICE_t;


/**
 * @brief System Service Layer API to handle system operations.
 * 
 *
 * @param[in] request   Service request ::SYS_WINCS_SYSTEM_SERVICE_t
 * @param[in] input     Parameters for the requested service
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_SYSTEM_SrvCtrl(SYS_WINCS_SYSTEM_SERVICE_t request, void *input);


/* Get System Service task status*/
bool SYS_WINCS_SYSTEM_getTaskStatus(void);
#endif	/* SYS_WINCS_SYSTEM_SERVICE_H */

/** @}*/
