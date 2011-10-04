/**
 * Capacitive-touch-sensor, A capacitive touch sensor for the AVR platform.
 * Copyright (C) 2011  Skrupellos
 * 
 * This file is part of Capacitive-touch-sensor.
 * 
 * Capacitive-touch-sensor is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Capacitive-touch-sensor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Capacitive-touch-sensor.
 * If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 * @author Skrupellos
 */


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
