all: flash

TARGET := main

TARGET_MCU?=CH32V003

ADDITIONAL_C_FILES :=  
EXTRA_CFLAGS := -I.

# Include the ch32v003fun makefile
include ../src/ch32v003fun/ch32fun/ch32fun.mk

# Flash the firmware to the device
flash : cv_flash

# Clean up build files
clean : cv_clean