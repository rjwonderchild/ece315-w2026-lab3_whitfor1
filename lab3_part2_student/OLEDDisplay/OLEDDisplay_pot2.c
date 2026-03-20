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
#include "pmodkypd.h"
#include "sleep.h"
#include "PmodOLED.h"
#include "OLEDControllerCustom.h"

// Game State libraries
#include "game_state.h"
#include "object.h"
#include "toggle.h"

// Files for displaying story
#include "story.h"

#define BTN_DEVICE_ID  XPAR_GPIO_INPUTS_BASEADDR
#define LEDS_DEVICE_ID	XPAR_GPIO_LEDS_BASEADDR
#define BTN_CHANNEL    1
#define RGB_CHANNEL 2

#define FRAME_DELAY 50000

// Declaring the devices
XGpio btnInst;
XGpio rgbInst;
PmodOLED oledDevice;

// Keyboard Queue
QueueHandle_t xKeyboardQueue;
extern QueueHandle_t xRxQueue;
extern QueueHandle_t xTxQueue;

// Function prototypes
void initializeScreen();
static void oledTask( void *pvParameters );
static void buttonTask( void *pvParameters );
static void keyboardTask(void *pvParameters);
int grphClampXco(int xco);
int grphClampYco(int yco);
int grphAbs(int foo);
void OLED_DrawLineTo(PmodOLED *InstancePtr, int xco, int yco);
void OLED_getPos(PmodOLED *InstancePtr, int *pxco, int *pyco);
void drawTarget(u8 targetX, u8 targetY, u8 width, u8 length);
static void HandleStingButtonPress(void);
static void HandleRingButtonPress(void);
static void UpdateRgbFromGameState(void);

const u8 orientation = 0x1; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters
u8 size = 8, origin = 0;
u8 aimx=0, aimy=0;
u8 targetx=30, targety=5;
u8 targetWidth=8, targetLength=15;
int score = 0, lives = 3;

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
	XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_OFF);

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


// -------------------------
// Fast Random Pixel Dissolve
// -------------------------

static u32 lfsrFast = 0xA341316C; // pseudo-random seed

// Simple pseudo-random generator (8-bit)
static u8 nextNoiseBitFast(void) {
    lfsrFast ^= lfsrFast << 13;
    lfsrFast ^= lfsrFast >> 17;
    lfsrFast ^= lfsrFast << 5;
    return (u8)(lfsrFast & 0xFF);
}

// Fast dissolve function
void dissolveScreenFast(void) {
    int i, j;
    int x, y;

    // Clear a lot of pixels quickly
    for (i = 0; i < 200; i++) {           // outer loop controls iterations
        for (j = 0; j < 110; j++) {         // erase 100 pixels per iteration
            x = nextNoiseBitFast() % OledColMax;
            y = nextNoiseBitFast() % OledRowMax;

            OLED_SetDrawColor(&oledDevice, 0); // set pixel off
            OLED_MoveTo(&oledDevice, x, y);
            OLED_DrawPixel(&oledDevice);
        }

        // Update the screen periodically to animate
        OLED_Update(&oledDevice);

        // Tiny delay to let FreeRTOS run other tasks (optional)
        vTaskDelay(1);
    }

    OLED_Update(&oledDevice); // final update to ensure all pixels cleared
}

static void oledTask( void *pvParameters )
{
	u8 buttonVal = 0;
	char temp[10];
	xil_printf("UART and SPI opened for PmodOLED Demo\n");
	OLED_SetDrawMode(&oledDevice, 0);
	// Turn automatic updating off
	OLED_SetCharUpdate(&oledDevice, 0);

    // Variables needed for keyboard entry.
    char kbChar;
    char inputBuffer[32];   // stores typed characters
    int index = 0;

    // Story Flags
    int titleFlag = 1;
    int storyCardFlag = 0;

	while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);
        // Check for keyboard input
/*
        if (xQueueReceive(xKeyboardQueue, &kbChar, 0) == pdTRUE) {

            // Handle ENTER key (carriage return)
            if (kbChar == '\r') {
                inputBuffer[index] = '\0';  // null terminate

                OLED_ClearBuffer(&oledDevice);
                OLED_SetCursor(&oledDevice, 0, 1);
                OLED_PutString(&oledDevice, inputBuffer);
                OLED_Update(&oledDevice);

                index = 0; // reset for next input
            }
            else {
                if (index < sizeof(inputBuffer) - 1) {
                    inputBuffer[index++] = kbChar;
                }
            }
        }
*/
        if (titleFlag) {
            showTitleScreen(&oledDevice);

            titleFlag = 0;
            storyCardFlag = 1;
        }

        if (storyCardFlag) {
            showStoryCards(&oledDevice);

            storyCardFlag = 0;
        }
	}
}

static void UpdateRgbFromGameState(void)
{
    switch (gRgbMode)
    {
    case RGB_MODE_CYAN:
        XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_CYAN);
        break;

    case RGB_MODE_YELLOW:
        XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_YELLOW);
        break;

    default:
        XGpio_DiscreteWrite(&rgbInst, RGB_CHANNEL, RGB_OFF);
        break;
    }
}

static void HandleStingButtonPress(void)
{
    if (!gHasSting)
    {
        return;
    }

    gStingDrawn = !gStingDrawn;

    if (gStingDrawn)
    {
        if (lampOff->location == player)
        {
            toggleLamp();
        }
    }
    else
    {
        if (lampOn->location == player)
        {
            toggleLamp();
        }
    }

    GameState_UpdateRgbMode();
    UpdateRgbFromGameState();
}

static void HandleRingButtonPress(void)
{
    if (!gHasRing)
    {
        return;
    }

    gRingDrawn = !gRingDrawn;
    GameState_UpdateRgbMode();
    UpdateRgbFromGameState();
}

static void buttonTask(void *pvParameters)
{
    u8 buttonVal = 0;
    u8 prevButtonVal = 0;

    while (1)
    {
        buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);

        /*
         * Edge detect: only act when a button is newly pressed.
         * Adjust button mapping as needed 
         */
        if ((buttonVal & 0x01) && !(prevButtonVal & 0x01))
        {
            HandleStingButtonPress();
        }

        if ((buttonVal & 0x02) && !(prevButtonVal & 0x02))
        {
            HandleRingButtonPress();
        }

        prevButtonVal = buttonVal;
        vTaskDelay(10);
    }
}
