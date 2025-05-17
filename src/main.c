#include "ch32fun.h"
#include "dram.h"
#include <stdio.h>

// Timer for DRAM refresh
volatile uint32_t refresh_counter = 0;

// Timer interrupt handler for DRAM refresh
void TIM1_UP_IRQHandler(void) __attribute__((interrupt));
void TIM1_UP_IRQHandler(void) {
    // Clear the interrupt flag
    TIM1->INTFR = 0;
    
    // Increment counter
    refresh_counter++;
}

// Setup timer for periodic DRAM refresh
void setup_refresh_timer(void) {
    // Enable TIM1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;
    
    // Configure Timer 1 for periodic interrupts at ~1ms intervals
    // System clock is 48 MHz, prescaler divides by 48000 to get 1kHz
    // Auto-reload value of 1 gives 1ms period
    TIM1->PSC = 48000 - 1;      // 48 MHz / 48000 = 1 KHz
    TIM1->ATRLR = 1;           // Auto reload value
    
    // Enable update interrupt
   // TIM1->DMAINTENR |= TIM_INTENA_CC1IE;
    TIM1->DMAINTENR |= TIM_CC1IE;
    
    // Enable TIM1 update interrupt in NVIC
    NVIC_EnableIRQ(TIM1_UP_IRQn);
    
    // Enable counter
    TIM1->CTLR1 |= TIM_CEN;
    
    printf("Refresh timer configured\n");
}

// DRAM refresh handling in main loop
void handle_dram_refresh(void) {
    static uint8_t current_row = 0;
    
    // Perform row refresh every 1ms
    if (refresh_counter > 0) {
        refresh_counter = 0;
        
        // Refresh a batch of rows (4 rows every ms = all 256 rows in ~64ms, well within 4ms per row requirement)
        for (uint8_t i = 0; i < 4; i++) {
            dram_refresh_row(current_row++);
        }
    }
}

// Initialize system
void system_init(void) {
    // Initialize system clock
    SystemInit();
    
    // Configure UART for debug output
    Delay_Ms(100);
    // SetupUART(UART_BRR);
    
    // Enable interrupts
    // __enable_irq();
    
    printf("\r\nCH32V003 DRAM Interface Test\r\n");
}

// Main function
int main(void) {
    // Initialize system components
    system_init();
    
    // Initialize DRAM
    dram_init();

    // Plot the first 16 bits of every page
    printf("Plotting first 16 bits of every page\r\n");
    dram_scan_array();

    // Write test
    printf("Writing 0x55AACAFE to row 0, column 0\r\n");
    dram_write_fpm(0, 0, 0x55aacafe, 32);
    // plot details of the first 8 pages
    printf("Reading first row of DRAM\r\n");
    dram_readpages_fpm(0, 1);
 
    printf("\n\n");
    printf("------------------------------- Test row setting ------------------------------\n");
    for (uint32_t i=0; i<3; i++) {
        printf("\nBefore RAW glitching:\n");
        dram_write_fpm(0x00, 0, 0x55aacafe, 32);
        dram_write_fpm(0x40, 0, 0x55aacafe, 32);
        dram_readpages_fpm(0x00, 1);
        dram_readpages_fpm(0x40, 1);
        dram_set_row(0x00, i);
        dram_set_row(0x40, i);
        printf("After %ld cycles of RAS glitching:\n",i);
        dram_readpages_fpm(0x00, 1);
        dram_readpages_fpm(0x40, 1);
    }


    printf("\n\n");
    printf("------------------------------- Test row copying ------------------------------\n");

    uint8_t srcrows[4] = {0x00, 0x00, 0x40, 0x40};
    uint8_t dstrows[4] = {0x01, 0x40, 0x00, 0x41};

    for (uint32_t i=0; i<4; i++) {
        printf("\nInitializing rows:\n");
        dram_write_fpm(srcrows[i], 0, 0x55aacafe, 32);
        dram_write_fpm(dstrows[i], 0, 0x00ff00ff, 32);
        dram_readpages_fpm(srcrows[i], 1);
        dram_readpages_fpm(dstrows[i], 1);
        dram_copyrow(srcrows[i],dstrows[i]);
        printf("After copying row %d to row %d:\n", srcrows[i], dstrows[i]);
        dram_readpages_fpm(srcrows[i], 1);
        dram_readpages_fpm(dstrows[i], 1);
    }
    
    printf("DRAM test completed\r\n");
    // printf("Entering main loop with continuous DRAM refresh\r\n");
    
    // Main loop - handle DRAM refresh and any application tasks
    while(1) {
        Delay_Ms(100);
    }
    
    return 0;
}