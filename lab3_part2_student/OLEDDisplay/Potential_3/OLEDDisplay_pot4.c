// Include FreeRTOS Libraries
#include "FreeRTOS.h"
#include "task.h"

// UART driver header file
#include "uart_driver.h"
#include "rgb_led.h"

// Include xilinx Libraries
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"

// Other libraries
#include <stdlib.h>
#include <string.h>
#include "pmodkypd.h"
#include "sleep.h"
#include "PmodOLED.h"
#include "OLEDControllerCustom.h"

// Custom files
#include "story.h"
#include "effects.h"

// Device IDs
#define BTN_DEVICE_ID  XPAR_GPIO_INPUTS_BASEADDR
#define LEDS_DEVICE_ID XPAR_GPIO_LEDS_BASEADDR
#define KYPD_BASE_ADDR XPAR_GPIO_KEYPAD_BASEADDR

#define BTN_CHANNEL 1
#define RGB_CHANNEL 2

#define DEFAULT_KEYPAD "0FED789C456B123A"

// Devices
XGpio btnInst;
XGpio rgbInst;
PmodOLED oledDevice;
PmodKYPD KYPDInst;

// Globals
u8 invert = 0x0;
int oneRingEquipped = 0;
volatile int resetGame = 0;
volatile u8 keypad_val = 'x';

// Prototypes
void InitializeKeypad();
static void oledTask(void *pvParameters);
static void buttonTask(void *pvParameters);
static void keypadTask(void *pvParameters);

// ---------------- MAIN ----------------
int main()
{
    int status;

    InitializeKeypad();

    OLED_Begin(&oledDevice,
               XPAR_GPIO_OLED_BASEADDR,
               XPAR_SPI_OLED_BASEADDR,
               1,
               invert);

    // Buttons
    status = XGpio_Initialize(&btnInst, BTN_DEVICE_ID);
    if(status != XST_SUCCESS) return XST_FAILURE;

    // RGB
    status = XGpio_Initialize(&rgbInst, LEDS_DEVICE_ID);
    if(status != XST_SUCCESS) return XST_FAILURE;

    XGpio_SetDataDirection(&rgbInst, RGB_CHANNEL, 0x00);

    xil_printf("System Ready!\n");

    xTaskCreate(oledTask, "OLED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(buttonTask, "BTN", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(keypadTask, "KYPD", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1);
    return 0;
}

// ---------------- KEYPAD INIT ----------------
void InitializeKeypad()
{
    KYPD_begin(&KYPDInst, KYPD_BASE_ADDR);
    KYPD_loadKeyTable(&KYPDInst, (u8*) DEFAULT_KEYTABLE);
}

// ---------------- KEYPAD TASK ----------------
static void keypadTask(void *pvParameters)
{
    u16 keystate;
    XStatus status;
    u8 key;

    while (1) {
        keystate = KYPD_getKeyStates(&KYPDInst);
        status = KYPD_getKeyPressed(&KYPDInst, keystate, &key);

        if (status == KYPD_SINGLE_KEY) {
            keypad_val = key;
        }

        vTaskDelay(50);
    }
}

// ---------------- OLED TASK ----------------
static void oledTask(void *pvParameters)
{
    OLED_SetDrawMode(&oledDevice, 0);
    OLED_SetCharUpdate(&oledDevice, 0);

    int titleFlag = 1;
    int storyFlag = 0;
    int gameState = 0;

    while(1)
    {
        if(resetGame){
            OLED_ClearBuffer(&oledDevice);
            OLED_Update(&oledDevice);

            titleFlag = 1;
            storyFlag = 0;
            gameState = 0;
            resetGame = 0;
        }

        // Title
        if(titleFlag){
            showTitleScreen(&oledDevice);
            titleFlag = 0;
            storyFlag = 1;
        }

        // Story
        if(storyFlag){
            showStoryCards(&oledDevice);
            storyFlag = 0;
        }

        // Consume key safely (prevents repeat)
        u8 key = keypad_val;
        keypad_val = 'x';
        int lastGameState = -1;
        u8 lastKey = 'x';

        // Only redraw if gameState or key changed
        if(gameState != lastGameState || key != lastKey)
        {
            OLED_ClearBuffer(&oledDevice);

            // -------- STATE 0: EQUIP SWORD --------
            if(gameState == 0)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "Use something?");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Equip sword");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Wait");

                if(key == '2')
                {
                    showTextWithDissolve(&oledDevice, "You draw Sting!", 1200);
                    showTextWithDissolve(&oledDevice, "Press button 1", 1200);
                    showTextWithDissolve(&oledDevice, "It glows blue...", 1200);
                    gameState = 1;
                    vTaskDelay(300);
                }   else if(key == '8') {
                showTextWithDissolve(&oledDevice, "You hesitate...", 1200);
                }
            }

            // -------- STATE 1 --------
            else if(gameState == 1)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "A tunnel lies ahead");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Enter");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Stay");

                if(key == '2'){
                    showTextWithDissolve(&oledDevice, "You move deeper...", 1200);
                    gameState = 2;
                    vTaskDelay(300);
                }
                else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "You stay still...", 1200);
                }
            }

            // -------- STATE 2 --------
            else if(gameState == 2)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "Gollum!");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Talk");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Hide");

                if(key == '2'){
                    showTextWithDissolve(&oledDevice, "Riddle time!", 1200);
                    gameState = 3;
                    vTaskDelay(300);
                } 
                else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "You hide...", 1200);
                }
            }

            // -------- STATE 3 --------
            else if(gameState == 3)
            {

                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "What has hands but cannot clap?");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Clock");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Sword");

                if(key == '2'){
                    showTextWithDissolve(&oledDevice, "Correct!", 1200);
                    showTextWithDissolve(&oledDevice, "You find a gold ring!", 1200);
                    showTextWithDissolve(&oledDevice, "It feels precious", 1200);
                    oneRingEquipped = 1;
                    gameState = 4;

                    vTaskDelay(300);
                } else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "Wrong!", 1200);
                }
            }

                    // -------- STATE 4: END --------
            else if(gameState == 4) {

                    OLED_SetCursor(&oledDevice, 0, 1);
                    OLED_PutString(&oledDevice, "You escape!");

                    OLED_SetCursor(&oledDevice, 0, 3);
                    OLED_PutString(&oledDevice, "BTN2 = Ring");
                }

            OLED_Update(&oledDevice);
            lastGameState = gameState;
            lastKey = key;
        }
    
        vTaskDelay(100);

    }
}

// ---------------- BUTTON TASK ----------------
static void buttonTask(void *pvParameters)
{
    u8 buttonVal, lastVal = 0;

    while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);

        if (buttonVal != lastVal)   // detect change
        {
            if (buttonVal == 1){
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);
                invert = 1;
                setOLEDInvert(&oledDevice, &invert);
            }
            else if (buttonVal == 2){
                if(oneRingEquipped){
                    XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
                }
            }
            else if (buttonVal == 8){
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_OFF);
                invert = 0x0;
                oneRingEquipped = 0;
                resetGame = 1;
            }
        }

        lastVal = buttonVal;
        vTaskDelay(50);
    }
}
