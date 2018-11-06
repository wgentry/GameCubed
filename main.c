/*******************************************************************************
* File Name: main.c
*
* Version: 1.20
*
* Description:
*  This example project demonstrates the basic operation of the I2C master
*  (SCB mode) component. The I2C master sends a packet with a command to
*  the I2C slave to control the RGB LED color. The packet with a status
*  is read back.
*  For more information refer to the example project datasheet.
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
#include <math.h>
#include "main.h"

/* contains the stillTouch value from all cubes */
uint8 touchArray[3][3][3], 
      gameArray[3][3][3],
      debounceTouchArray[3][3][3]   = { { {0,0,0}, {0,0,0}, {0,0,0} },
							            { {0,0,0}, {0,0,0}, {0,0,0} },
							            { {0,0,0}, {0,0,0}, {0,0,0} } };

/* countains the color values for each cube */
uint32 colorArray[3][3][3];

/* addressArray[x][y][z] = address */	      /* y = 0 */ /* y = 1 */ /* y = 2 */											
uint8 addressArray[3][3][3] = { /* x = 0 */ { {12,11,10}, {20,19,18}, {29,28,27} },
								/* x = 1 */	{ {13, 8, 9}, {21,34,17}, {30,25,26} },
								/* x = 2 */	{ {14,15,16}, {22,23,24}, {31,32,33} } };

/* contains the coordinates of the winning line */
Point winArray[3];

int main()
{
 
    CyGlobalIntEnable;
    I2CM_Start();
    StripLights_Start();
	
	// Select Function Variable
	uint8 state = 0; // 0 Choose Function, 1 tic tac toe, 2 interactive touch
	
	// 0: Choose Function
    uint32 idleCount = 0; // idle animation variabes
    uint32 count = 0;
	uint32 chooseFunctionTimeout = 0; // add this macro to .h file!!!
	
    // 1: Tic tac toe variables
    Player player = {1, StripLights_RED};
    uint8 winner = 0; // 0 no one won, 1 and 2 someone one, 3 cats game
	
	// 2: interactive touch variables
	uint8 exitCount = 0;
    
    // LED initialization 
    ColorAllCubes(StripLights_BLUE);
    UpdateCubeLights();
    CyDelay(500);
    ColorAllCubes(StripLights_GREEN);
    UpdateCubeLights();
    CyDelay(500);
    ColorAllCubes(StripLights_RED);
    UpdateCubeLights();
    CyDelay(500);
    ColorAllCubes(StripLights_OFF);
    UpdateCubeLights();

    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    for(;;)
    {   
        /* Updates the values of the TouchArray before and game code is run */
		UpdateTouchArray();
		
		// The following state machine controls the mode that the cube is in
		// State variable: state =
			// 0: Choose Function
			// 1: Tic Tac Toe
			// 2: Interactive Touch
			
		switch (state) 
		{
							
			case 0: // Choose Function

				// if any touch is recognized, prompts function choice:
				if (ReadAnyTouch() == 1 || chooseFunctionTimeout > 0) 
				{	
					if (chooseFunctionTimeout == 0)
					{
						chooseFunctionTimeout = CHOOSE_FUNCTION_TIMEOUT_LENGTH;
						for (int i = 0; i < 3; i++)
						{
							for (int j = 0; j < 3; j++)
							{
								colorArray[i][0][j] = StripLights_RED;
								colorArray[i][1][j] = StripLights_BLUE;
								colorArray[i][2][j] = StripLights_GREEN;
							}
						}
						UpdateCubeLights();
						while(ReadAnyTouch() == 1) {UpdateTouchArray();} // waits for user to let go
					}
					else {chooseFunctionTimeout--;}
					
					for (int i = 0; i < 3; i++)
					{
						for (int j = 0; j < 3; j++)
						{
							for (int k = 0; k < 3; k++)
							{
								if (touchArray[i][j][k] == 1)
								{
                                    uint32 color = colorArray[i][j][k];
									chooseFunctionTimeout = 0;
									
									if (color == StripLights_RED)
										state = 0;
									else if (color == StripLights_BLUE)
										state = 1;
                                    else if (color == StripLights_GREEN)
										state = 2;
                                    
                                    ColorAllCubes(color);
									UpdateCubeLights();
									while(ReadAnyTouch() == 1) {UpdateTouchArray();} // waits for user to let go
                                    ResetArrays();
								}
							}
						}
					}
				}
				// idle animation
				else 
				{   
					idleCount++;
					// random flashing
					if ( (idleCount < ANIMATION_LENGTH) && ((idleCount % ANIMATION_UPDATE_DELAY) == 0) )
					{
						for (int i = 0; i < 3; i++)
						{
							for (int j = 0; j < 3; j++)
							{
								for (int k = 0; k < 3; k++)
								{
									colorArray[i][j][k] = StripLights_ColorInc(1);
								}
							}
						}
							UpdateCubeLights();      
					}
					// all cubes one color cycle through color wheel
					else if ( (idleCount < ANIMATION_LENGTH * 2) && ((idleCount % ANIMATION_UPDATE_DELAY) == 0) )
					{
						ColorAllCubes(StripLights_ColorInc(1));
						UpdateCubeLights();
					}
					// one cube bounces around
					else if ( (idleCount < ANIMATION_LENGTH * 4) && ((idleCount % ANIMATION_UPDATE_DELAY) == 0) )
					{
						IdleAnimation1(count++);
						UpdateCubeLights();

					}
					// Resets animation cycle
					else if (idleCount >= ANIMATION_LENGTH * 4)
					{
						idleCount = 0;
						count = 0;
					}
					CyDelay(1);
				}
				
			case 1: // Tic Tac Toe
			
				switch (winner)
				{
					case 0: // no cats game and no winner
			  
						for (int i = 0; i < 3; i++) 
						{
							for (int j = 0; j < 3; j++)
							{
								for (int k = 0; k < 3; k++) 
								{
									if (touchArray[i][j][k] == 1 && gameArray[i][j][k] == 0)
									{
										colorArray[i][j][k] = player.color;
										gameArray[i][j][k] = player.num;
										Point p = {i,j,k};
										
										winner = CheckTicTacToeWin(player, p);
										if (winner > 0)
											break;
										
										player = PlayerSwitch(player);
										UpdateCubeLights();
										
									}
								}
							}   
						}
						break;
					
					case 1: // player 1 wins
					case 2: // player 2 wins
					
						for (int i = 0; i < 10; i++) 
						{   
							ColorAllCubes(StripLights_OFF);
							if ( (i % 2) == 0)
							{
								colorArray[winArray[0].x][winArray[0].y][winArray[0].z] = player.color;
								colorArray[winArray[1].x][winArray[1].y][winArray[1].z] = player.color;
								colorArray[winArray[2].x][winArray[2].y][winArray[2].z] = player.color;
							}
							else
							{
								colorArray[winArray[0].x][winArray[0].y][winArray[0].z] = StripLights_GREEN;
								colorArray[winArray[1].x][winArray[1].y][winArray[1].z] = StripLights_GREEN;
								colorArray[winArray[2].x][winArray[2].y][winArray[2].z] = StripLights_GREEN;
							}
							UpdateCubeLights();
							CyDelay(500);
						}
						
						ResetArrays();
                        winner = 0;
                        state = 0;
						break;
						
					case 3: // cats game
					
						for (int i = 0; i < 10; i++) 
						{
							if ( (i % 2) == 0)
							{
								StripLights_DisplayClear(StripLights_OFF);
							}
							else
							{
								UpdateCubeLights();
							}
							CyDelay(500);
						}
						
						for (int i = 2; i >= 0; i--) 
						{
							for (int j = 0; j < 3; j++)
							{
								for (int k = 0; k < 3; k++) 
								{
									colorArray[j][k][i] = StripLights_OFF;
								}
							}  
							UpdateCubeLights();
							CyDelay(500);
						}   
						
						ResetArrays();
                        winner = 0;
                        state = 0;
						break;
						
				}
				break; 
			
			case 2: // Touch to Light Up
				
				if (touchArray[1][1][2] == 1)
				{
					exitCount++;
					if(exitCount >= 3)
					{
                        state = 0;
						ColorAllCubes(StripLights_RED);
                        UpdateCubeLights();
                        while(ReadAnyTouch() == 1) {UpdateTouchArray();} // waits for user to let go
                        ResetArrays();
					}
				}
				else
				{
					for (int i = 0; i < 3; i++) 
					{
						for (int j = 0; j < 3; j++)
						{
							for (int k = 0; k < 3; k++) 
							{
								if (touchArray[i][j][k] == 1 && gameArray[i][j][k] == 0)
								{
									colorArray[i][j][k] = StripLights_ColorInc(1);
									gameArray[i][j][k] = 1;
									UpdateCubeLights();	
								}
								else if (touchArray[i][j][k] == 0 && gameArray[i][j][k] == 1)
								{
									colorArray[i][j][k] = StripLights_OFF;
									gameArray[i][j][k] = 0;
									UpdateCubeLights();	
								}
							}
						}   
					}	
				}		
				break;
				
		}		  
    
    }
        
}


/*******************************************************************************
* Function Name: ReadStillTouch
********************************************************************************
* Summary:
*  Reads stillTouch value from slave
*
* Parameters:
*  Address
*
* Return:
*  Returns value of stillTouch
*  - 0 if not being touched
*  - 1 if being touched
*
*******************************************************************************/
uint8 ReadStillTouch(uint8 address)
{
    
    uint8  buffer[BUFFER_SIZE];
    uint8  stillTouch = 0;

    (void) I2CM_I2CMasterReadBuf(address, buffer, PACKET_SIZE, \
                                 I2CM_I2C_MODE_COMPLETE_XFER);

    /* Waits until master complete read transfer */
    uint8 counter = 0;
    while ((0u == (I2CM_I2CMasterStatus() & I2CM_I2C_MSTAT_RD_CMPLT)) && (counter < 200))
    {
        counter++;
    }
    
    if(counter < 200)
    {
        if ((I2CM_I2CMasterGetReadBufSize() == BUFFER_SIZE) &&
            (PACKET_SOP == buffer[PACKET_SOP_POS]) &&
            (PACKET_EOP == buffer[PACKET_EOP_POS]))
        {
            stillTouch = buffer[PACKET_STS_POS];
        }

        (void) I2CM_I2CMasterClearStatus();

        return stillTouch;
    }
    else 
    {
        (void) I2CM_I2CMasterClearStatus();
        return 100;
    }
}

/*******************************************************************************
* Function Name: LightCube
********************************************************************************
* Summary:
*  allows coordinate control of all individual cubes
*
* Parameters:
*  x,y,z coordinate
*  color to light LEDS
*  dimlevel
*
* Return:
*  nothing, just lights the cube using built in strip light functions
*
*******************************************************************************/
void LightCube(uint8 x, uint8 y, uint8 z, uint32 color) {
    uint8 address = addressArray[x][y][z];
    uint8 startLED = 0;
    
    if ( address  < 17 ) 
    {
        startLED = (address - 8) * LEDS_PER_CUBE;
    }
    else if ( address < 25 )
    {
        startLED = (address - 17) * LEDS_PER_CUBE;
    }
    else
    {
        startLED = (address - 25) * LEDS_PER_CUBE;
    }
    
    StripLights_DrawLine(startLED,y,startLED + LEDS_PER_CUBE - 1,y,color);
    return;
}

/*******************************************************************************
* Function Name: ColorAllCubes
********************************************************************************
* Summary:
*  sets all the color values in colorArray to specified color
*
* Parameters:
*   Color
*   Dim value
*
* Return:
*  nothing
*
*******************************************************************************/
void ColorAllCubes(uint32 color) {
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                colorArray[i][j][k]= color;
            }
        }   
    }
    
    return;
}    

/*******************************************************************************
* Function Name: UpdateCubeLights
********************************************************************************
* Summary:
*  takes value from colorArray and updates the cubes.
*
* Parameters:
*   none
*
* Return:
*  nothing, just updates the cube colors
*
*******************************************************************************/
void UpdateCubeLights(void) {
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                LightCube(i,j,k, colorArray[i][j][k]);
            }
        }   
    }
    
    StripLights_TriggerWait(1);
    //while (!StripLights_Ready()) {}
    
    return;
}

/*******************************************************************************
* Function Name: AddressToCoordinate
********************************************************************************
* Summary:
*  Converts I2C address to the spacial coordinate of a cube as struct 'Point'
*
* Parameters:
*  Address
*
* Return:
*  Coordinate in the form of a Point struct
*
*******************************************************************************/
Point AddressToCoordinate(uint8 address) {
    Point coordinateDefault = {0,0,0};
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                if (address == addressArray[i][j][k]) 
                {
                    Point coordinate = {i,j,k};
                    return coordinate;
                }
            }
        }   
    }
    
    return coordinateDefault;
}

/*******************************************************************************
* Function Name: UpdateTouchArray
********************************************************************************
* Summary:
*  Updates the touchArray that contains all the touch values from the cubes.
*
* Parameters:
*   none
* Return:
*   none
*
*******************************************************************************/
void UpdateTouchArray(void) {
 
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                uint8 result = ReadStillTouch(addressArray[i][j][k]);
                if (result == 1)
                {
                    touchArray[i][j][k] = 1;
                    debounceTouchArray[i][j][k] = OFF_DEBOUNCE;
                }
                else if ( debounceTouchArray[i][j][k] > 0 )
                {
                    touchArray[i][j][k] = 1;
                    debounceTouchArray[i][j][k] -= 1;
                }
                else
                {
                    touchArray[i][j][k] = 0;
                }
                    
            }
        }   
    }
    
    return;
}

/*******************************************************************************
* Function Name: ReadTouchArray
********************************************************************************
* Summary:
*  Reads all values in touch Array and outputs coordinate of first coorinate that
*  that contains a 1. If nothing is a 1, ouputs (1,1,1) (center block)
*
* Parameters:
*   nothing
* Return:
*   coordinate being touched.
*
*******************************************************************************/
uint8 ReadAnyTouch(void) {
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                if (touchArray[i][j][k] != 0) 
                {
                    return 1;
                }
            }
        }   
    }
    
    return 0;
}
/*******************************************************************************
* Function Name: PlayerSwitch
********************************************************************************
* Summary:
*  Switches the player from 1 to 2 and vice versa
*
*******************************************************************************/ 
Player PlayerSwitch(Player player) {
    
    if (player.num == 1) {
        player.num = 2;
        player.color = StripLights_BLUE;
    }
    else {
        
        player.num = 1;
        player.color = StripLights_RED;
    }    
    
    return player;
}    
/*******************************************************************************
* Function Name: CheckTicTacToeWin
********************************************************************************
* Summary:
*  checks the golbal array gameArray to see if there is a 3 in a row for inputed player.
*   returns a 0 if no winner, or the player that one if there is a winner. 
*   This function only checks the minimum faces needed to see if there is a winner, only checks
*   the current players last move in order reduce computations.
*  
* Parameters:
*   player
*   p (last coordinate that was touched)
*
* Return:
*   uint8 winner;
*******************************************************************************/
uint8 CheckTicTacToeWin(Player player, Point p) {
    uint8 winner = 0;
    uint8 numDiagUpCount = 0;
    uint8 numDiagDownCount = 0;
    uint8 numHorizCount = 0;
    uint8 numVertCount = 0;
    uint8 catsGameCount = 0;

    // Will check xy, xz, and zy planes individually
    
    // checks xy plane
        // checks diaonal going up
    for (int8 i = -2; i < 3; i++) 
    {
        
        if ( (p.x + i) >= 0 && (p.x + i) <= 2 && (p.y + i) >= 0 && (p.y + i) <= 2 ) 
        {
            
            if ( gameArray[p.x + i][p.y + i][p.z] == player.num )
            {
                // saves the win
                Point w = {p.x + i,p.y + i,p.z};
                winArray[numDiagUpCount] = w;
                
                numDiagUpCount++;
                if (numDiagUpCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagUpCount = 0;
            }
            
        }
    }
        // checks diagonal going down
    for (int8 i = -2; i < 3; i++) 
    {
        
        if ( (p.x + i) >= 0 && (p.x + i) <= 2 && (p.y - i) >= 0 && (p.y - i) <= 2 ) 
        {
            if ( gameArray[p.x + i][p.y - i][p.z] == player.num )
            {
                // saves the win
                Point w = {p.x + i,p.y - i,p.z};
                winArray[numDiagDownCount] = w;
                
                numDiagDownCount++;
                if (numDiagDownCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagDownCount = 0;
            } 
        }
    }
        // checks horizontal line
    for (int8 i = -2; i < 3; i++) 
    {
        
        if (( p.x + i ) >= 0 && ( p.x + i) <= 2) 
        {
            if (gameArray[p.x + i][p.y][p.z] == player.num)
            {
                // saves the win
                Point w = {p.x + i,p.y,p.z};
                winArray[numHorizCount] = w;
                
                numHorizCount++;
                if (numHorizCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
        }
    }
        // checks vertical line
    for (int8 i = -2; i < 3; i++) 
    {

        if (( p.y + i ) >= 0 && ( p.y + i) <= 2) 
        {
            if (gameArray[p.x][p.y + i][p.z] == player.num)
            {   
                // saves the win
                Point w = {p.x,p.y + i,p.z};
                winArray[numVertCount] = w;
                
                numVertCount++;
                if (numVertCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
        }

    }
    
    // resets checks to 0 for the next plane check
    numDiagUpCount = 0;
    numDiagDownCount = 0;
    numHorizCount = 0;
    numVertCount = 0;
    
    // checks xz plane
    for (int8 i = -2; i < 3; i++) 
    {
        // checks diaonal going up
        if ( (p.x + i) >= 0 && (p.x + i) <= 2 && (p.z + i) >= 0 && (p.z + i) <= 2 ) 
        {
            
            if ( gameArray[p.x + i][p.y][p.z + i] == player.num )
            {
                // saves the win
                Point w = {p.x + i, p.y, p.z + i};
                winArray[numDiagUpCount] = w;
                numDiagUpCount++;
                if (numDiagUpCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagUpCount = 0;
            }
            
        }
    }
        // checks diagonal going down
    for (int8 i = -2; i < 3; i++) 
    {
        
        if ( (p.x + i) >= 0 && (p.x + i) <= 2 && (p.z - i) >= 0 && (p.z - i) <= 2 ) 
        {
            if ( gameArray[p.x + i][p.y][p.z - i] == player.num )
            {
                // saves the win
                Point w = {p.x + i,p.y,p.z - i};
                winArray[numDiagDownCount] = w;
                
                numDiagDownCount++;
                if (numDiagDownCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagDownCount = 0;
            } 
        }
    }
        // checks horizontal line
    // x line already checked in previous face check
        // checks vertical line
    for (int8 i = -2; i < 3; i++) 
    {
        
        
        
        if (( p.z + i ) >= 0 && ( p.z + i) <= 2) 
        {
            if (gameArray[p.x][p.y][p.z + i] == player.num)
            {
                // saves the win
                Point w = {p.x,p.y,p.z + i};
                winArray[numVertCount] = w;
                
                numVertCount++;
                if (numVertCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
        }

    }
    
    // resets checks to 0 for the next plane check
    numDiagUpCount = 0;
    numDiagDownCount = 0;
    numHorizCount = 0;
    numVertCount = 0;
    
    // checks zy plane 
        // checks diaonal going up
    for (int8 i = -2; i < 3; i++) 
    {
        
        if ( (p.z + i) >= 0 && (p.z + i) <= 2 && (p.y + i) >= 0 && (p.y + i) <= 2 ) 
        {
            
            if ( gameArray[p.x][p.y + i][p.z + i] == player.num )
            {
                // saves the win
                Point w = {p.x,p.y + i,p.z + i};
                winArray[numDiagUpCount] = w;
                
                numDiagUpCount++;
                if (numDiagUpCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagUpCount = 0;
            }
            
        }
    }
        // checks diagonal going down
    for (int8 i = -2; i < 3; i++) 
    {
        if ( (p.z + i) >= 0 && (p.z + i) <= 2 && (p.y - i) >= 0 && (p.y - i) <= 2 ) 
        {
            if ( gameArray[p.x][p.y - i][p.z + i] == player.num )
            {
                // saves the win
                Point w = {p.x,p.y - i,p.z + i};
                winArray[numDiagDownCount] = w;
                
                numDiagDownCount++;
                if (numDiagDownCount == 3)
                {
                    winner = player.num;
                    return winner;
                }
            }
            else 
            {
                numDiagDownCount = 0;
            } 
        }
    }    
        // checks horizontal line
        // already checked
        
        // checks vertical line
        // already checked

    // Checks for cats game
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                if ((gameArray[i][j][k] != 0))
                {
                    catsGameCount++;
                    if (catsGameCount >= 26) 
                    {
                        winner = 3;
                        return winner;
                    }
                }
            }
        }   
    }
    
    return winner;
}    
/*******************************************************************************
* Function Name: ResetArrays
********************************************************************************
* Summary:
*  resets the all array values and other variables all to 0 and colors all leds to off, 
*  then updates the leds
*  
* Parameters:
*
* Return:
*******************************************************************************/
void ResetArrays(void) {
    
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++) 
            {
                gameArray[i][j][k] = 0;
                colorArray[i][j][k] = StripLights_OFF;
                debounceTouchArray[i][j][k] = 0;
                touchArray[i][j][k] = 0;
            }
        }   
    }
    
    UpdateCubeLights();
    return;
}    
/*******************************************************************************
* Function Name: PropagateColor
********************************************************************************
* Summary:
*  propagates a given color outward from a selected Point {x,y,z}
*  
*  
* Parameters:
*   color
*	Point
*
* Return:
*   none (updates global array)
*******************************************************************************/
/*
void PropagateColor(Point p, uin32 color) 
{
	for (int i = 0; i < 3; i++) 
	{
		if ( (p.x + i) <= 2 && (p.y + i <= 2) && (p.z + i <= 2) )
		{
			
*/
// one cube bounces around in the same pattern as the led wiring
void IdleAnimation1(uint32 count)
{   
    ColorAllCubes(StripLights_OFF);
    uint32 color = StripLights_ColorInc(1);
    
    switch(count%12) 
    {   

        case 0: 
            colorArray[1][0][0] = color;
            break;
        case 1:
            colorArray[2][1][0] = color;
            break;
        case 2:
            colorArray[2][0][1] = color;
            break;
        case 3:
            colorArray[1][0][2] = color;
            break;
        case 4:
            colorArray[2][1][2] = color;
            break;
        case 5:
            colorArray[2][2][1] = color;
            break;
        case 6:
            colorArray[1][2][2] = color;
            break;
        case 7:
            colorArray[0][1][2] = color;
            break;
        case 8:
            colorArray[0][2][1] = color;
            break;
        case 9:
            colorArray[1][2][0] = color;
            break;
        case 10:
            colorArray[0][1][0] = color;
            break;
        case 11:
            colorArray[0][0][1] = color;
            break;
    }
    
    return;
}

/* [] END OF FILE */
