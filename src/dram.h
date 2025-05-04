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
// #define DRAM_RAS_PIN  GPIO_Pin_3  // PD3 for Row Address Strobe
#define DRAM_WR_PIN   GPIO_Pin_4  // PD4 for Write/Read control
#define DRAM_DOUT_PIN GPIO_Pin_5  // PD5 for Data Out

#define DRAM_RAS_PIN  GPIO_Pin_7  // PC7 for Row Address Strobe

void dram_glitch_row(uint8_t row) __attribute__((section(".srodata"))) __attribute__((used));


// Function prototypes
void dram_init(void);
void dram_write(uint8_t row, uint8_t col, uint8_t data);
uint8_t dram_read(uint8_t row, uint8_t col);
void dram_refresh_row(uint8_t row);
void dram_test(void);
void dram_readpages(uint8_t startpage, uint8_t pages);
void dram_exercise_RAS(void);
void dram_glitch_refresh_test(void);
void dram_glitch_read_test(void);


#endif // DRAM_H