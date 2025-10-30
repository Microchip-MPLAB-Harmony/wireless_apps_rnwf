/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ota.h

  Summary:
    This file contains the header code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files
 ************************************************************************** */
#ifndef _APP_OTA_H    /* Guard against multiple inclusion */
#define _APP_OTA_H
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* This section lists the other files that are included in this file.
 */
#include "configuration.h"
#include "config/sam_e54_xpro_wincs02/system/command/sys_command.h"
#include "config/sam_e54_xpro_wincs02/system/ota/sys_wincs_ota_service.h"


// *****************************************************************************
// Enumeration: APP_OTA_STATES
//
// Summary:
//    Defines the states of the OTA application state machine.
//
// Description:
//    This enumeration defines the various states that the OTA application can
//    be in during its operation.
//
// Remarks:
//    None
// *****************************************************************************
typedef enum
{
    /* Application's state machine's initial state. */
    APP_OTA_STATE_INIT=0,
    
    APP_OTA_STATE_CONFIG,
            
    /* Start the OTA download process */
    APP_OTA_STATE_DOWNLOAD_START,
            
    /* OTA download completed */
    APP_OTA_STATE_DOWNLOAD_DONE,
            
    /* OTA partition switch completed */
    APP_OTA_STATE_SWITCH_DONE,
            
    /* Wait state */
    APP_OTA_STATE_WAIT,
            
    /* Stop the OTA process */
    APP_OTA_STATE_STOP,
            
    /* Error state */
    APP_OTA_STATE_ERROR,
    
    /*OTA Image activate*/
    APP_OTA_STATE_IMG_ACTIVATE,
            
} APP_OTA_STATES;

// *****************************************************************************
// *****************************************************************************
// Structure: APP_OTA_DATA
//
// Summary:
//    Holds the application data for the OTA process.
//
// Description:
//    This structure holds the current state of the OTA application and the
//    configuration for the OTA process.
//
// Remarks:
//    None
// *****************************************************************************
typedef struct
{
    /* The application's current state */
    APP_OTA_STATES state;

    /* OTA configuration parameters */
    SYS_WINCS_OTA_CFG_t otaCfg;
    
} APP_OTA_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_CMDProcessing
//
// Summary:
//    Processes OTA commands received from the console.
//
// Description:
//    This function processes the OTA commands received from the console and
//    updates the OTA state accordingly.
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
void APP_OTA_CMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_Initialize
//
// Summary:
//    Initializes the OTA application.
//
// Description:
//    This function initializes the OTA application and registers the OTA command
//    group.
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
void APP_OTA_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Function: APP_OTA_Tasks
//
// Summary:
//    Manages the OTA application tasks.
//
// Description:
//    This function manages the OTA application tasks based on the current state.
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
void APP_OTA_Tasks(void);

#endif // _APP_OTA_H