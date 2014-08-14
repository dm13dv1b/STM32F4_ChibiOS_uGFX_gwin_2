/*
 * sram.h
 *
 *  Created on: 2014 aug. 14
 *      Author: root
 */

#ifndef SRAM_H_
#define SRAM_H_

#include "ch.h"
#include "hal.h"

int8_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );
int8_t read_from_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );
int8_t write_to_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset );
int8_t read_from_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset );

#endif /* SRAM_H_ */
