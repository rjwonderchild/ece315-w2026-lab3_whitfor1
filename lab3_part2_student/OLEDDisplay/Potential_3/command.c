// command.c
#include "command.h"
#include "effects.h"
#include "xgpio.h"
#include "PmodOLED.h"
#include "rgb_led.h"
#include <string.h>
#include <stdio.h>

// Extern devices
extern PmodOLED oledDevice;
extern XGpio rgbInst;
extern int oneRingEquipped;
#define RGB_CHANNEL 2

// Invert control (Sting)
extern u8 invert;
extern int oneRingEquipped;

// Game states
typedef enum {
    STATE_START,
    STATE_CAVE,
    STATE_GOLLUM,
    STATE_RIDDLE,
    STATE_END
} GameState;

static GameState currentState = STATE_START;

// Function to process user commands
void processCommand(char *input, char *outputBuffer)
{
    // Empty input
    if(strlen(input) == 0)
    {
        strcpy(outputBuffer, "Type a command...");
        return;
    }

    // Reset story
    if(strcmp(input, "reset") == 0)
    {
        currentState = STATE_START;
        strcpy(outputBuffer, "Story reset!");
        return;
    }

    // Command handling based on current state
    switch(currentState)
    {
        case STATE_START:
            if(strcmp(input, "move") == 0)
            {
                strcpy(outputBuffer, "Bilbo falls into a hole!");
                currentState = STATE_CAVE;
            }
            else
            {
                strcpy(outputBuffer, "Try 'move' to start adventure");
            }
            break;

        case STATE_CAVE:
            if(strcmp(input, "look") == 0)
            {
                strcpy(outputBuffer, "Dark cave... something moves...");
            }
            else if(strcmp(input, "equip sting") == 0)
            {
                strcpy(outputBuffer, "You equip Sting! It glows cyan.");
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);

                invert = 0x1;
                setOLEDInvert(&oledDevice, &invert);
            }
            else if(strcmp(input, "equip ring") == 0)
            {
                strcpy(outputBuffer, "You put on the One Ring. You vanish!");
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
                oneRingEquipped = 1;   // now button 2 will work
            }
            else if(strcmp(input, "forward") == 0)
            {
                strcpy(outputBuffer, "You move forward and meet Gollum!");
                currentState = STATE_GOLLUM;
            }
            else
            {
                strcpy(outputBuffer, "Try 'look', 'equip sting', 'equip ring', or 'forward'");
            }
            break;

        case STATE_GOLLUM:
            if(strcmp(input, "talk") == 0)
            {
                strcpy(outputBuffer, "Gollum: I will ask you riddles!");
                currentState = STATE_RIDDLE;
            }
            else
            {
                strcpy(outputBuffer, "Try 'talk' to Gollum");
            }
            break;

        case STATE_RIDDLE:
            // For simplicity, only one riddle
            if(strcmp(input, "time") == 0)
            {
                strcpy(outputBuffer, "Correct! You escape the cave.");
                currentState = STATE_END;
            }
            else if(strcmp(input, "guess") == 0)
            {
                strcpy(outputBuffer, "Wrong! Try again.");
            }
            else
            {
                strcpy(outputBuffer, "Riddle: What has hands but cannot clap?");
            }
            break;

        case STATE_END:
            strcpy(outputBuffer, "Adventure complete! Type 'reset' to play again.");
            break;
    }
}