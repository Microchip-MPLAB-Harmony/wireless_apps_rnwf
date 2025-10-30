/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_azure.h

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

/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifndef _APP_AZURE_H    /* Guard against multiple inclusion */
#define _APP_AZURE_H
/* This section lists the other files that are included in this file.
 */
#include "configuration.h"
#include "system/console/sys_console.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "system/mqtt/sys_wincs_mqtt_service.h"
#include "config/sam_e54_xpro_wincs02/configuration.h"
#include "config/sam_e54_xpro_wincs02/system/command/sys_command.h"


// *****************************************************************************
// *****************************************************************************
// Azure Application States
//
// Summary:
//    Defines the states for the Azure application state machine.
//
// Description:
//    This enumeration defines the various states that the Azure application
//    can be in during its lifecycle.
//
// Remarks:
//    None
// *****************************************************************************
typedef enum
{
    /* Application's state machine's initial state. */
    APP_AZURE_STATE_INIT = 0,

    /* State to start the Azure application. */
    APP_AZURE_STATE_START,

    /* State to connect to the Azure cloud. */
    APP_AZURE_STATE_CONNECT_CLOUD,

    /* State indicating the Azure cloud connection is up. */
    APP_AZURE_STATE_CLOUD_UP,

    /* State indicating an error has occurred. */
    APP_AZURE_STATE_ERROR,

    /* State indicating the application has completed its tasks. */
    APP_AZURE_STATE_DONE,

    /* State to wait for a certain condition or event. */
    APP_AZURE_STATE_WAIT,

    /* State to stop the Azure application. */
    APP_AZURE_STATE_STOP
            
} APP_AZURE_STATES;

// *****************************************************************************
// *****************************************************************************
// Azure Application Data
//
// Summary:
//    Defines the data structure for the Azure application.
//
// Description:
//    This structure defines the data used by the Azure application, including
//    the current state and any additional data required.
//
// Remarks:
//    None
// *****************************************************************************
typedef struct
{
    /* The application's current state */
    APP_AZURE_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_AZURE_CONTEXT;

// *****************************************************************************
// *****************************************************************************
// Function: APP_AZURE_CMDProcessing
//
// Summary:
//    Processes Azure commands.
//
// Description:
//    This function processes commands related to the Azure application. It
//    takes the command input, parses it, and performs the necessary actions.
//
// Parameters:
//    pCmdIO - Pointer to the command device node.
//    argc - Number of arguments.
//    argv - Array of arguments.
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_AZURE_CMDProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

// *****************************************************************************
// *****************************************************************************
// Function: APP_AZURE_Initialize
//
// Summary:
//    Initializes the Azure application.
//
// Description:
//    This function initializes the Azure application, registers the Azure command
//    group, and sets the initial state of the Azure application.
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
void APP_AZURE_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Function: APP_AZURE_Tasks
//
// Summary:
//    Manages the Azure application tasks.
//
// Description:
//    This function defines the state machine for the Azure application and
//    manages the tasks based on the current state.
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
void APP_AZURE_Tasks(void);
#endif // _APP_OTA_H