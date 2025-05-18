#include "ch32fun.h"
#include <stdio.h>
#include <stdint.h>

// Include the test instruction definitions
// #include "test_instructions.h"

// #define TEST_INSTR "nop\nnop\nnop\nnop"
// #define TEST_INSTR "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop"
// #define TEST_INSTR "nop\nnop"
// #define TEST_INSTR "nop\nnop\nnop"
// #define TEST_INSTR "nop"
// #define TEST_INSTR ""
// 32 bit NOP instruction
// #define TEST_INSTR "mv x0,x0"
#define TEST_INSTR "mv x0,x0\nmv x0,x0"
// #define TEST_INSTR "mv x0,x0\nmv x0,x0\nmv x0,x0\nmv x0,x0\nmv x0,x0\nmv x0,x0\nmv x0,x0\nmv x0,x0"
// store
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])"
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])"
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])"
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])"
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])\nsw %[val_to_store], 0(%[addr_reg])"

// IO access
// #define TEST_INSTR "sw %[val_to_store], 0(%[addr_reg])"

// Helper macro to calculate elapsed cycles with wraparound handling
#define CALC_ELAPSED_CYCLES(start, end) ((end >= start) ? (end - start) : ((0xFFFFFFFF - start) + end + 1))

#define TEST_COUNT_3  10000

// ============================================================================
// Pure assembly test functions to eliminate compiler-generated overhead
// ============================================================================

// Flash version of test
__attribute__((noinline))
uint32_t run_test_flash(uint32_t count) {
    uint32_t start, end;
    volatile uint32_t value_to_store = 0x00010000; // Value to store (e.g., reset PD0)
    volatile uint32_t *ptr_to_target = (volatile uint32_t *)&GPIOD->BCR;

    // Get start time directly from SysTick->CNT
    start = SysTick->CNT;
    
    asm volatile (
        ".balign 4\n"                    // Align to 4-byte boundary
        // "nop\n"
        // Loop with count iterations
        "1:\n"                           // Loop start label
        TEST_INSTR "\n"                  // The instruction(s) being tested
        "addi %[count], %[count], -1\n"  // Decrement counter
        "bnez %[count], 1b\n"            // Branch to 1b if count is not zero
        
        : [count] "+r" (count)
        : [val_to_store] "r" (value_to_store), [addr_reg] "r" (ptr_to_target)
        : "memory"
    );
    
    // Get end time directly from SysTick->CNT
    end = SysTick->CNT;
    
    return CALC_ELAPSED_CYCLES(start, end);
}

// SRAM version of test (placed in SRAM)
__attribute__((section(".srodata"))) __attribute__((used)) __attribute__((noinline))
uint32_t run_test_sram(uint32_t count) {
    uint32_t start, end;
    volatile uint32_t value_to_store = 0x00010000; // Value to store (e.g., reset PD0)
    volatile uint32_t *ptr_to_target = (volatile uint32_t *)&GPIOD->BCR;

    // Get start time directly from SysTick->CNT
    start = SysTick->CNT;
    
    asm volatile (
        ".balign 4\n"                    // Align to 4-byte boundary
        // "nop\n"
        // Loop with count iterations
        "1:\n"                           // Loop start label
        TEST_INSTR "\n"                  // The instruction(s) being tested
        "addi %[count], %[count], -1\n"  // Decrement counter
        "bnez %[count], 1b\n"            // Branch to 1b if count is not zero
        
        : [count] "+r" (count)
        : [val_to_store] "r" (value_to_store), [addr_reg] "r" (ptr_to_target)
        : "memory"
    );
    
    // Get end time directly from SysTick->CNT
    end = SysTick->CNT;
    
    return CALC_ELAPSED_CYCLES(start, end);
}

// Run timing tests for both Flash and SRAM
void run_instruction_timing_tests(void) {
    uint32_t cycles_flash_3;
    uint32_t cycles_sram_3;
    const int32_t SCALE_FACTOR = 1000; // For 3 decimal places
    
    printf("\n--- Instruction Timing Tests ---\n");
    printf("System Clock: %d MHz\n", FUNCONF_SYSTEM_CORE_CLOCK / 1000000);
    printf("Testing instruction: %s\n\n", TEST_INSTR);
    printf("Flash Wait States: %ld\n", FLASH->ACTLR & FLASH_ACTLR_LATENCY);
    
    printf("Running Flash tests...\n");
           
    cycles_flash_3 = run_test_flash(TEST_COUNT_3);
    int32_t cf3_scaled = (int32_t)(((int64_t)cycles_flash_3 * SCALE_FACTOR) / TEST_COUNT_3);
    printf("Flash test with %d iterations: %lu cycles (%s%ld.%03ld cycles/iteration)\n", 
           TEST_COUNT_3, cycles_flash_3, (cf3_scaled < 0 && (cf3_scaled/SCALE_FACTOR)==0)?"-":"", cf3_scaled/SCALE_FACTOR, (cf3_scaled%SCALE_FACTOR)<0?-(cf3_scaled%SCALE_FACTOR):(cf3_scaled%SCALE_FACTOR));
    
    printf("\nRunning SRAM tests...\n");   
          
    cycles_sram_3 = run_test_sram(TEST_COUNT_3);
    int32_t cs3_scaled = (int32_t)(((int64_t)cycles_sram_3 * SCALE_FACTOR) / TEST_COUNT_3);
    printf("SRAM test with %d iterations: %lu cycles (%s%ld.%03ld cycles/iteration)\n", 
           TEST_COUNT_3, cycles_sram_3, (cs3_scaled < 0 && (cs3_scaled/SCALE_FACTOR)==0)?"-":"", cs3_scaled/SCALE_FACTOR, (cs3_scaled%SCALE_FACTOR)<0?-(cs3_scaled%SCALE_FACTOR):(cs3_scaled%SCALE_FACTOR));
    
}

int main() {
    SystemInit();
    
    printf("\nCH32V003 Instruction Timing Test\n");
    printf("================================\n");
    
    run_instruction_timing_tests();
    
    while (1) {
        Delay_Ms(1000);
        printf(".");
    }
    
    return 0;
}