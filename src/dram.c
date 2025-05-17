#include "dram.h"
#include <stdio.h>

// Compile-time delay macros for exact cycle counts without loop overhead
#define DELAY_1_CYCLES() __asm volatile ("nop")
#define DELAY_2_CYCLES() __asm volatile ("nop\nnop")
#define DELAY_3_CYCLES() __asm volatile ("nop\nnop\nnop")
#define DELAY_4_CYCLES() __asm volatile ("nop\nnop\nnop\nnop")
#define DELAY_5_CYCLES() __asm volatile ("nop\nnop\nnop\nnop\nnop")

// Specific delay macros for each timing parameter
#define DELAY_RAS_CYCLES() DELAY_5_CYCLES() // ~100ns
#define DELAY_CAS_CYCLES() DELAY_5_CYCLES() // ~100ns
#define DELAY_RCD_CYCLES() DELAY_2_CYCLES()  // ~20ns
#define DELAY_RP_CYCLES()  DELAY_5_CYCLES() // ~100ns

// Initialize DRAM interface
void dram_init(void) {
    
    // Enable GPIO port clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
 
    // Set initial pin states
    GPIOD->BSHR = DRAM_CAS_PIN | DRAM_RAS_PIN; // Set CAS and RAS high (inactive)
    GPIOD->BSHR = DRAM_WR_PIN;                 // Set W/R high (read mode)

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
    
    printf("DRAM pins configured\r\n");
    
}

// Refresh a single row
void dram_refresh_row(uint8_t row) {
    // RAS-only refresh cycle
    DRAM_ADDR_PORT->OUTDR = row;  // Set row address
    GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
    DELAY_RAS_CYCLES();          // RAS pulse width    
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    DELAY_RP_CYCLES();           // RAS precharge time
}

// Read a bit from DRAM
uint8_t dram_read_bit(uint8_t row, uint8_t col) {
    uint8_t data;
    
    // Ensure read mode
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row;
    DELAY_2_CYCLES(); // Delay for address setup time    
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    
    DELAY_RCD_CYCLES();        // RAS to CAS delay
   
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

// Read a int32 value from DRAM using fast page mode
uint32_t dram_read_fpm(uint8_t row, uint8_t col, uint8_t bits) {
    uint32_t data=0;
    uint32_t bitcount=0;

    if (bits>32) {
        bits=32;
    }   
    
    // Ensure read mode
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row;
    DELAY_2_CYCLES(); // Delay for address setup time    
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    
    DELAY_RCD_CYCLES();        // RAS to CAS delay
   
    for (bitcount=0; bitcount<bits; bitcount++) {
        // Set column address
        DRAM_ADDR_PORT->OUTDR = col + bitcount;
        GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
        DELAY_CAS_CYCLES();        // CAS pulse width
        
        // Read data bit
        if (GPIOD->INDR & DRAM_DOUT_PIN) {
            data |= (1<<bitcount);
        }
        
        // End cycle
        GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
        DELAY_CAS_CYCLES();        // CAS pulse width
    }

    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    DELAY_RP_CYCLES();         // RAS precharge time
    
    return data;
}

// Write a bit to DRAM
void dram_write_bit(uint8_t row, uint8_t col, uint8_t data) {
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

// write a int32 value from DRAM using fast page mode
void dram_write_fpm(uint8_t row, uint8_t col_start, uint32_t data_val, uint8_t bits) {
    uint32_t bitcount = 0;

    if (bits == 0) {
        return; // Nothing to write
    }
    if (bits > 32) {
        bits = 32;
    }

    // Set Write Mode
    GPIOD->BCR = DRAM_WR_PIN;  // W/R low (write mode)

    // Activate Row
    DRAM_ADDR_PORT->OUTDR = row; // Set row address
    DELAY_2_CYCLES();            // Delay for address setup time (consistent with dram_read_fpm)
    GPIOD->BCR = DRAM_RAS_PIN;   // RAS low (active)
    DELAY_RCD_CYCLES();          // RAS to CAS delay

    // Write Data (multiple columns)
    for (bitcount = 0; bitcount < bits; bitcount++) {
        // Set column address
        DRAM_ADDR_PORT->OUTDR = col_start + bitcount;

        // Set data bit on DIN pin
        if ((data_val >> bitcount) & 1) {
            GPIOD->BSHR = DRAM_DIN_PIN; // Set DIN high for 1
        } else {
            GPIOD->BCR = DRAM_DIN_PIN;  // Set DIN low for 0
        }
        // Small delay for data setup before CAS (t_DS), if necessary, can be added here.
        // Current implementation assumes instruction timing is sufficient.

        GPIOD->BCR = DRAM_CAS_PIN;  // CAS low (active)
        DELAY_CAS_CYCLES();         // CAS pulse width (t_CAS or t_WP - Write Pulse Width)

        GPIOD->BSHR = DRAM_CAS_PIN; // CAS high (inactive)
        // Delay for CAS high time / CAS cycle time in FPM (t_CH or t_CP)
        DELAY_CAS_CYCLES();
    }

    // Deactivate Row and End Cycle
    // W/R should go high before RAS goes high to properly terminate the write cycle.
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode) - W goes high
    // A small delay might be needed here for tWR (Write Recovery time) if specified by DRAM datasheet,
    // ensuring W is high for a certain duration before RAS goes high.
    // For now, assuming direct transition is acceptable or covered by subsequent delays.
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    DELAY_RP_CYCLES();          // RAS precharge time
}

// Read and diplay rows from the DRAM using fast page mode
void dram_readpages_fpm(uint8_t startrow, uint8_t rows) {
    uint32_t read_val;

    for (uint8_t row = startrow; row < startrow+rows; row++) {
        // Print the row number at the start
        printf("Row %02X: ", row);
        
        // Read and print 8 32-bit values from the row (256 bits total)
        for (uint8_t col = 0; col < 8; col++) {
            read_val = dram_read_fpm(row, col * 32, 32); // Reading 32 bits starting at columns 0, 32, 64, ...
            printf("%08lX ", read_val);
        }
        
        printf("\r\n"); // Add a newline after printing the row
    }
}

// Prints the first 16 bits of every pages in the dram
void dram_scan_array() {
    uint32_t read_val;
    uint8_t dram_row;

    // Print header row
    printf("    "); 
    for (uint8_t col_header = 0; col_header < 16; col_header++) {
        printf("  %X  ", col_header);
    }
    printf("\r\n");

    for (uint8_t grid_row = 0; grid_row < 16; grid_row++) { // Iterate 16 times for grid rows (0-F)
        printf("%X0: ", grid_row); // Print grid row header (e.g., 0_:, 1_:, ..., F_:)
        for (uint8_t grid_col = 0; grid_col < 16; grid_col++) { // Iterate 16 times for grid columns (0-F)
            dram_row = (grid_row << 4) | grid_col; // Calculate actual DRAM row (0-255)
            read_val = dram_read_fpm(dram_row, 0, 16); // Read first 16 bits from column 0
            printf("%04lX ", read_val & 0xFFFF); // Print 16-bit value (4 hex chars)
        }
        printf("\r\n"); 
    }
    // printf("DRAM Scan Complete.\r\n");
}

// Test DRAM functionality
void dram_readpages(uint8_t startrow,uint8_t rows) {
    uint8_t read_val;
    uint8_t page_buffer[32]; // Buffer to hold 256 bits (32 bytes)
    uint8_t current_byte;
    uint8_t bit_index;

    for (uint8_t row = startrow; row < startrow+rows; row++) {
        current_byte = 0;
        bit_index = 0;

        // Read the current page (Row 'row', Columns 0-255)
        for (uint16_t col = 0; col < 256; col++) {
            read_val = dram_read_bit(row, (uint8_t)col); // Read bit from current row, current column

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

void dram_write_page(uint8_t row, uint32_t pattern) {
    // Write a full page (256 bits) to DRAM
    for (uint16_t col = 0; col < 256; col++) {
        dram_write_bit(row, (uint8_t)col, (pattern>>(col&31))&1); // Write each bit
    }
}

// Refresh a single row
void dram_set_row(uint8_t row,int32_t reps) {
    // RAS-only refresh cycle
    GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
    GPIOD->BSHR = DRAM_CAS_PIN;   // CAS high (inactive)
    DRAM_ADDR_PORT->OUTDR = row;  // Set row address
    DELAY_3_CYCLES();             // Make sure row address is latched

    for (int32_t i=0; i<reps; i++) {
        GPIOD->BCR = DRAM_RAS_PIN;    // RAS low (active)
        GPIOD->BSHR = DRAM_RAS_PIN;   // RAS high (inactive)
        DELAY_RP_CYCLES();            // RAS precharge time -> ensureds bitlines are at VDD/2
    }

    dram_refresh_row(row);             // Refresh the row to ensure stable levels on the cells
}

// Copy a row to another row
void dram_copyrow(uint8_t row1, uint8_t row2) {
    // Ensure read mode
    GPIOD->BSHR = DRAM_WR_PIN;  // W/R high (read mode)
    
    // Set row address
    DRAM_ADDR_PORT->OUTDR = row1;
    DELAY_RP_CYCLES();         // RAS precharge time
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)  
    DELAY_RCD_CYCLES();         // RAS to CAS delay
    DRAM_ADDR_PORT->OUTDR = row2;
    DELAY_2_CYCLES();           // RAS to CAS delay

    // Open row2 while bitlines are still precharged with row1 content
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    // violate RAS precharge time
    GPIOD->BCR = DRAM_RAS_PIN;  // RAS low (active)    
     
    DELAY_RAS_CYCLES();         // CAS pulse width
        
    // End cycle
    GPIOD->BSHR = DRAM_RAS_PIN; // RAS high (inactive)
    DELAY_RP_CYCLES();          // RAS precharge time
}
