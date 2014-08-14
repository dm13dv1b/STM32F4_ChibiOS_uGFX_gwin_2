/*
 * sram.c
 *
 *  Created on: 2014 aug. 14
 *      Author: root
 */

#include "sram.h"
#include "ch.h"
#include "hal.h"
#include <stdint.h>

int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 0x1000;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }

  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  /* disable backup domain write protection */
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);   // set RCC->APB1ENR.pwren
  //PWR_BackupAccessCmd(ENABLE);                          // set PWR->CR.dbp = 1;
  /** enable the backup regulator (used to maintain the backup SRAM content in
    * standby and Vbat modes).  NOTE : this bit is not reset when the device
    * wakes up from standby, system reset or power reset. You can check that
    * the backup regulator is ready on PWR->CSR.brr, see rm p144 */
  //PWR_BackupRegulatorCmd(ENABLE);     // set PWR->CSR.bre = 1;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  //PWR_BackupAccessCmd(DISABLE);                     // reset PWR->CR.dbp = 0;
  return 0;
}

int8_t read_from_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 0x1000;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return 0;
}

int8_t write_to_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 80;
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* disable backup domain write protection */
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);   // set RCC->APB1ENR.pwren
  //PWR_BackupAccessCmd(ENABLE);                          // set PWR->CR.dbp = 1;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  //PWR_BackupAccessCmd(DISABLE);                     // reset PWR->CR.dbp = 0;
  // consider also disabling the power peripherial?
  return 0;
}

int8_t read_from_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 80;
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* read should be 32 bit aligned */
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return 0;
}
