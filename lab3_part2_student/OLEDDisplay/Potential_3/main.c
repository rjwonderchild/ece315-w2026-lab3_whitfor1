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

#define DEFAULT_KEYTABLE "0FED789C456B123A"

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
volatile int button1Locked = 1;  // Locks button 1 usage
volatile int button2Locked = 1;  // Locks button 2 usage
volatile int button8Locked = 1;  // Locks reset until splash done

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

    int lastGameState = -1;
    u8 lastKey = 'x';

    while(1)
    {
        if(resetGame){
            invert = 0x0;               // ensure background reset
            button1Locked = 1;
            button2Locked = 1;
            button8Locked = 1;
            oneRingEquipped = 0;
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

        u8 key = keypad_val;
        keypad_val = 'x';

        // Redraw only if state or key changes
        if(gameState != lastGameState || key != lastKey)
        {
            OLED_ClearBuffer(&oledDevice);

            // -------- STATE 0 --------
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
                    button1Locked = 0;
                    showTextWithDissolve(&oledDevice, "You draw Sting!", 1200);
                    showTextWithDissolve(&oledDevice, "It glows blue...", 1200);
                    showTextWithDissolve(&oledDevice, "Press button 1  to equip", 1200);

                    // WAIT until button 1 pressed AND button is not locked
                    while(button1Locked == 0){
                        u8 btnVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);
                        if(btnVal == 1){
                            XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);
                            invert = 1;
                            button1Locked = 1;       // lock button 1 after use
                            setOLEDInvert(&oledDevice, &invert);
                        }
                        vTaskDelay(50);
                    }

                    gameState = 1;
                    vTaskDelay(300);
                }
                else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "You hesitate...", 1200);
                }
            }

            // -------- STATE 1 --------
            else if(gameState == 1)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "A tunnel lies   ahead");

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
                    showTextWithDissolve(&oledDevice, "Gollum speaks...", 1200);
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
                OLED_PutString(&oledDevice, "What has hands  but cannot clap?");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Sword");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Clock");

                if(key == '8'){
                    showTextWithDissolve(&oledDevice, "Correct!", 1200);
                    showTextWithDissolve(&oledDevice, "You find a gold ring!", 1200);
                    showTextWithDissolve(&oledDevice, "It calls to you...", 1200);
                    gameState = 4;
                    vTaskDelay(300);
                } else if(key == '2') {
                    showTextWithDissolve(&oledDevice, "Wrong!", 1200);
                }
            }

            // -------- STATE 4 --------
            else if(gameState == 4)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "Equip ring?");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Put it on.");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Resist.");

                if(key == '2'){
                    oneRingEquipped = 1;
                    button2Locked = 0;
                    showTextWithDissolve(&oledDevice, "Voices echo     around you", 1200);
                    showTextWithDissolve(&oledDevice, "Press button 2", 1200);

                    // WAIT until button 2 pressed AND button is not locked
                    while(button2Locked == 0){
                        u8 btnVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);
                        if(btnVal == 2){
                            XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
                            button2Locked = 1;       // lock button 2 after use
                        }
                        vTaskDelay(50);
                    }
                    
                    showTextWithDissolve(&oledDevice, "You turn        invisible", 1200);
                    showTextWithDissolve(&oledDevice, "Gollum cannot   see you", 1200);
                    
                    gameState = 5;
                    vTaskDelay(300);
                } else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "About to   be    attacked!", 1200);
                }
            }

            // -------- STATE 5 --------
            else if(gameState == 5)
            {
                OLED_SetCursor(&oledDevice, 0, 0);
                OLED_PutString(&oledDevice, "You see an exit");

                OLED_SetCursor(&oledDevice, 0, 2);
                OLED_PutString(&oledDevice, "2: Go towards it");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "8: Look around");

                if(key == '2'){
                    oneRingEquipped = 1;
                    showTextWithDissolve(&oledDevice, "You run towards the exit", 1200);
                    showTextWithDissolve(&oledDevice, "Gollum starts   screaming", 1200);   
                    showTextWithDissolve(&oledDevice, "MY", 300);
                    showTextWithDissolve(&oledDevice, "PRECIOUS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", 700);
                    showTextWithDissolve(&oledDevice, "You see daylight", 1500);

                    gameState = 6;
                    vTaskDelay(300);
                } else if(key == '8') {
                    showTextWithDissolve(&oledDevice, "Its just you andGollum", 1500);
                }
            }

            // -------- STATE 6 --------
            else if(gameState == 6)
            {
                button8Locked = 0;
                OLED_SetCursor(&oledDevice, 0, 1);
                OLED_PutString(&oledDevice, "You escape!");

                OLED_SetCursor(&oledDevice, 0, 3);
                OLED_PutString(&oledDevice, "BTN8 = Reset");
            }

            OLED_Update(&oledDevice);

            lastGameState = gameState;
            lastKey = key;
        }

        vTaskDelay(50);
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
            if (buttonVal == 1 && !button1Locked){
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);
                invert = 0x1;
                button1Locked = 1;       // lock button 1 after use
                setOLEDInvert(&oledDevice, &invert);
            }
            else if (buttonVal == 2 && !button2Locked){
                if(oneRingEquipped){
                    XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
                    button2Locked = 1;
                }
            }
            else if (buttonVal == 8 && !button8Locked){
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_OFF);
                invert = 0x0;
                setOLEDInvert(&oledDevice, &invert);
                oneRingEquipped = 0;
                button1Locked = 1;
                button2Locked = 1;
                button8Locked = 1;
                resetGame = 1;
            }
        }

        lastVal = buttonVal;
        vTaskDelay(50);
    }
}
