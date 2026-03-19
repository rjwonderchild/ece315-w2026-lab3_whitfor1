// Include FreeRTOS Libraries
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

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


#define BTN_DEVICE_ID  XPAR_GPIO_INPUTS_BASEADDR
#define KYPD_DEVICE_ID XPAR_GPIO_KEYPAD_BASEADDR
#define KYPD_BASE_ADDR XPAR_GPIO_KEYPAD_BASEADDR
#define BTN_CHANNEL    1

#define FRAME_DELAY 50000

// keypad key table
#define DEFAULT_KEYTABLE 	"0FED789C456B123A"

// Declaring the devices
XGpio btnInst;
PmodOLED oledDevice;
PmodKYPD 	KYPDInst;

// Function prototypes
void InitializeKeypad();
void initializeScreen();
static void keypadTask( void *pvParameters );
static void oledTask( void *pvParameters );
static void buttonTask( void *pvParameters );
int grphClampXco(int xco);
int grphClampYco(int yco);
int grphAbs(int foo);
void OLED_DrawLineTo(PmodOLED *InstancePtr, int xco, int yco);
void OLED_getPos(PmodOLED *InstancePtr, int *pxco, int *pyco);
void drawTarget(u8 targetX, u8 targetY, u8 width, u8 length);


const u8 orientation = 0x1; // Set up for Normal PmodOLED(false) vs normal
                            // Onboard OLED(true)
const u8 invert = 0x0; // true = whitebackground/black letters
                       // false = black background /white letters
u8 keypad_val = 'x';
u8 size = 8, origin = 0;
u8 aimx=0, aimy=0;
u8 targetx=30, targety=5;
u8 targetWidth=8, targetLength=15;
int score = 0, lives = 3;

int main()
{
	int status = 0;
	// Initialize Devices
	InitializeKeypad();

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


	xil_printf("Initialization Complete, System Ready!\n");


	xTaskCreate( keypadTask					// The function that implements the task.
			   , "keypad task"				// Text name for the task, provided to assist debugging only.
			   , configMINIMAL_STACK_SIZE	// The stack allocated to the task.
			   , NULL						// The task parameter is not used, so set to NULL.
			   , tskIDLE_PRIORITY			// The task runs at the idle priority.
			   , NULL
			   );


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

	vTaskStartScheduler();


   while(1);

   return 0;
}


void InitializeKeypad()
{
   KYPD_begin(&KYPDInst, KYPD_BASE_ADDR);
   KYPD_loadKeyTable(&KYPDInst, (u8*) DEFAULT_KEYTABLE);
}


static void keypadTask( void *pvParameters )
{
   u16 keystate;
   u8 flag = 0;
   XStatus status, last_status = KYPD_NO_KEY;
   u8 new_key = 'x';

   const TickType_t xDelay = 25 / portTICK_RATE_MS;

   xil_printf("Pmod KYPD app started. Press any key on the Keypad.\r\n");
   while (1) {
	  // Capture state of the keypad
	  keystate = KYPD_getKeyStates(&KYPDInst);

	  // Determine which single key is pressed, if any
	  // if a key is pressed, store the value of the new key in new_key
	  status = KYPD_getKeyPressed(&KYPDInst, keystate, &new_key);

	  // Print key detect if a new key is pressed or if status has changed
	  if (status == KYPD_SINGLE_KEY){
	  } else if (status == KYPD_MULTI_KEY && status != last_status){
		 xil_printf("Error: Multiple keys pressed\r\n");
	  } else if (status == KYPD_NO_KEY && last_status == KYPD_NO_KEY ){
		  new_key = '0';
	  }

	  last_status = status;
	  keypad_val = new_key;
	  if (keypad_val == '2'){
		  flag = !flag;
		  if(aimy > 0 && flag) {
			  aimy-=1;
		  }
		  if(origin > 0 && flag){
			  origin--;
		  }
	  } else if (keypad_val == '8'){
		  flag = !flag;
		  if(aimy < (OledRowMax - size - 1) && flag) {
			  aimy+=1;
		  }
		  if(origin < (OledRowMax - size - 1) && flag) {
			  origin++;
		  }
	  } else if (keypad_val == '6'){
		  flag = !flag;
		  if(aimx < (OledColMax - 1) && flag) {
			  aimx+=1;
		  }
	  } else if (keypad_val == '5'){
		  flag = !flag;
	  } else if (keypad_val == '4'){
		  flag = !flag;
		  if(aimx > 0 && flag) {
			  aimx-=1;
		  }
	  }
	  vTaskDelay(xDelay); // Scanning Delay
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
static void dissolveScreenFast(void) {
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

    // Variables needed for the title card and transition
    int titleFlag = 1;
    
    // Variables needed for the story setting information
    int storyCardFlag;

	while(1){
        buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);
        if (titleFlag) {
            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "The Lord of the Rings");
            OLED_Update(&oledDevice);

            vTaskDelay(1500);

            dissolveScreenFast();
            
            titleFlag = 0;
            storyCardFlag = 1;
        }

        if (storyCardFlag) {
            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "Where am I?...");
            OLED_Update(&oledDevice);

            vTaskDelay(1500);
            dissolveScreenFast();

            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "Who am I?");
            OLED_Update(&oledDevice);

            vTaskDelay(1500);
            dissolveScreenFast();

            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "Ah... I'm Bilbo");
            OLED_Update(&oledDevice);

            vTaskDelay(1500);
            dissolveScreenFast();

            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "Its dark in here");
            OLED_Update(&oledDevice);

            vTaskDelay(1500);
            dissolveScreenFast();

            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "There must be a way out of here");
            OLED_Update(&oledDevice);

            vTaskDelay(2500);
            dissolveScreenFast();

            OLED_ClearBuffer(&oledDevice);
            OLED_SetCursor(&oledDevice, 0, 1);
            OLED_PutString(&oledDevice, "Is there something I can use to see?");
            OLED_Update(&oledDevice);

            storyCardFlag = 0;
        }
	}
}


static void buttonTask(void *pvParameters)
{
	u8 buttonVal = 0;
    u8 lastButtonVal = 0;

	while(1){
		buttonVal = XGpio_DiscreteRead(&btnInst, BTN_CHANNEL);
		
        if (buttonVal == 1 && lastButtonVal == 0){
            if (lives > 0) {
			//checkShot();
            }
		} else if (buttonVal == 1 && lives == 0){
			xil_printf("game over, reset with BTN3\n");
		} else if (buttonVal == 8 && lastButtonVal == 0){
			xil_printf("reset\n");
			lives = 3;
			score = 0;
		}

        lastButtonVal = buttonVal;
		vTaskDelay(10);
	}
}
