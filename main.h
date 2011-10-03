#include <avr/io.h>        // this contains all the IO port definitions
#include <avr/interrupt.h> // definitions for interrupts
#include <avr/sleep.h>     // definitions for power-down modes
#include <stdio.h>
#include <stdbool.h>
#include "config.h"

// ==== Self-Configuration =====================================================
// CPU Frequency
#ifndef F_CPU
#	error "F_CPU is not defined."
#endif

// UART
#define UBRR_VAL   (F_CPU/BAUD/16)


// Global variable declarations
volatile uint8_t discharging;


// Prototypes
static int sendUart(char c, FILE *stream);
uint16_t accuireCap(uint8_t samples);


// Constants
#define theory true
#define experience false


// Macros
#define enableTimer() { TCCR0B |= PRESCALER; }

#define disableTimer() { TCCR0B &= ~PRESCALER; }

#define SBI(port, bit) \
	{asm volatile("sbi %0, %1" \
		: \
		: "I" (_SFR_IO_ADDR(port)), "I" (bit));}

#define CBI(port, bit) \
	{asm volatile("cbi %0, %1" \
		: \
		: "I" (_SFR_IO_ADDR(port)), "I" (bit));}

#define sleep() \
	{sleep_enable(); \
	sleep_cpu(); \
	sleep_disable();}

