#include "dram.h"
#include <stdio.h>

// Compile-time delay macros for exact cycle counts without loop overhead
#define DELAY_1_CYCLES() __asm volatile ("nop")
#define DELAY_2_CYCLES() __asm volatile ("nop\nnop")
#define DELAY_3_CYCLES() __asm volatile ("nop\nnop\nnop")
#define DELAY_4_CYCLES() __asm volatile ("nop\nnop\nnop\nnop")
#define DELAY_5_CYCLES() __asm volatile ("nop\nnop\nnop\nnop\nnop")
#define DELAY_10_CYCLES() __asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop")

// Specific delay macros for each timing parameter
#define DELAY_RAS_CYCLES() DELAY_5_CYCLES() // ~100ns
#define DELAY_CAS_CYCLES() DELAY_5_CYCLES() // ~100ns
// #define DELAY_CAS_CYCLES() DELAY_10_CYCLES() // ~100ns
#define DELAY_RCD_CYCLES() DELAY_2_CYCLES()  // ~20ns
#define DELAY_RP_CYCLES()  DELAY_5_CYCLES() // ~100ns

// Initialize DRAM interface
void dram_init(void) {
    // Enable GPIO port clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
    
    // Configure PC0-PC7 as outputs for address bus
    GPIOC->CFGLR &= ~0xFFFFFFFF; // Clear all PC0-PC7 config
    GPIOC->CFGLR |= 0x33333333;  // Set PC0-PC7 as push-pull outputs, 50MHz
    
    // Configure PD pins
    // PD0 (DIN) as output
    GPIOD->CFGLR &= ~(0xF << (4 * 0));
    GPIOD->CFGLR |= (0x3 << (4 * 0)); // Push-pull output, 50MHz
    
    // PD2 (CAS) as output
    GPIOD->CFGLR &= ~(0xF << (4 * 2));
    GPIOD->CFGLR |= (0x3 << (4 * 2)); // Push-pull output, 50MHz
    
    // PD3 (RAS) as output
    GPIOD->CFGLR &= ~(0xF << (4 * 3));
    GPIOD->CFGLR |= (0x3 << (4 * 3)); // Push-pull output, 50MHz
    
    // PD4 (W/R) as output
    GPIOD->CFGLR &= ~(0xF << (4 * 4));
    GPIOD->CFGLR |= (0x3 << (4 * 4)); // Push-pull output, 50MHz
    
    // PD5 (DOUT) as input
    GPIOD->CFGLR &= ~(0xF << (4 * 5));
    GPIOD->CFGLR |= (0x4 << (4 * 5)); // Input with pull-up/down
    
    // Set initial pin states
    GPIOD->BSHR = DRAM_CAS_PIN | DRAM_RAS_PIN; // Set CAS and RAS high (inactive)
    GPIOD->BSHR = DRAM_WR_PIN;                 // Set W/R high (read mode)
    
    printf("DRAM pins configured\r\n");
    
}

// Write a bit to DRAM
void dram_write(uint8_t row, uint8_t col, uint8_t data) {
    // Set write mode
    GPIOD->BCR = DRAM_WR_PIN;  // W/R low (write mode)
    
    // Set data bit
    if (data) {
        GPIOD->BSHR = DRAM_DIN_PIN; // Set DIN high for 1
    } else {
        GPIOD->BCR = DRAM_DIN_PIN;  // Set DIN low for 0
    }
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row;
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    DELAY_RCD_CYCLES();        // RAS to CAS delay
    
    // Set column address
    DRAM_ADDR_PORT->OUTDR = col;
    GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
    DELAY_CAS_CYCLES();        // CAS pulse width
    
    // End cycle
    GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    DELAY_RP_CYCLES();         // RAS precharge time
}

void dram_write_page(uint8_t row, uint32_t pattern) {
    // Write a full page (256 bits) to DRAM
    for (uint16_t col = 0; col < 256; col++) {
        dram_write(row, (uint8_t)col, (pattern>>(col&31))&1); // Write each bit
    }
}


// Read a bit from DRAM
uint8_t dram_read(uint8_t row, uint8_t col) {
    uint8_t data;
    
    // Ensure read mode
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row;
    DELAY_2_CYCLES(); // Delay for address setup time    
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    
    DELAY_RCD_CYCLES();        // RAS to CAS delay
 
    // GPIOD->BSHR = DRAM_DOUT_PIN;  // W/R high (read mode)
   
    // Set column address
    DRAM_ADDR_PORT->OUTDR = col;
    GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
    DELAY_CAS_CYCLES();        // CAS pulse width
    
    // Read data bit
    data = (GPIOD->INDR & DRAM_DOUT_PIN) ? 1 : 0;
    
    // End cycle
    GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    DELAY_RP_CYCLES();         // RAS precharge time
    
    return data;
}


void dram_copyrow(uint8_t, uint8_t,uint8_t) __attribute__((section(".srodata"))) __attribute__((used));

// Read a bit from DRAM
void dram_copyrow(uint8_t row1, uint8_t row2, uint8_t delay) {    
    // Ensure read mode
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row1;
    DELAY_2_CYCLES(); // Delay for address setup time    
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)  

    // set second row address
    // DELAY_RCD_CYCLES();        // RAS to CAS delay
    DRAM_ADDR_PORT->OUTDR = row2;
    // GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)

    // DELAY_2_CYCLES(); // Delay for address setup time

    if (delay==0) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        // DELAY_3_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    
    
    } else if (delay==1) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        DELAY_1_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    } else if (delay==2) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        DELAY_2_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    } else if (delay==3) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        DELAY_3_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    } else if (delay==4) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        DELAY_4_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    } else if (delay==5) {
        GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
        // violate RAS precharge time
        DELAY_10_CYCLES(); // Delay for address setup time
        GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    } 

    
    // GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    // // violate RAS precharge time
    // DELAY_3_CYCLES(); // Delay for address setup time
    // GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    

    DELAY_CAS_CYCLES();        // CAS pulse width
        
    // End cycle
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    DELAY_RP_CYCLES();         // RAS precharge time
}

void dram_copyrow_cas(uint8_t, uint8_t,uint8_t) __attribute__((section(".srodata"))) __attribute__((used));
// Write a bit to DRAM
void dram_copyrow_cas(uint8_t row1, uint8_t row2, uint8_t delay) {
    // Set write mode
    GPIOD->BCR = DRAM_WR_PIN;  // W/R low (write mode)
    
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row1;
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    DELAY_RCD_CYCLES();        // RAS to CAS delay
    
    // Set column address
    DRAM_ADDR_PORT->OUTDR = 4;
    GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
    DELAY_CAS_CYCLES();        // CAS pulse width
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
    DELAY_1_CYCLES(); // Delay for address setup time
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)
    GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
    
    
    // End cycle
    GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    DELAY_RP_CYCLES();         // RAS precharge time
}


void dram_refresh_row(uint8_t row) __attribute__((section(".srodata"))) __attribute__((used));

// Refresh a single row
void dram_refresh_row(uint8_t row) {
    // RAS-only refresh cycle
    DRAM_ADDR_PORT->OUTDR = row;  // Set row address
    GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
    DELAY_RAS_CYCLES();          // RAS pulse width    
    // DELAY_RAS_CYCLES();          // RAS pulse width
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    DELAY_RP_CYCLES();           // RAS precharge time
}


void dram_glitch_row(uint8_t row) __attribute__((section(".srodata"))) __attribute__((used));

// Refresh a single row
void dram_glitch_row(uint8_t row) {
    // RAS-only refresh cycle
    DRAM_ADDR_PORT->OUTDR = row;  // Set row address
    GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
    DELAY_RAS_CYCLES();          // RAS pulse width    
    DELAY_RAS_CYCLES();          // RAS pulse width
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
    DELAY_RAS_CYCLES();          // RAS pulse width    
    DELAY_RAS_CYCLES();          // RAS pulse width
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    DELAY_RP_CYCLES();           // RAS precharge time
    DELAY_RP_CYCLES();           // RAS precharge time
    DELAY_RP_CYCLES();           // RAS precharge time
    GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
}

void dram_exercise_RAS(void) {
    // Test RAS refresh functionality
    printf("Starting DRAM test: RAS refresh\r\n");

    dram_write_page(0, 0x00000000); // Write a pattern to each page (row number)
    dram_write_page(2, 0xffffffff); // Write a pattern to each page (row number)

    while(1) {
        dram_refresh_row(0); // Refresh row 0
        dram_glitch_row(0); // Refresh row 2

        Delay_Us(10);
    }
}

void dram_glitch_refresh(uint8_t row,int32_t reps) __attribute__((section(".srodata"))) __attribute__((used));

// Refresh a single row
void dram_glitch_refresh(uint8_t row,int32_t reps) {
    // RAS-only refresh cycle
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    GPIOD->BSHR = DRAM_CAS_PIN;   // CAS high (inactive)
    DRAM_ADDR_PORT->OUTDR = row;  // Set row address

    for (int32_t i=0; i<reps; i++) {
        GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
        // DELAY_3_CYCLES();          // RAS pulse width
        GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
        DELAY_RP_CYCLES();           // RAS precharge time
    }
}

void dram_glitch_read(uint8_t row1, uint8_t row2, int32_t reps) __attribute__((section(".srodata"))) __attribute__((used));

// Refresh a single row
void dram_glitch_read(uint8_t row1, uint8_t row2,int32_t reps) {
    // RAS-only refresh cycle
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    GPIOD->BSHR = DRAM_CAS_PIN;   // CAS high (inactive)
    DELAY_RP_CYCLES();           // RAS precharge time

 
    for (int32_t i=0; i<reps; i++) {
        DRAM_ADDR_PORT->OUTDR = row1;  // Set row1 address
        DELAY_RP_CYCLES();           // RAS precharge time
        GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
        DELAY_3_CYCLES();          // RAS pulse width
        DRAM_ADDR_PORT->OUTDR = row2;  // Set row2 address
        DELAY_3_CYCLES();          // RAS pulse width
        GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
        // DELAY_1_CYCLES();          // RAS pulse width
        GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
        DELAY_RAS_CYCLES();          // RAS pulse width    
        GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
        DELAY_RP_CYCLES();           // RAS precharge time
    }
}


// copies row by latching new row address before the refresh cycle is terminated
void dram_glitch_read_test(void) {
    // Test RAS refresh functionality
    uint8_t row1 = 64,row2 = 65;

    printf("Starting DRAM test: read glitch\r\n");

    dram_readpages(row1,1);
    dram_readpages(row2,1);

    for (int32_t i=0; i<4; i++) {
        printf("cycle: %ld\n",i);
        dram_write_page(row1, 0x5A5A5A5A); // Write a pattern to each page (row number)
        dram_write_page(row2, 0x00FF00FF); // Write a pattern to each page (row number)
        dram_glitch_read(row1,row2,i);
        dram_readpages(row1,1);
        dram_readpages(row2,1);
    }
}



void dram_glitch_refresh_test(void) {
    // Test RAS refresh functionality
    uint8_t row1 = 0,row2 = 64 ;
    printf("Starting DRAM test: refresh glitch\r\n");
    dram_readpages(row1,1);
    dram_readpages(row2,1);

    for (int32_t i=0; i<8; i++) {
        printf("cycle: %ld\n",i);
        dram_write_page(row1, 0x0F550F55); // Write a pattern to each page (row number)
        dram_write_page(row2, 0x0F550F55); // Write a pattern to each page (row number)
        dram_glitch_refresh(row1,i);        
        dram_glitch_refresh(row2,i);        
        dram_readpages(row1,1);
        dram_readpages(row2,1);
    }

}



void dram_test(void) {
    // Test writing and reading from DRAM
    printf("Starting DRAM test: Writing and reading from first 16 pages (Rows 0-15)\r\n");

    dram_readpages(0,16);

    for (uint8_t delay=0; delay < 6; delay++) {
        for (uint8_t row = 0; row <16; row+=4) {
            dram_write_page(row, 0x55aaff00); // Write a pattern to each page (row number)
            dram_write_page(row+1, 0x55aaff00); // Write a pattern to each page (row number)
            dram_write_page(row+2, 0x00FF00FF); // Write a pattern to each page (row number)
            dram_write_page(row+3, 0x00FF00FF); // Write a pattern to each page (row number)
        }
        // printf("Testpatterns written to DRAM\r\n");
        // dram_readpages(4);    
        dram_copyrow(1,3,delay); // Copy row 0 to row 2
        // dram_copyrow_cas(2,3,delay); // Copy row 0 to row 2
        printf("Copying row 1 to row 3. Delay: %d\r\n", delay);

        dram_readpages(0,4);

    }

}

// Test DRAM functionality
void dram_readpages(uint8_t startrow,uint8_t rows) {
    uint8_t read_val;
    uint8_t page_buffer[32]; // Buffer to hold 256 bits (32 bytes)
    uint8_t current_byte;
    uint8_t bit_index;

    printf("Starting DRAM test: Reading first 16 pages (Rows 0-15)\r\n");

    // Loop through the first 16 rows
    for (uint8_t row = startrow; row < startrow+rows; row++) {
        current_byte = 0;
        bit_index = 0;

        // Read the current page (Row 'row', Columns 0-255)
        for (uint16_t col = 0; col < 256; col++) {
            read_val = dram_read(row, (uint8_t)col); // Read bit from current row, current column

            // Assemble bits into bytes
            if (read_val) {
                current_byte |= (1 << bit_index);
            }

            bit_index++;
            if (bit_index == 8) {
                page_buffer[col / 8] = current_byte;
                current_byte = 0;
                bit_index = 0;
            }
        }

        // Print the hex dump of the current page on a single line
        printf("Row %02X: ", row); // Print row number at the start
        for (uint8_t i = 0; i < 32; i++) {
            printf("%02X ", page_buffer[i]);
        }
        printf("\r\n"); // Add a single newline after printing all 32 bytes
    }

}