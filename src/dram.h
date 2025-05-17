#ifndef DRAM_H
#define DRAM_H

#include "ch32fun.h"

// Pin definitions for 4164 DRAM according to updated README
// Address bus on PORT C
#define DRAM_ADDR_PORT GPIOC
#define DRAM_ADDR_MASK 0xFF  // PC0-PC7 for address bus

// Control pins on PORT D
#define DRAM_DIN_PIN  GPIO_Pin_0  // PD0 for Data In
#define DRAM_CAS_PIN  GPIO_Pin_2  // PD2 for Column Address Strobe
#define DRAM_RAS_PIN  GPIO_Pin_3  // PD3 for Row Address Strobe
#define DRAM_WR_PIN   GPIO_Pin_4  // PD4 for Write/Read control
#define DRAM_DOUT_PIN GPIO_Pin_5  // PD5 for Data Out

// Function prototypes
void dram_init(void);

void dram_write_bit(uint8_t row, uint8_t col, uint8_t data);
uint8_t dram_read_bit(uint8_t row, uint8_t col);

void dram_write_fpm(uint8_t row, uint8_t col_start, uint32_t data_val, uint8_t bits);
uint32_t dram_read_fpm(uint8_t row, uint8_t col, uint8_t bits);

void dram_readpages(uint8_t startpage, uint8_t pages);
void dram_readpages_fpm(uint8_t startrow,uint8_t rows);

void dram_scan_array();

void dram_refresh_row(uint8_t row);

void dram_set_row(uint8_t row,int32_t reps);

void dram_copyrow(uint8_t row1, uint8_t row2);

#endif // DRAM_H