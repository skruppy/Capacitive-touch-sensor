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


// UART
#define BAUD               9600UL

// The following addresses have to be below 0x20
#define PORT_UP    PORTF
#define DDR_UP     DDRF
#define PIN_UP     PF0

#define PORT_DOWN  PORTE
#define DDR_DOWN   DDRE
#define PIN_DOWN   PE3

#define HISTORY_LEN 10
#define SW_SAMPLES 10
#define THRESHOLD 0.98

// CS02:0: Clock Select (clk/64)
#define PRESCALER (_BV(CS01) | _BV(CS00))
