

/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "myADC.h"
#include "gfx.h"
#include "main.h"
#include "aprilia.h"
#include "Aprilia-4.h"
#include "console.h"

/*
 * Globals definition
 */

//#ifdef BOOTSCREEN
//static GConsoleObject			gc;
static GHandle					ghc;
//#endif

//static GSourceHandle 			mouse;
#ifdef LOGO
static GHandle					ghApriliaLogo, ghAprilia;
#endif
//static GEventMouse				*pem;
//static GEvent*					pe;
//static GListener				gl;
//static GSourceHandle			gs;
static GHandle					ghStatus1, ghStatus2;
static GHandle					ghConsole;
static GHandle					ADClabel, ADClabel2;
static GHandle					ADCvalue, ADCvalue2;
static GHandle					ICU1label, ICU1value;
static coord_t 					width, height;
static coord_t					bHeight, bWidth;
static coord_t					swidth, sheight;
static font_t					font;

unsigned int					i;
uint16_t						average;
uint8_t							console;
uint32_t 						sum;
uint8_t							ADC2status;
char							Result[6];
uint16_t 						j;

/* ICU variables
 *
 */
icucnt_t last_width, last_period;

/*
 * Internal Reference Voltage, according to ST this is 1.21V typical
 * with -40°C<T<+105°C its Min: 1.18V, Typ 1.21V, Max: 1.24V
 */
#define VREFINT 121
/*
 * The measured Value is initialized to 2^16/3V*2.21V
 */
uint32_t VREFMeasured = 26433;

/*
* Defines for single scan conversion
*/
#define ADC_GRP1_NUM_CHANNELS 2
#define ADC_GRP1_BUF_DEPTH 512
/*
* Buffer for single conversion
*/
static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

/*
 * Defines for continuous scan conversions
 */
#define ADC_GRP2_NUM_CHANNELS   2
#define ADC_GRP2_BUF_DEPTH      512
static adcsample_t samples2[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

/* ICU part
 *
 *
 */
static void icuwidthcb(ICUDriver *icup) {

  palSetPad(GPIOD, GPIOD_LED4);
  last_width = icuGetWidth(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  palClearPad(GPIOD, GPIOD_LED4);
  last_period = icuGetPeriod(icup);
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  10000,                                    /* 10kHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
};

void ICUinit(void)
{
	  icuStart(&ICUD5, &icucfg);
	  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
	  icuEnable(&ICUD5);
	  chThdSleepMilliseconds(500);
}
/* End of ICU part
 *
 *
 */

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
	(void)adcp;
	(void)err;
	palSetPad(GPIOD, GPIOD_LED4);
}

/*
* ADC conversion group.
* Mode: Linear buffer, 8 samples of 1 channel, SW triggered.
* Channels: IN11.
*/
static const ADCConversionGroup adcgrpcfg1 = {
	FALSE, //circular buffer mode
	ADC_GRP1_NUM_CHANNELS, //Number of the analog channels
	NULL, //Callback function (not needed here)
	adcerrorcallback, //Error callback
	0, /* CR1 */
	ADC_CR2_SWSTART, /* CR2 */
	ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3), //sample times ch10-18
	0, //sample times ch0-9
	ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS), //SQR1: Conversion group sequence 13...16 + sequence length
	0, //SQR2: Conversion group sequence 7...12
	ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) //SQR3: Conversion group sequence 1...6
};

static const ADCConversionGroup adcgrpcfg2 = {
  FALSE,                     //circular buffer mode
  ADC_GRP2_NUM_CHANNELS,    //Number of the analog channels
  NULL,              //Callback function
  adcerrorcallback,         //Error callback
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480) | ADC_SMPR1_SMP_VREF(ADC_SAMPLE_480),  //sample times ch10-18
  0,                                                                        //sample times ch0-9
  ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),                                   //SQR1: Conversion group sequence 13...16 + sequence length
  0, //SQR2
  ADC_SQR2_SQ10_N(ADC_CHANNEL_SENSOR) | ADC_SQR2_SQ9_N(ADC_CHANNEL_VREFINT)  //SQR3
};

void myADCinit(void){
	palSetGroupMode(GPIOC, PAL_PORT_BIT(1), 0, PAL_MODE_INPUT_ANALOG);
	adcStart(&ADCD1, NULL);
	//enable temperature sensor and Vref
	adcSTM32EnableTSVREFE();
}

int uitoa(unsigned int value, char * buf, int max) {
		int n = 0;
		int i = 0;
		unsigned int tmp = 0;
		if (NULL == buf) {
		return -3;
		}
		if (2 > max) {
		return -4;
		}
		i=1;
		tmp = value;
		if (0 > tmp) {
		tmp *= -1;
		i++;
		}
		for (;;) {
		tmp /= 10;
		if (0 >= tmp) {
		break;
		}
		i++;
		}
		if (i >= max) {
		buf[0] = '?';
		buf[1] = 0x0;
		return 2;
		}
		n = i;
		tmp = value;
		if (0 > tmp) {
		tmp *= -1;
		}
		buf[i--] = 0x0;
		for (;;) {
		buf[i--] = (tmp % 10) + '0';
		tmp /= 10;
		if (0 >= tmp) {
		break;
		}
		}
		if (-1 != i) {
		buf[i--] = '-';
		}
		return n;
}

void readADC(void)
{
	uint32_t i;
	char Result2[6];

	adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
	  ADC2status = 0;
	  sum=0;
	  for (i=0;i<=ADC_GRP1_BUF_DEPTH;i++){
	      //chprintf(chp, "%d  ", samples1[i]);
	      sum += samples1[i];
	  }

	  sum = ((uint64_t)sum)*VREFINT/(ADC_GRP1_BUF_DEPTH/16*VREFMeasured/100);

	  sprintf ( Result, "%i", (uint16_t)sum/10000 ); // megadja a feszültség egész részét
	  sprintf ( Result2, ".%iV", (uint16_t)sum%10000 );
	  strncat(Result, Result2, 6);

//	  sprintf ( Result, "%i", (uint16_t)sum/10000 );
	  gwinSetText(ghStatus1, Result, TRUE);
//	  sprintf ( Result, "%i", (uint16_t)sum%10000 );
//	  gwinSetText(ghStatus2, Result, TRUE);
	  //memset (Result, 0, sizeof(Result));
	  //uitoa(sum, Result, sizeof(Result));
	  gwinSetText(ADCvalue, Result, TRUE);
	  //prints the averaged value with 4 digits precision
	  chprintf((BaseSequentialStream *)&SD2, "Measured: %U.%04UV\r\n", sum/10000, sum%10000);
	/*
	i = avg;
	sprintf ( Result, "%i", i ); // %d makes the result be a decimal integer

	  for (i=0; i<=4; i++)
	  {
		  chprintf( (BaseSequentialStream *)&SD2, "%c", Result[i] );
	  }
	  chprintf( (BaseSequentialStream *)&SD2, "\r\n", NULL);
	*/
}

void readVoltage(void)
{
	uint8_t thisTemp;
	uint32_t i;
	char Result2[6];

	adcStartConversion(&ADCD1, &adcgrpcfg2, samples2, ADC_GRP2_BUF_DEPTH);

	  sum=0;

	  for (i=0;i<=ADC_GRP2_BUF_DEPTH;i++){
	      sum += samples2[i];
	  }
	  sum /= ADC_GRP2_BUF_DEPTH;

	  thisTemp = (((int64_t)sum)*VREFINT*400/VREFMeasured-30400)+2500;
	  chprintf((BaseSequentialStream *)&SD2, "Temperatur: %d.%2U°C\r\n", thisTemp/100,thisTemp%100);

	  sprintf(Result, "%d", (uint8_t)thisTemp/100);
	  sprintf(Result2, ".%iC", (uint8_t)thisTemp%100);
	  strncat(Result, Result2, 6);
	  gwinSetText(ADCvalue2, Result, TRUE);
}


static WORKING_AREA(waThread2, 2048);
static msg_t Thread2(void *arg) {

  (void)arg;
  chRegSetThreadName("ADC blinker");
  while (TRUE) {
#ifdef DEBUG_TO_SERIAL
  chprintf( (BaseSequentialStream *)&SD2, "ADC sampling\r\n", NULL );
#endif
  if (console == 1)
  {
	  gwinPrintf(ghc, "ADC sampling\r\n");
  }
  palTogglePad(GPIOD, GPIOD_LED4);       /* Orange.  */
  chSysLockFromIsr();
    readADC();
    readVoltage();
  chSysUnlockFromIsr();
  chThdSleepMilliseconds(1000);
  }
  return 0;
}

#ifdef LOGO
static void CreateLogo(void)
{
	GWidgetInit	wi;
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;
	wi.g.show = TRUE;

	wi.g.x = 0; wi.g.y = 0; wi.g.width = 320; wi.g.height = 240;
	ghApriliaLogo = gwinImageCreate(NULL, &wi.g);
	gwinImageOpenMemory(ghApriliaLogo, Aprilia_4);
	gwinImageCache(ghApriliaLogo);

	chThdSleepMilliseconds(1000);

	wi.g.x = 0; wi.g.y = 0; wi.g.width = 320; wi.g.height = 240;
	ghAprilia = gwinImageCreate(NULL, &wi.g);
	gwinImageOpenMemory(ghAprilia, aprilia);
	gwinImageCache(ghAprilia);

	chThdSleepMilliseconds(1000);
}
#endif

static void mysave(uint16_t instance, const uint8_t *calbuf, size_t sz)
{
	/*
    (void)instance;
    (void)calbuf;
    (void)sz;
    memcpy(&t_calibration, calbuf, (uint8_t) sz);
	*/
	GFILE* f;
	uint8_t bsize;

	(void)instance;

	bsize = (uint8_t)sz;

	f = gfileOpen("calib.gfx", "w");
	gfileWrite(f, (void*)&bsize, 1);
	gfileWrite(f, (void*)calbuf, bsize);
	gfileClose(f);

	chprintf( (BaseSequentialStream *)&SD2, "%s\r\n", calbuf );
	chprintf( (BaseSequentialStream *)&SD2, "%i\r\n", bsize );
}

static const char *myload(uint16_t instance)
	{
		GFILE* f;
		char* buf;
		uint8_t bsize;

		(void)instance;

		f = gfileOpen("calib.gfx", "r");

		gfileRead(f, (void*)&bsize, 1);
		buf = gfxAlloc(bsize);
		gfileRead(f, (void*)buf, bsize);

		gfileClose(f);
		buf = "/x3d/x84/x21/xa6/x39/x65/x11/x4e/xc1/x51/xda/xf1/xb9/xfe/x85/x01/xbd/xb8/xec/xa2/x43/xb0/xcb/x9f";
		return buf;
}

#if BOOTSCREEN
static void bootScreen(void)
{
    gdispFillStringBox(0, 0, width, bHeight, "Boot", font, Red, White, justifyLeft);

	// Create our main display window
	{
		GWindowInit				wi;
		gwinClearInit(&wi);
		wi.show = TRUE;
		wi.x = 0;
		wi.y = bHeight;
		wi.width = width;
		wi.height = height-bHeight;
		ghc = gwinConsoleCreate(&gc, &wi);
	}
	gwinPrintf(ghc, "Boot finished.\n");
	chThdSleepMilliseconds(500);
}
#endif

static void createWidgets(void)
{
	gwinSetDefaultFont(gdispOpenFont("UI2"));
	gwinSetDefaultStyle(&WhiteWidgetStyle, FALSE);
	gdispClear(White);

	bHeight = gdispGetFontMetric(font, fontHeight)+2;

	// apply default settings
	GWidgetInit	wi;
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;
	wi.g.show = TRUE;

	// create two status label
	//status 1
	wi.g.y = sheight-bHeight;
	wi.g.x = 0;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "Status1";

	ghStatus1 = gwinLabelCreate(NULL, &wi);

	// status 2
	wi.g.y = sheight-bHeight;
	wi.g.x = 160;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "Status2";

	ghStatus2 = gwinLabelCreate(NULL, &wi);

	// create ADC label
	// ADClabel1

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.width = 50;
	wi.g.height = bHeight;
	wi.text = "ADC1:";

	ADClabel = gwinLabelCreate(NULL, &wi);
	// ADClabel1

	wi.g.y = 0;
	wi.g.x = 40;
	wi.g.width = 50;
	wi.g.height = bHeight;
	wi.text = "ADC value";

	ADCvalue = gwinLabelCreate(NULL, &wi);

	// create ADC label
	// ADClabel1

	wi.g.y = bHeight;
	wi.g.x = 0;
	wi.g.width = 70;
	wi.g.height = bHeight;
	wi.text = "Core temp:";

	ADClabel2 = gwinLabelCreate(NULL, &wi);
	// ADClabel1

	wi.g.y = bHeight;
	wi.g.x = 75;
	wi.g.width = 50;
	wi.g.height = bHeight;
	wi.text = "ADC value2";

	ADCvalue2 = gwinLabelCreate(NULL, &wi);

	//create console button
	bWidth = gdispGetStringWidth("Console", font);

	wi.g.y = 10; //sheight-(bHeight*4);
	wi.g.x = swidth-70;
	wi.g.width = 50;
	wi.g.height = bHeight+4;
	wi.text = "Console";
	ghConsole = gwinButtonCreate(NULL, &wi);

	//create console BMP button
	/*
	wi.g.x = swidth-85; wi.g.y = 40; wi.g.width = 84; wi.g.height = 23;
	ghbConsole = gwinImageCreate(NULL, &wi.g);
	gwinImageOpenMemory(ghbConsole, Console);
	gwinImageCache(ghbConsole);
	*/
}

static WORKING_AREA(waThread1, 1024);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }
  return 0;
}

void startBlinker(void){
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  ADC2status = 0;

  //adcStart(&ADCD1, NULL);
  //adcSTM32EnableTSVREFE();
  //palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);

  /* initialize and clear the display */
  gfxInit();
  //mouse = ginputGetMouse(0);
  //get screen size
  width = gdispGetWidth();
  height = gdispGetHeight();
  swidth = gdispGetWidth();
  sheight = gdispGetHeight();
  font = gdispOpenFont("UI2");

  startBlinker();

  myADCinit();
  ICUinit();
  /*
   * Activates the serial driver 1 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state, when the button is
   * pressed the test procedure is launched with output on the serial
   * driver 2.
   */

  ginputSetMouseCalibrationRoutines(0, mysave, myload, FALSE);
  //mouse = ginputGetMouse(0);

  font = gdispOpenFont("UI2");
  gwinSetDefaultFont(font);
  gwinSetDefaultBgColor(Black);
  gwinSetDefaultColor(White);
  bHeight = gdispGetFontMetric(font, fontHeight)+2;

#if BOOTSCREEN
  bootScreen();
#endif

#ifdef LOGO
  gdispClear(Black);
  CreateLogo();
#endif

  // create main screen
  createWidgets();

  chprintf( (BaseSequentialStream *)&SD2, "Main loop\r\n", NULL );

  //geventListenerInit(&gl);
  //gwinAttachListener(&gl);

  while (TRUE)
  {
	  /*
	  pe = geventEventWait(&gl, TIME_INFINITE);
	  switch(pe->type) {
	  case GEVENT_GWIN_BUTTON:
	  	  		   	   if (((GEventGWinButton*)pe)->button == ghConsole)
	  	  		   	   	   {
	  	  		   		   	   gwinSetText(ghStatus2, "Console", TRUE);
	  	  		   		   	   	   CreateConsole();
	  	  		   		   	   chprintf( (BaseSequentialStream *)&SD2, "Create console\r\n", NULL );
	  	  		   	   	   }; break;
	  default:
		  break;
	  }
	  */
	  /*
		pem = (GEventMouse *)geventEventWait(&gl, TIME_INFINITE);
			if (pem->y < sheight && pem->x >= swidth) {
				if ((pem->meta & GMETA_MOUSE_UP)) {
					CreateConsole();
				}
			}
			*/
	  chThdSleepMilliseconds(500);
  }
}

