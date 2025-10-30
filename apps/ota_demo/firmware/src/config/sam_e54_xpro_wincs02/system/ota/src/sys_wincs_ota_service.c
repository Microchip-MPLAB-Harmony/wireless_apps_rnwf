/*******************************************************************************
  WINCS Internal OTA Service Implementation

  File Name:
    sys_wincs_ota_service.c

  Summary:
    Source code for the WINCS Internal OTA Service implementation.

  Description:
    This file contains the source code for the WINCS Internal OTA Service
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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

/* This section lists the other files that are included in this file. */
#include "system/ota/sys_wincs_ota_service.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/* Variable to hold Callback handler address */
static SYS_WINCS_OTA_CALLBACK_t g_otaCallBackHandler;

// *****************************************************************************
// OTA Status Callback
//
// Summary:
//    Callback function for OTA status updates.
//
// Description:
//    This function is called to notify the application of changes in the OTA
//    update process.
//
// Parameters:
//    handle - The handle to the WINC driver instance
//    operation - The type of OTA operation being performed
//    opId - The operation ID
//    status - The current status of the OTA update, represented by ::WDRV_WINC_OTA_UPDATE_STATUS
//
// Returns:
//    None.
//
// Remarks:
//    This function is typically registered with the WINC driver to handle OTA status updates.
// *****************************************************************************

static void SYS_WINCS_OTA_statusCallback
(
    DRV_HANDLE handle, 
    WDRV_WINC_OTA_OPERATION_TYPE operation, 
    uint8_t opId, 
    WDRV_WINC_OTA_UPDATE_STATUS status
)
{
    switch(status)
    {
        case WDRV_WINC_OTA_STATUS_STARTED:
        {
            /* Notify that OTA download has started */
            g_otaCallBackHandler(SYS_WINCS_OTA_DOWNLOAD_STARTED, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_COMPLETE:
        {
            if (WDRV_WINC_OTA_OPERATION_DOWNLOAD_VERIFY == operation)
            {
                /* Notify that OTA download is complete */
                g_otaCallBackHandler(SYS_WINCS_OTA_DOWNLOAD_COMPLETE, (SYS_WINCS_OTA_HANDLE_t)&opId);
            }
            else if (WDRV_WINC_OTA_OPERATION_VERIFY == operation)
            { 
                /* Notify that new partition is active */
                g_otaCallBackHandler(SYS_WINCS_OTA_IMAGE_VERIFY, (SYS_WINCS_OTA_HANDLE_t)&opId);
            }
            else if (WDRV_WINC_OTA_OPERATION_ACTIVATE == operation)
            { 
                /* Notify that new partition is active */
                g_otaCallBackHandler(SYS_WINCS_OTA_NEW_PARTITION_ACTIVE, (SYS_WINCS_OTA_HANDLE_t)&opId);
            }  
            
            break;
        }

        case WDRV_WINC_OTA_STATUS_INVALID_URL:
        {
            /* Notify that the URL is invalid */
            g_otaCallBackHandler(SYS_WINCS_OTA_INVALID_URL, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_INSUFFICIENT_FLASH:
        {
            /* Notify that there is insufficient flash memory */
            g_otaCallBackHandler(SYS_WINCS_OTA_INSUFFICIENT_FLASH, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_BUSY:
        {
            /* Notify that the OTA process is busy */
            g_otaCallBackHandler(SYS_WINCS_OTA_BUSY, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_VERIFY_FAILED:
        {
            /* Notify that the OTA verification failed */
            g_otaCallBackHandler(SYS_WINCS_OTA_ERROR, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_CONN_ERROR:
        {
            /* Notify that there was a connection error */
            g_otaCallBackHandler(SYS_WINCS_OTA_ERROR, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_SERVER_ERROR:
        {
            /* Notify that there was a server error */
            g_otaCallBackHandler(SYS_WINCS_OTA_ERROR, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }

        case WDRV_WINC_OTA_STATUS_FAIL:
        {
            /* Notify that the OTA process failed */
            g_otaCallBackHandler(SYS_WINCS_OTA_ERROR, (SYS_WINCS_OTA_HANDLE_t)&opId);
            break;
        }
    }
}

// *****************************************************************************
// OTA Service Control
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

SYS_WINCS_RESULT_t SYS_WINCS_OTA_SrvCtrl
( 
    SYS_WINCS_OTA_SERVICE_t request,
    SYS_WINCS_OTA_HANDLE_t otaHandle
)
{
    DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;

    /* Get the driver handle */
    SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE, &wdrvHandle);
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_OK;

    switch(request)
    {
        /* Set OTA options */
        case SYS_WINCS_OTA_OPTIONS_SET:
        {
            SYS_WINCS_OTA_OPTIONS *otaOptions = (SYS_WINCS_OTA_OPTIONS *)otaHandle;

            status = WDRV_WINC_OTAOptionsSet(wdrvHandle, (WDRV_WINC_OTA_OPTIONS *)otaOptions);
            return SYS_WINCS_WIFI_GetWincsStatus(status, __FUNCTION__, __LINE__);
        }

        /* Start OTA download */
        case SYS_WINCS_OTA_DOWNLOAD_START:
        {
            SYS_WINCS_OTA_CFG_t *otaCfg = (SYS_WINCS_OTA_CFG_t *)otaHandle;
            status = WDRV_WINC_OTAUpdateFromURL(wdrvHandle, otaCfg->url, otaCfg->tlsCtxHandle, SYS_WINCS_OTA_statusCallback);
            return SYS_WINCS_WIFI_GetWincsStatus(status, __FUNCTION__, __LINE__);
        }

        /* Activate the new image */
        case SYS_WINCS_OTA_IMG_ACTIVATE:
        {
            status = WDRV_WINC_OTAImageActivate(wdrvHandle, SYS_WINCS_OTA_statusCallback);
            return SYS_WINCS_WIFI_GetWincsStatus(status, __FUNCTION__, __LINE__);
        }

        /* Verify the new image */
        case SYS_WINCS_OTA_IMG_VERIFY:
        {
            status = WDRV_WINC_OTAImageVerify(wdrvHandle, SYS_WINCS_OTA_statusCallback);
            return SYS_WINCS_WIFI_GetWincsStatus(status, __FUNCTION__, __LINE__);
        }

        /* Invalidate the current image */
        case SYS_WINCS_OTA_IMG_INVALIDATE:
        {
            status = WDRV_WINC_OTAImageInvalidate(wdrvHandle, SYS_WINCS_OTA_statusCallback);
            return SYS_WINCS_WIFI_GetWincsStatus(status, __FUNCTION__, __LINE__);
        }

        /* Set the callback handler */
        case SYS_WINCS_OTA_SET_CALLBACK:
        {   
            g_otaCallBackHandler = (SYS_WINCS_OTA_CALLBACK_t)(otaHandle);
            return SYS_WINCS_WIFI_GetWincsStatus(WDRV_WINC_STATUS_OK, __FUNCTION__, __LINE__); 
        }

        default:
        {
            /* Handle unknown request */
            break;
        }
    }

    return SYS_WINCS_FAIL;
}

/* *****************************************************************************
 End of File
 */
