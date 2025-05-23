#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

// Though this should be on by default we can extra force it on.
#define FUNCONF_USE_DEBUGPRINTF 1
#define FUNCONF_DEBUGPRINTF_TIMEOUT (1<<31) // Wait for a very very long time.

#define FUNCONF_USE_HSE 1  			// external crystal on PA1 PA2
#define FUNCONF_USE_HSI 0    		// internal 24MHz clock oscillator
#define FUNCONF_USE_PLL 1			// use PLL x2
#define FUNCONF_HSE_BYPASS 0 		// bypass the HSE when using an external clock source
									// requires enabled HSE
#define FUNCONF_USE_CLK_SEC	1		// clock security system

#define FUNCONF_SYSTICK_USE_HCLK 1
#define FUNCONF_SYSTICK_PRESCALE 1  // No prescaler for most accurate timing

#define CH32V003        1

// Ensure the .srodata section is placed in SRAM
// This ensures any function with __attribute__((section(".srodata"))) runs from SRAM
#define FUNCONF_SRODATA_IN_RAM 1


#endif /* _FUNCONFIG_H */