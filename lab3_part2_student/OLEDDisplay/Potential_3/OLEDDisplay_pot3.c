// Include FreeRTOS Libraries
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// UART driver header file
#include "uart_driver.h"
#include "rgb_led.h"

// Include xilinx Libraries
#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xil_cache.h"

// Other miscellaneous libraries
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "sleep.h"
#include "PmodOLED.h"
#include "OLEDControllerCustom.h"

// Files for displaying story
#include "story.h"
#include "effects.h"
#include "command.h"

#define BTN_DEVICE_ID  XPAR_GPIO_INPUTS_BASEADDR
#define LEDS_DEVICE_ID	XPAR_GPIO_LEDS_BASEADDR
#define BTN_CHANNEL    1
#define RGB_CHANNEL 2

#define FRAME_DELAY 50000

// Declaring the devices
XGpio btnInst;
XGpio rgbInst;
PmodOLED oledDevice;

// Display state machine
typedef enum {
    DISPLAY_TITLE,
    DISPLAY_STORY,
    DISPLAY_GAME
} DisplayState;

// Keyboard Queue
QueueHandle_t xKeyboardQueue;
extern QueueHandle_t xRxQueue;
extern QueueHandle_t xTxQueue;

// Function prototypes
void initializeScreen();
extern void processCommand(char *input, char *outputBuffer);
static void oledTask( void *pvParameters );
static void buttonTask( void *pvParameters );
static void keyboardTask(void *pvParameters);
int grphClampXco(int xco);
int grphClampYco(int yco);
int grphAbs(int foo);
void OLED_DrawLineTo(PmodOLED *InstancePtr, int xco, int yco);
void OLED_getPos(PmodOLED *InstancePtr, int *pxco, int *pyco);
void drawTarget(u8 targetX, u8 targetY, u8 width, u8 length);


const u8 orientation = 0x1; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
extern u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters

// New global flag
int oneRingEquipped = 0;

/*
u8 size = 8, origin = 0;
u8 aimx=0, aimy=0;
u8 targetx=30, targety=5;
u8 targetWidth=8, targetLength=15;
int score = 0, lives = 3;
*/

int main()
{
	int status = 0;

	 // orientation: 0 is usually normal, invert: 0 = normal colors
    OLED_Begin(&oledDevice,
               XPAR_GPIO_OLED_BASEADDR,
               XPAR_SPI_OLED_BASEADDR,
               orientation,
               invert);

	// Buttons
	status = XGpio_Initialize(&btnInst, BTN_DEVICE_ID);
	if(status != XST_SUCCESS){
		xil_printf("GPIO Initialization for SSD failed.\r\n");
		return XST_FAILURE;
	}

    // RGB LED
    status = XGpio_Initialize(&rgbInst, LEDS_DEVICE_ID);
    if (status != XST_SUCCESS){
        xil_printf("GPIO Initialization for RGB failed.\r\n");
        return XST_FAILURE;
    }

    // Set RGB as output
    XGpio_SetDataDirection(&rgbInst, RGB_CHANNEL, 0x00);

    // Initialize UART
    status = initializeUART();

    extern XScuGic InterruptController;
    extern XUartPs UART;

    xTxQueue = xQueueCreate(64, sizeof(u8));
    xRxQueue = xQueueCreate(64, sizeof(u8));
    configASSERT(xTxQueue);
    configASSERT(xRxQueue);

    status = setupInterruptSystem(&InterruptController, &UART, UART_INT_IRQ_ID);
    if (status != XST_SUCCESS){
        xil_printf("UART interrupt setup failed\n");
    }

    // Create keyboard queue
    xKeyboardQueue = xQueueCreate(32, sizeof(char));
    configASSERT(xKeyboardQueue);

	xil_printf("Initialization Complete, System Ready!\n");

	xTaskCreate( oledTask					// The function that implements the task.
			   , "screen task"				// Text name for the task, provided to assist debugging only.
			   , configMINIMAL_STACK_SIZE	// The stack allocated to the task. 
			   , NULL						// The task parameter is not used, so set to NULL. 
			   , tskIDLE_PRIORITY			// The task runs at the idle priority. 
			   , NULL
			   );
	xTaskCreate( buttonTask
			   , "button task"
			   , configMINIMAL_STACK_SIZE
			   , NULL
			   , tskIDLE_PRIORITY
			   , NULL
			   );
    xTaskCreate( keyboardTask
               , "keyboard task"
               , configMINIMAL_STACK_SIZE
               , NULL
               , tskIDLE_PRIORITY
               , NULL );
    

	vTaskStartScheduler();


   while(1);

   return 0;
}

static void keyboardTask(void *pvParameters)
{
    char rxChar;
    while (1)
    {
        if (myReceiveData() == pdTRUE)
        {
            rxChar = (char) myReceiveByte();
            // Send character to OLED via queue
            xQueueSend(xKeyboardQueue, &rxChar, 10);
        }

        vTaskDelay(5);
    }
}

static void oledTask(void *pvParameters)
{
    char kbChar;
    char inputBuffer[32];
    char outputBuffer[64];
    int index = 0;

	OLED_SetDrawMode(&oledDevice, 0);
	OLED_SetCharUpdate(&oledDevice, 0);

	int titleFlag = 1;
	int storyFlag = 0;

	while(1)
	{
        if(titleFlag){
            showTitleScreen(&oledDevice);
            titleFlag = 0;
            storyFlag = 1;
        }

        if(storyFlag){
            showStoryCards(&oledDevice);
            storyFlag = 0;
        }

        // Following is handling the keyboard entries
        if(xQueueReceive(xKeyboardQueue, &kbChar, 0) == pdTRUE)
        {
            // Backspacing allowed when typing in a command
            if(kbChar == 8 || kbChar == 127)
            {
                if(index > 0) index--;
                inputBuffer[index] = '\0';
            }

            // Enter key case
            else if(kbChar == '\r' || kbChar == '\n')
            {
                inputBuffer[index] = '\0';
                processCommand(inputBuffer, outputBuffer);
                index = 0;
            }
            // Normal character case
            else
            {
                if(index < sizeof(inputBuffer)-1)
                {
                    inputBuffer[index++] = kbChar;
                    inputBuffer[index] = '\0';
                }
            }
            
            // Display the input and output with dissolve effect
            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 0);
            OLED_PutString(&oledDevice, "> ");
            OLED_PutString(&oledDevice, inputBuffer);
            OLED_Update(&oledDevice);
            vTaskDelay(300);

            showTextWithDissolve(&oledDevice, outputBuffer, 1200);
        }

        vTaskDelay(10);
	}
}

// Button Task
static void buttonTask(void *pvParameters)
{
    u8 buttonVal = 0;

    while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);

        if (buttonVal == 1){  // Equip Sting
            XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);
            invert = 0x1;
            setOLEDInvert(&oledDevice, &invert);
        }
        else if (buttonVal == 2){  // Equip One Ring
            if(oneRingEquipped){
                XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
            }
            // else do nothing until Bilbo finds the ring
        }
        else if (buttonVal == 4){  // Magenta for other action
            XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_MAGENTA);
        }
        else if (buttonVal == 8){  // Reset
            XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_OFF);
            OLED_ClearBuffer(&oledDevice);
            OLED_Update(&oledDevice);
            showTitleScreen(&oledDevice);
            showStoryCards(&oledDevice);
            oneRingEquipped = 0;
        }

        vTaskDelay(10);
    }
}
