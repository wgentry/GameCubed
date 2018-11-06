/*******************************************************************************
* File Name: main.h
*
* Version: 1.20
*
* Description:
*  This file provides function prototypes, constants and macros for the example
*  project.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#if !defined(CY_MAIN_H)
#define CY_MAIN_H

#include <math.h>
#include <project.h>

/***************************************
*         Struct Definitions
****************************************/    

typedef struct Point{
    uint8 x, y, z;
} Point;

typedef struct Player{
    uint8 num;
    uint32 color;
} Player;
    
/***************************************
*         Function Prototypes
****************************************/

/* Function prototypes */
uint32 CheckSwitchState(void);

/* Functions for collecting touch data */
void UpdateTouchArray(void);
uint8 ReadStillTouch(uint8 address);
uint8 ReadAnyTouch(void);

/* Functions for Controlling Cubes */
void LightCube(uint8 x, uint8 y, uint8 z, uint32 color);
void ColorAllCubes(uint32 color);
void UpdateCubeLights(void);
Point AddressToCoordinate(uint8 address);

/* Functions for TIC TAC TOE Game */
Player PlayerSwitch(Player player);
uint8 CheckTicTacToeWin(Player player, Point p);
void ResetArrays(void);

/* Functions for Idle Animations */
void IdleAnimation1(uint32 idleCount);

/***************************************
*            Constants
****************************************/
/* idle animation length */
#define ANIMATION_LENGTH 1000u
#define ANIMATION_UPDATE_DELAY 6u
#define CHOOSE_FUNCTION_TIMEOUT_LENGTH 1000u

/* number of leds per cube */
#define LEDS_PER_CUBE 24u

/* number of consecutive '0's that must be read to read 0 */
#define OFF_DEBOUNCE 3u

/* I2C slave address to communicate with */
#define I2C_SLAVE_ADDR  (0x08u)

/* Buffer and packet size */
#define BUFFER_SIZE     (3u)
#define PACKET_SIZE     (BUFFER_SIZE)

/* Packet positions */
#define PACKET_SOP_POS  (0u)
#define PACKET_CMD_POS  (1u)
#define PACKET_STS_POS  (PACKET_CMD_POS)
#define PACKET_EOP_POS  (2u)

/* Start and end of packet markers */
#define PACKET_SOP      (0x01u)
#define PACKET_EOP      (0x17u)

/* Command valid status */
#define STS_CMD_DONE    (0x00u)
#define STS_CMD_FAIL    (0xFFu)

/* Command valid status */
#define TRANSFER_CMPLT    (0x00u)
#define TRANSFER_ERROR    (0xFFu)

/* Commands set */
#define CMD_SET_OFF     (0u)
#define CMD_SET_RED     (1u)
#define CMD_SET_GREEN   (2u)
#define CMD_SET_BLUE    (3u)

/* Delay between commands in milliseconds */
#define CMD_TO_CMD_DELAY  (500u)


/***************************************
*               Macros
****************************************/

/* Set LED RED color */
#define RGB_LED_ON_RED  \
                do{     \
                    LED_RED_Write  (0u); \
                    LED_GREEN_Write(1u); \
                }while(0)

/* Set LED GREEN color */
#define RGB_LED_ON_GREEN \
                do{      \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(0u); \
                }while(0)

/* Set LED TURN OFF */
#define RGB_LED_OFF \
                do{ \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(1u); \
                }while(0)

/* Makes array of struct type Point that contains the coordinates in order */
#define MAKE_COORDINATE_ARRAY 0

#endif /* (CY_MAIN_H) */


/* [] END OF FILE */
