/*******************************************************************************
  WINCS Host Assisted System Service Implementation

  File Name:
    sys_wincs_system_service.c

  Summary:
    Source code for the WINCS Host Assisted System Service implementation.

  Description:
    This file contains the source code for the WINCS Host Assisted System Service
    implementation.
 *******************************************************************************/

/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

 
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
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <string.h>
#include <stdbool.h>


/* This section lists the other files that are included in this file.
 */
#include "system/sys_wincs_system_service.h"
#include "wdrv_winc_common.h"
#include "wdrv_winc.h"


static bool wincsSysTaskCompleted = false;
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

bool SYS_WINCS_SYSTEM_getTaskStatus()
{
    return wincsSysTaskCompleted;
}

bool SYS_WINCS_SYSTEM_setTaskStatus(bool taskStatus)
{
    wincsSysTaskCompleted = taskStatus;
    return true;
            
}

static void SYS_WINCS_SYSTEM_FindFileCallback
(
    DRV_HANDLE handle, 
    uintptr_t userCtx, 
    WDRV_WINC_FILE_TYPE type, 
    const char *pFilename, 
    WDRV_WINC_FILE_STATUS_TYPE status
)
{
    if (NULL == pFilename)
    {
        if (WDRV_WINC_FILE_STATUS_OK == status)
        {
            SYS_CONSOLE_PRINT("---------------END------------------\r\n\r\n");
            wincsSysTaskCompleted = true;
        }
        return;
    }

    if (WDRV_WINC_FILE_TYPE_CERTS == type)
    {
        SYS_CONSOLE_PRINT("Cert: %s\r\n", pFilename);
    }
    else if (WDRV_WINC_FILE_TYPE_PRIKEYS == type)
    {
        SYS_CONSOLE_PRINT("Key: %s\r\n", pFilename);
    }
    else
    {
        SYS_CONSOLE_PRINT("U: %s\r\n", pFilename);
    }
}


static void SYS_WINCS_SYSTEM_EventCallback
(
    SYS_MODULE_OBJ object, 
    WDRV_WINC_SYSTEM_EVENT_TYPE event
)
{
    SYS_CONSOLE_PRINT("System Event: ");

    switch (event)
    {
        case WDRV_WINC_SYSTEM_EVENT_DEVICE_HARD_RESET:
        {
            SYS_CONSOLE_PRINT("HARD_RESET\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_INIT_BEGIN:
        {
            SYS_CONSOLE_PRINT("INIT_BEGIN\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_INIT_COMPLETE:
        {
            SYS_CONSOLE_PRINT("INIT_COMPLETE\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_READY:
        {
            SYS_CONSOLE_PRINT("DEVICE_READY\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_RESET_FAILED:
        {
            SYS_CONSOLE_PRINT("RESET_FAILED\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_RESET_TIMEOUT:
        {
            SYS_CONSOLE_PRINT("RESET_TIMEOUT\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_RESET_RETRY:
        {
            SYS_CONSOLE_PRINT("RESET_RETRY\r\n");
            break;
        }

        case WDRV_WINC_SYSTEM_EVENT_DEVICE_COMMS_ERROR:
        {
            SYS_CONSOLE_PRINT("COMMS_ERROR\r\n");
            break;
        }

        default:
        {
            SYS_CONSOLE_PRINT("unknown %d\r\n", event);
            break;
        }
    }
}



/* WINCS System Service Conttrol Function */
SYS_WINCS_RESULT_t SYS_WINCS_SYSTEM_SrvCtrl
(
    SYS_WINCS_SYSTEM_SERVICE_t request,
    void *input
)
{
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_OK;
    DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;
    
    SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE, &wdrvHandle);
    
    switch(request)
    {
        /* WINCS Reset */
        case SYS_WINCS_SYSTEM_RESET:
        {
            WDRV_WINC_RESETN_Set();
            break;
        }
        
        /* WINCS Software Revision */
        case SYS_WINCS_SYSTEM_SW_REV:
        {
            *(uint8_t*)input = '\0';
            status = WDRV_WINC_InfoDeviceFirmwareVersionGet(wdrvHandle, true,(WDRV_WINC_FIRMWARE_VERSION_INFO *) input);
            break;
        }
        
        /* WINCS Device Info  */
        case SYS_WINCS_SYSTEM_DEV_INFO:
        {
            *(uint8_t*)input = '\0';
            status = WDRV_WINC_InfoDeviceGet(wdrvHandle, (WDRV_WINC_DEVICE_INFO *)input);
            
            break;
        }
        
        /* WINCS Driver version  */
        case SYS_WINCS_SYSTEM_DRIVER_VER:
        {
            *(uint8_t*)input = '\0';
            status = WDRV_WINC_InfoDriverVersionGet(wdrvHandle, (WDRV_WINC_DRIVER_VERSION_INFO *)input);
            
            break;
        }
        
        /* WINCS Get Certificates List */
        case SYS_WINCS_SYSTEM_GET_CERT_LIST:
        {
            wincsSysTaskCompleted = false;
            status = WDRV_WINC_FileFind(wdrvHandle, WDRV_WINC_FILE_TYPE_CERTS, SYS_WINCS_SYSTEM_FindFileCallback, 0);
            break;
        }
        
        /* WINCS Get Key List*/
        case SYS_WINCS_SYSTEM_GET_KEY_LIST:
        {
            wincsSysTaskCompleted = false;
            status = WDRV_WINC_FileFind(wdrvHandle, WDRV_WINC_FILE_TYPE_PRIKEYS, SYS_WINCS_SYSTEM_FindFileCallback, 0);
            break;
        }
        
        case SYS_WINCS_SYSTEM_SET_SYS_EVENT_CALLBACK:
        {
            WDRV_WINC_RegisterSystemEventCallback(sysObj.drvWifiWinc, SYS_WINCS_SYSTEM_EventCallback);
            break;
        }
        
        case SYS_WINCS_SYSTEM_SET_DEBUG_REG_CLBK:
        {
            WDRV_WINC_DebugRegisterCallback((WDRV_WINC_DEBUG_PRINT_CALLBACK)input);
            WINC_DevSetDebugPrintf((WINC_DEBUG_PRINTF_FP)input);
            break;
        }
        
        case SYS_WINCS_SYSTEM_DEBUG_UART_SET:
        {
            status = WDRV_WINC_DebugUARTSet(wdrvHandle, WDRV_WINC_DEBUG_UART_2, 0);
            break;
        }
        
        default:
        {
            break;
        }
    }
    
    return SYS_WINCS_WIFI_GetWincsStatus(status);
}

/* *****************************************************************************
 End of File
 */
