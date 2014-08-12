/*
 * myFunc.c
 *
 *  Created on: 2014 aug. 8
 *      Author: root
 */

#include "ch.h"
#include "hal.h"
#include "gfx.h"
#include "myFunc.h"

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */

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

