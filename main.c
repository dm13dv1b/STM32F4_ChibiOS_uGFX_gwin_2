

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
#include "shell.h"
#include "myADC.h"
#include "gfx.h"
#include "main.h"
#include "sram.h"
#include "aprilia.h"
#include "Aprilia-4.h"
#include "console.h"

/*
 * Globals definition
 */

Thread *shelltp = NULL;

#if LOGO
static GHandle					ghApriliaLogo, ghAprilia;
#endif

#if BOOTSCREEN
static GHandle					ghc;
static GConsoleObject			gc;
#endif

static GEventMouse				*pem;
static GEvent*					pe;
static GListener				gl;
static GHandle					ghStatus1, ghStatus2;
static GHandle					ghConsole;
static GHandle					ghBrightness;
static GHandle					ADClabel, ADClabel2;
static GHandle					ADCvalue, ADCvalue2;
static GHandle					ICU1label, ICU1value, ICU2label, ICU2value;
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

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WA_SIZE(2048)
#define TEST_WA_SIZE    THD_WA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {THD_STATE_NAMES};
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state time\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%.8lx %.8lx %4lu %4lu %9s %lu\r\n",
            (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
            (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
            states[tp->p_state], (uint32_t)tp->p_time);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  Thread *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  //tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriority(),
  //                         TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {
  static uint8_t buf[] =
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: write\r\n");
    return;
  }

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT) {
    chSequentialStreamWrite(&SD2, buf, sizeof buf - 1);
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"test", cmd_test},
  {"write", cmd_write},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD2,
  commands
};

/* ICU variables
 *
 */
icucnt_t last_width, last_period, last_width_2, last_period_2;

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

  last_width = icuGetWidth(icup);
}

static void icuperiodcb(ICUDriver *icup) {

  last_period = icuGetPeriod(icup);
}

static void icuwidthcb2(ICUDriver *icup) {

  last_width_2 = icuGetWidth(icup);
}

static void icuperiodcb2(ICUDriver *icup) {

  last_period_2 = icuGetPeriod(icup);
}

static ICUConfig icucfg = {
  ICU_INPUT_ACTIVE_HIGH,
  100000,                                    /* 10kHz ICU clock frequency.   */
  icuwidthcb,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_1,
  0
};

static ICUConfig icucfg2 = {
  ICU_INPUT_ACTIVE_HIGH,
  100000,                                    /* 10kHz ICU clock frequency.   */
  icuwidthcb2,
  icuperiodcb2,
  NULL,
  ICU_CHANNEL_2,
  0
};

void ICUinit(void)
{
	  icuStart(&ICUD5, &icucfg);
	  icuStart(&ICUD2, &icucfg2);
	  palSetPadMode(GPIOA, 0, PAL_MODE_ALTERNATE(2));
	  palSetPadMode(GPIOB, 3, PAL_MODE_ALTERNATE(2)); //TIM2_CH2
	  icuEnable(&ICUD5);
	  icuEnable(&ICUD2);
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

	  gwinSetText(ADCvalue, Result, TRUE);
	  //prints the averaged value with 4 digits precision
	  //chprintf((BaseSequentialStream *)&SD2, "\r\nMeasured: %U.%04UV", sum/10000, sum%10000);
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
	  //chprintf((BaseSequentialStream *)&SD2, "\r\nTemperature: %d.%2U°C", thisTemp/100,thisTemp%100);

	  sprintf(Result, "%d", (uint8_t)thisTemp/100);
	  sprintf(Result2, ".%iC", (uint8_t)thisTemp%100);
	  strncat(Result, Result2, 6);
	  gwinSetText(ADCvalue2, Result, TRUE);
}

void readICU(void)
{
	uint16_t last_period_float;
	uint16_t last_period_float_2;
	last_period_float = RTT2US(last_period);
	last_period_float_2 = RTT2US(last_period_2);
	sprintf(Result, "%u", last_period_float);
	gwinSetText(ICU1value, Result, TRUE);
	//chprintf((BaseSequentialStream *)&SD2, "\r\nICU1 %d", last_period);
	sprintf(Result, "%u", last_period_float_2);
	gwinSetText(ICU2value, Result, TRUE);
	//chprintf((BaseSequentialStream *)&SD2, "\r\nICU2 %d", last_period_2);
}

static WORKING_AREA(waThread2, 2048);
static msg_t Thread2(void *arg) {

  (void)arg;
  chRegSetThreadName("ADC blinker");
  while (TRUE) {
#if DEBUG_TO_SERIAL
  chprintf( (BaseSequentialStream *)&SD2, "\r\nADC sampling", NULL );
#endif

  palTogglePad(GPIOD, GPIOD_LED4);       /* Orange.  */
  chSysLockFromIsr();
    readADC();
    readVoltage();
    readICU();
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
	gwinPrintf(ghc, "\r\nADC init");
	gwinPrintf(ghc, "\r\nICU init");
	gwinPrintf(ghc, "\r\nGFX init");
	gwinPrintf(ghc, "\r\nThreads created");
#if LOGO
	gwinPrintf(ghc, "\r\nLOGO enabled");
#endif
	gwinPrintf(ghc, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
	gwinPrintf(ghc, "Port Info:    %s\r\n", CH_PORT_INFO);
	gwinPrintf(ghc, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
	chThdSleepMilliseconds(1000);
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

	// create ICU1label
	wi.g.y = bHeight*2;
	wi.g.x = 0;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "ICU1:";

	ICU1label = gwinLabelCreate(NULL, &wi);


	// ICU1vaule
	wi.g.y = bHeight*2;
	wi.g.x = 40;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "Status2";

	ICU1value = gwinLabelCreate(NULL, &wi);

	// create ICU2label

	wi.g.y = bHeight*3;
	wi.g.x = 0;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "ICU2:";

	ICU2label = gwinLabelCreate(NULL, &wi);

	// ICU2vaule
	wi.g.y = bHeight*3;
	wi.g.x = 40;
	wi.g.width = 160;
	wi.g.height = bHeight;
	wi.text = "000";

	ICU2value = gwinLabelCreate(NULL, &wi);

	// create two status label
	//status 1
	wi.g.y = sheight-bHeight;
	wi.g.x = 0;
	wi.g.width = 50;
	wi.g.height = bHeight;
	wi.text = "Status1";

	ghStatus1 = gwinLabelCreate(NULL, &wi);

	// status 2
	wi.g.y = sheight-bHeight;
	wi.g.x = 160;
	wi.g.width = 50;
	wi.g.height = bHeight;
	wi.text = "Status2";

	ghStatus2 = gwinLabelCreate(NULL, &wi);

	// Brightness
	wi.g.y = sheight-(bHeight*2);
	wi.g.x = 0;
	wi.g.width = swidth;
	wi.g.height = bHeight;
	wi.text = "Brightness";

	ghBrightness = gwinSliderCreate(NULL, &wi);
	gwinSliderSetRange(ghBrightness, 0, 100);
	gwinSliderSetPosition(ghBrightness, 50);
	gdispSetBacklight(50);

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
    gwinSetText(ghStatus1, "Running", TRUE);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    gwinSetText(ghStatus1, "        ", TRUE);
    chThdSleepMilliseconds(500);
  }
  return 0;
}

void startBlinker(void)
{
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

  shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);

  /*
   * Shell manager initialization.
   */
  shellInit();

  ADC2status = 0;

  //write_to_backup_sram(0, 0, 0);

  /* initialize and clear the display */
  gfxInit();
  ginputGetMouse(9999);
  gwinAttachMouse(0);
  geventListenerInit(&gl);
  gwinAttachListener(&gl);
  //geventAttachSource(&gl, mouse, GLISTEN_MOUSEDOWNMOVES|GLISTEN_MOUSEMETA);

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

  while (TRUE)
  {
	  //get an event

	  	  pe = geventEventWait(&gl, TIME_INFINITE);

	  	  switch(pe->type) {
	  	  	  case GEVENT_TOUCH:
	  		  		  	  	  	  {
	  		  		  	  	  	  	  pem = (GEventMouse *)pe;
	  		  		  	  	  	  	  if ((pem->meta & GMETA_MOUSE_CLICK)) {
	  		  		  	  	  					//gwinSetColor(ghc, Yellow);
	  		  		  	  	  					//chprintf((BaseSequentialStream *)&SD2, "\r\n-touch-click");
	  		  		  	  	  				}
	  		  		  	  	  	  }


	  	  	  case GEVENT_GWIN_BUTTON:
	  	  		   	   if (((GEventGWinButton*)pe)->button == ghConsole)
	  	  		   	   	   {
	  	  		   		   	   gwinSetText(ghStatus2, "Console", TRUE);
	  	  		   		   	   //chprintf( (BaseSequentialStream *)&SD2, "\r\nConsole button", NULL );
	  	  		   	   	   };
	  	  		   	   break;

	  	  	  case GEVENT_GWIN_SLIDER:
	  	  		  	  if (((GEventGWinSlider*)pe)->slider == ghBrightness)
	  	  		  	  {
	  	  	  	  	  	  gdispSetBacklight(((GEventGWinSlider *)pe)->position);
	  	  		  	  	  //chprintf((BaseSequentialStream *)&SD2,"Slider %s = %d\r\n", gwinGetText(((GEventGWinSlider *)pe)->slider),
	  	  		  	  	                                                           //((GEventGWinSlider *)pe)->position);
	  	  		  	  }
	  	  		  	   break;
	  		  default:
	  				   break ;
	  	  }
  }
}

