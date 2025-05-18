#ifndef TEST_INSTRUCTIONS_H
#define TEST_INSTRUCTIONS_H

/* 
 * This file contains different instruction test definitions.
 * To test a different instruction or sequence, uncomment one of
 * these definitions and recompile.
 */

// Basic instruction tests
// #define TEST_INSTR "nop"                         // No operation
// #define TEST_INSTR "addi t0, t0, 1"             // Register-immediate addition
// #define TEST_INSTR "add t0, t0, t1"              // Register-register addition
// #define TEST_INSTR "sub t0, t0, t1"              // Register-register subtraction
// #define TEST_INSTR "and t0, t0, t1"              // Bitwise AND
// #define TEST_INSTR "or t0, t0, t1"               // Bitwise OR
// #define TEST_INSTR "xor t0, t0, t1"              // Bitwise XOR
// #define TEST_INSTR "slli t0, t0, 1"              // Shift Left Logical Immediate
// #define TEST_INSTR "srli t0, t0, 1"              // Shift Right Logical Immediate
// #define TEST_INSTR "srai t0, t0, 1"              // Shift Right Arithmetic Immediate

// Memory operation tests
// #define TEST_INSTR "lw t0, 0(t1)"                // Load Word
// #define TEST_INSTR "sw t0, 0(t1)"                // Store Word
// #define TEST_INSTR "lb t0, 0(t1)"                // Load Byte
// #define TEST_INSTR "lbu t0, 0(t1)"               // Load Byte Unsigned
// #define TEST_INSTR "sb t0, 0(t1)"                // Store Byte
// #define TEST_INSTR "lh t0, 0(t1)"                // Load Halfword
// #define TEST_INSTR "lhu t0, 0(t1)"               // Load Halfword Unsigned
// #define TEST_INSTR "sh t0, 0(t1)"                // Store Halfword

// Control flow tests
// #define TEST_INSTR "j 1f\n1:"                    // Jump
// #define TEST_INSTR "beq t0, t1, 1f\n1:"          // Branch if equal
// #define TEST_INSTR "bne t0, t1, 1f\n1:"          // Branch if not equal
// #define TEST_INSTR "blt t0, t1, 1f\n1:"          // Branch if less than
// #define TEST_INSTR "bge t0, t1, 1f\n1:"          // Branch if greater or equal

// Instruction sequences for more complex operations
// #define TEST_INSTR "add t0, t1, t2\nadd t3, t4, t5"  // Two additions
// #define TEST_INSTR "lw t0, 0(t1)\nsw t0, 0(t2)"      // Load and store sequence
// #define TEST_INSTR "mul t0, t1, t2"                   // Multiplication (if supported)

// Default test instruction (if none above is selected)
#ifndef TEST_INSTR
#define TEST_INSTR "nop"
#endif

#endif /* TEST_INSTRUCTIONS_H */