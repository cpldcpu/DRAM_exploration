# CH32V003 Instruction Timing Test

This subproject investigates instruction timing on the CH32V003 microcontroller. It measures cycle counts for different instructions when executing from both Flash memory and SRAM.

## What is tested?

This program tests the execution time of RISC-V instructions on the CH32V003 microcontroller. The test instructions are embedded in a loop, and the number of cycles taken to execute them is measured. The results are printed to the console.

Test instructions are defined on top of main.c. Example:

```c
#define TEST_INSTR "add t0, t0, t1"           // Register-register addition  
```

The instruction is then embedded into a simple loop that runs 10000 times. The program measures the time taken to execute this loop in both Flash and SRAM memory.

Example of an instruction test loop with a NOP instruction:

```assembly
20000006:	0001                	nop
20000008:	157d                	addi	a0,a0,-1
2000000a:	fd75                	bnez	a0,20000006 <run_test_sram+0x6>
```	

The CH32V003 is clocked at 48MHz by default, which means that one wait state is needed for Flash memory access. The program tests timing both in Flash and in SRAM, using the `__attribute__((section(".srodata")))` attribute to place the test code in SRAM. 1. The program uses the CH32V003's SysTick counter to measure the number of cycles taken.

## Usage

1. Make sure you are in the `instruction_timing` directory
2. Compile and flash the project using the Makefile:
   ```bash
   make
   ```

3. Call 'make monitor' to see the output in the terminal:
   ```bash
   make monitor
   ```
# Results

## 16 Bit NOP

The plot below shows the instruction timing of the loop vs. number of inserted 16 bit NOP instructions. 

<p align="center">
   <img src="16bitnop.png" alt="16 bit nop" width="70%">
</p>

## 32 Bit NOP

The plot below shows the instruction timing of the loop vs. number of inserted 32 bit NOP instructions. The 32 bit NOP instruction is defined as `mv x0,x0`

```assembly
2000001c:	00000013   nop
20000020:	157d       addi	a0,a0,-1
20000022:	f97d       bnez	a0,20000018 <run_test_sram+0x18>
```

<p align="center">
   <img src="32bitnop.png" alt="32 bit nop" width="70%">
</p>

## 16 Bit Store

The plot below shows the instruction timing of the loop vs. number of inserted compressed store word instructions. The timing on the CH32V003 seems to be the same irregardless of whether the SRAM or the I/O region is accessed, since it does not have any provisions to speed up GPIO access.

```assembly
20000018:	c214    	sw	a3,0(a2)
2000001a:	157d    	addi	a0,a0,-1
2000001c:	fd75    	bnez	a0,20000018 <run_test_sram+0x18>
```

<p align="center">
   <img src="16bitstore.png" alt="16 bit store" width="70%">
</p>

