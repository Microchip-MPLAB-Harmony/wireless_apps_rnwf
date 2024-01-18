/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

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
    files.
 ******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <stddef.h>                     
#include <stdbool.h>                   
#include <stdlib.h>                
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "app.h"
#include "user.h"
#include "definitions.h" 
#include "configuration.h"
#include "system/debug/sys_debug.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/sys_rnwf_system_service.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/ota/sys_rnwf_ota_service.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "peripheral/sercom/spi_master/plib_sercom6_spi_master.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Buffer

  Summary:
    Application Buffer array 

  Description:
    This array holds the application's buffer.

  Remarks:
    None
*/

static uint8_t g_appBuf[SYS_RNWF_OTA_BUF_LEN_MAX];

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

static APP_DATA g_appData;

// *****************************************************************************
/* Application Image size

  Summary:
    Holds size of the Image downloaded by Ota 

  Description:
    This variable size of the Image downloaded by Ota 

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

static uint32_t g_appImgSize;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* Wifi Callback Handler Function */
static void SYS_RNWF_WIFI_CallbackHandler
(
    SYS_RNWF_WIFI_EVENT_t event, 
    uint8_t *p_str
)
{
    switch(event)
    {
        /* Wifi Connected */
        case SYS_RNWF_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected\r\n");
            break;
        }
        
        /* Wifi Disconnected */
        case SYS_RNWF_DISCONNECTED:
        {    
            SYS_CONSOLE_PRINT("WiFi Disconnected\nReconnecting... \r\n");
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_STA_CONNECT, NULL);
            break;
        }
            
        /* DHCP IP allocated */
        case SYS_RNWF_DHCP_DONE:
        {
            SYS_CONSOLE_PRINT("DHCP IP:%s\r\n", &p_str[2]); 
            
            /* Enable OTA by passing the OTA buffer space */
            if(SYS_RNWF_OTA_SrvCtrl(SYS_RNWF_OTA_ENABLE, (void *)g_appBuf) == SYS_RNWF_PASS)
            {
                SYS_RNWF_OTA_DBG_MSG("Successfully Enabled the OTA\r\n");
            }
            else
            {
                SYS_RNWF_OTA_DBG_MSG("ERROR!!! Failed to enable the OTA\r\n");
            }
            break;
        }
        
        /* Wifi Scan Indication */
        case SYS_RNWF_SCAN_INDICATION:
        {
            break;
        }
            
        /* Wifi Scan Done */
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

/* OTA Callback Handler Function */
static void SYS_RNWF_OTA_CallbackHandler
(
    SYS_RNWF_OTA_EVENT_t event,
    void *p_str
)
{
    static uint32_t flash_addr = SYS_RNWF_OTA_FLASH_IMAGE_START;
    
    switch(event)
    {
        /* Change to UART mode */
        case SYS_RNWF_OTA_EVENT_MAKE_UART:
        {
            break;
        }
            
        /* FW Download start */
        case SYS_RNWF_OTA_EVENT_DWLD_START:
        {
            SYS_CONSOLE_PRINT("Total Size = %lu\r\n", *(uint32_t *)p_str); 
            SYS_CONSOLE_PRINT("Erasing the SPI Flash\r\n");
            
            SYS_RNWF_OTA_FlashErase();
            SYS_CONSOLE_PRINT("Erasing Complete!\r\n"); 
            break;
        }
        
        /* FW Download done */
        case SYS_RNWF_OTA_EVENT_DWLD_DONE:
        {       
            g_appImgSize = *(uint32_t *)p_str;  
            SYS_CONSOLE_PRINT("Download Success!= %lu bytes\r\n", g_appImgSize);  
            
            break; 
        }
              
        /* Write to SST26 */
        case SYS_RNWF_OTA_EVENT_FILE_CHUNK:
        {
            volatile SYS_RNWF_OTA_CHUNK_t *ota_chunk = (SYS_RNWF_OTA_CHUNK_t *)p_str;               
            SYS_RNWF_OTA_FlashWrite(flash_addr,ota_chunk->chunk_size ,ota_chunk->chunk_ptr);
            flash_addr += ota_chunk->chunk_size;
            break; 
        }    
                           
        default:
        {
            break;
        }
    }
    
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize 
( 
    void 
)
{
    /* Place the App state machine in its initial state. */
    g_appData.state = APP_STATE_INITIALIZE;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

void APP_RNWF_SwResetHandler
(
    void
)
{
    /* RNWF Reset */
    SYS_RNWF_OTA_DfuReset();
    
    /* Manual Delay to synchronise host reset. 
     * User can change according to their Host reset timing*/
    for(int i=0; i< 0xFFFFF; i++)
    {
        SYS_CONSOLE_PRINT("");
    }
    
    /* Host Reset */
    SYS_RESET_SoftwareReset();
}


/* Maintain the application's state machine. */
void APP_Tasks 
( 
    void 
)
{
    switch(g_appData.state)
    {
        /* Initialize Flash and RNWF device */
        case APP_STATE_INITIALIZE:
        {
            SYS_CONSOLE_PRINT("%s", "##############################################\r\n");
            SYS_CONSOLE_PRINT("%s", "  Welcome RNWF02 WiFi Host Assisted OTA Demo  \r\n");
            SYS_CONSOLE_PRINT("%s", "##############################################\r\n\r\n"); 
            
            if(false == SYS_RNWF_OTA_FlashInitialize())
            {
                SYS_CONSOLE_PRINT("ERROR : No valid SPI Flash found!\r\n\tConnect SPI MikroBus(SST26) to EXT2 and reset!\r\n");
                g_appData.state = APP_STATE_ERROR;
                break;
            }
            
            /* Initialize RNWF Module */
            SYS_RNWF_IF_Init();
            
            g_appData.state = APP_STATE_GET_DEV_INFO;
            break;
        }
        
        /* Get RNWF device Information */
        case APP_STATE_GET_DEV_INFO:
        {
            
            if (SYS_RNWF_SYSTEM_SrvCtrl( SYS_RNWF_SYSTEM_SW_REV, g_appBuf) != SYS_RNWF_PASS)
            {
                /* Check if Flash has the New FW pre loaded in it */
                SYS_RNWF_OTA_HDR_t otaHdr;
                SYS_RNWF_OTA_FlashRead(SYS_RNWF_OTA_FLASH_IMAGE_START, sizeof(SYS_RNWF_OTA_HDR_t), (uint8_t *)&otaHdr.seq_num);
                
                SYS_CONSOLE_PRINT("Image details in the Flash\r\n");
                SYS_CONSOLE_PRINT("Sequence Number 0x%X\r\n", (unsigned int)otaHdr.seq_num);
                SYS_CONSOLE_PRINT("Start Address 0x%X\r\n", (unsigned int)otaHdr.start_addr);
                SYS_CONSOLE_PRINT("Image Length 0x%X\r\n", (unsigned int)otaHdr.img_len);
                
                if(otaHdr.seq_num != 0xFFFFFFFF && otaHdr.start_addr != 0xFFFFFFFF && otaHdr.img_len != 0xFFFFFFFF)               
                {        
                    g_appImgSize = otaHdr.img_len;
                    /* Program RNWF with pre loaded FW in Flash*/
                    g_appData.state = APP_STATE_PROGRAM_DFU;
                    break;
                }
                SYS_CONSOLE_PRINT("Error: Module is Bricked!");
                
                g_appData.state = APP_STATE_ERROR;
                break;
            }
            else
            {
                if(g_appBuf[0] == '\0')
                {
                    SYS_CONSOLE_PRINT("ERROR : No RNWF02 module found\r\n\tConnect RNWF02 module to EXT1 and reset\r\n");
                    g_appData.state = APP_STATE_ERROR;
                    break;
                }
                
                SYS_CONSOLE_PRINT("Software Revision: %s\r\n",g_appBuf);
            }
            
            /* Get RNWF device Information */
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_DEV_INFO, g_appBuf);
            SYS_CONSOLE_PRINT("Device Info: %s\r\n", g_appBuf);
            
            /* Get RNWF device Wi-Fi Information*/
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RWWF_SYSTEM_GET_WIFI_INFO, g_appBuf);
            SYS_CONSOLE_PRINT("%s\r\n\n", g_appBuf);
            
            g_appData.state = APP_STATE_REGISTER_CALLBACK;
            break;
        }
        
        /* Register the Callbacks with Services */
        case APP_STATE_REGISTER_CALLBACK:
        {
            /* Configure SSID and Password for STA mode */
            SYS_RNWF_WIFI_PARAM_t wifi_sta_cfg = {SYS_RNWF_WIFI_MODE_STA, SYS_RNWF_WIFI_STA_SSID, SYS_RNWF_WIFI_STA_PWD, SYS_RNWF_STA_SECURITY, 1};
            SYS_CONSOLE_PRINT("Connecting to %s\r\n",SYS_RNWF_WIFI_STA_SSID);
            
            /* Register Callback with Wifi Service */
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, SYS_RNWF_WIFI_CallbackHandler);
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_SET_WIFI_PARAMS, &wifi_sta_cfg);
    
            /* Register Callback with OTA Service */
            SYS_RNWF_OTA_SrvCtrl(SYS_RNWF_OTA_SET_CALLBACK, (void *)SYS_RNWF_OTA_CallbackHandler);
            
            g_appData.state = APP_STATE_WAIT_FOR_DOWNLOAD;
            break;
        }
        
        /* Wait for Download to complete */
        case APP_STATE_WAIT_FOR_DOWNLOAD:
        {
            bool isDownloadDone = false;
            SYS_RNWF_OTA_SrvCtrl(SYS_RNWF_OTA_CHECK_DWLD_DONE,(void *)&isDownloadDone);
            
            if (isDownloadDone == true)
            {
                g_appData.state = APP_STATE_PROGRAM_DFU;
            }
            break;
        }
        
        /* Program the RNWF device with New FW */
        case APP_STATE_PROGRAM_DFU:
        {
            bool OtaDfuComplete = false;
            
            SYS_RNWF_OTA_ProgramDfu(); 
            SYS_RNWF_OTA_SrvCtrl ( SYS_RNWF_OTA_CHECK_DFU_DONE, (void *)&OtaDfuComplete);
            
            if(OtaDfuComplete == true )
            {
                g_appData.state = APP_STATE_RESET_DEVICE;
            }
            break;
        }
        
        /* Reset RNWF device and Host */
        case APP_STATE_RESET_DEVICE:
        {
            APP_RNWF_SwResetHandler();
            
            break;
        }
        
        /* Application Error State */
        case APP_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT("ERROR : APP Error\r\n");
            g_appData.state = APP_STATE_IDLE;
            break;
        }
        
        /* Application Idle state */
        case APP_STATE_IDLE:
        {
            break;
        }
        
        /* Default state */
        default:
        {
            break;
        }
    }
    
    /* Console Tasks */
    SYS_CONSOLE_Tasks(sysObj.sysConsole0);
    
    /* Interface Event Handler */
    SYS_RNWF_IF_EventHandler();
}

/*******************************************************************************
 End of File
 */
