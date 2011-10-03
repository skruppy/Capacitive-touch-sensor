#include "main.h"


// This interrupt gets fired if the TX buffer is empty. Hence the interrupt
// should only wake up the CPU, there is no need to implement an ISR.
EMPTY_INTERRUPT(USART0_UDRE_vect);


// This interrupt gets fired after 5 times \tau, when the capacitor is 99%
// discharged.
ISR(TIMER0_OVF_vect) {
	discharging = false;
}


static int sendUart(char c, FILE *stream) {
	// Automatically convert Unix into Win line endings
	if (c == '\n') {
		sendUart('\r', stream);
	}
	
	// Sleep until the TX buffer is empty
	cli();
	while( !(UCSR0A & _BV(UDRE0)) ) {
		sei();
		sleep();
	}
	
	// Put data into buffer, sends the data
	UDR0 = c;
	
	return 0;
}


static FILE uartOut = FDEV_SETUP_STREAM(sendUart, NULL, _FDEV_SETUP_WRITE);


uint16_t accuireCap(uint8_t samples) {
	// ## INIT #################################################################
	uint32_t sum = 0;
	uint8_t swSamples = samples;
	
	// Both: Disable pull-up / Set to GND
	CBI(PORT_UP, PIN_UP);
	CBI(PORT_DOWN, PIN_DOWN);
	
	while(swSamples--) {
		// ## RESET / EVALUATE #################################################
		uint16_t hwSamples = 0;
		
		// Both: Output (GND)
		SBI(DDR_UP,    PIN_UP);
		SBI(DDR_DOWN,  PIN_DOWN);
		// Both are now connected to GND directly
		
		discharging = true;
		
		// Reset the timer counter
		TCNT0 = 0;
		
		enableTimer();
		cli();
		while(discharging) {
			sei();
			sleep();
		}
		disableTimer();
		
		
		// ## SAMPLE ###########################################################
		do {
			// ==== Switch ports ===============================================
			// Upper: Input
			CBI(DDR_UP,   PIN_UP);
			// Upper is now open
			
			
			// Bottom: Set to Vcc / Enable pull-up
			SBI(PORT_DOWN, PIN_DOWN);
			// First:   Bottom is now connected to Vcc directly
			// Further: Bottom is now connected to Vcc through a pull-up
			
			// Bottom: Output (Vcc)
			SBI(DDR_DOWN, PIN_DOWN);
			// Bottom is now connected to Vcc directly
			
			
			// ==== Wait =======================================================
			if(experience) {
				// http://www.avr-asm-tutorial.net/avr_de/zeitschleifen/
				// delay8.html
				// 21cc / 3 = 7
				uint8_t cnt;
				asm volatile(
					"ldi %0, 7     ; 1cc                      \n"
					"Loop:                                    \n\t"
					"dec %0        ; 1cc                      \n\t"
					"brne Loop     ; 2cc if ne 0, 1cc if eq 0 \n\t"
					: "=&w" (cnt)
				);
			} else {
				asm volatile("nop");
			}
			
			
			// ==== Switch ports ===============================================
			// Bottom: Input (pull-up)
			CBI(DDR_DOWN, PIN_DOWN);
			// Bottom is now connected to Vcc through a pull-up
			
			// Bottom: Disable pull-up / Set to GND
			CBI(PORT_DOWN, PIN_DOWN);
			// Bottom is now open
			
			
			// Upper: Output (GND)
			SBI(DDR_UP, PIN_UP);
			// Upper is now connected to GND directly
			
			
			// ==== Measure ====================================================
			// The analog comparator result is available after 2cc. But
			// incrementing hwSamples takes already longer.
		} while(++hwSamples && (ACSR & _BV(ACO)));
		
		// Add the HW sampled value to the software sampled treating an overflow
		// as a maximum.
		if(hwSamples == 0) {
			sum += 0xFFFF;
		} else {
			sum += hwSamples;
		}
	}
	
	// Return the averaged value
	return sum / samples;
}


int main()
{
	// ==== Timer ==============================================================
	// TCCR0A – Timer/Counter Control Register A
	// WGM01:0: Waveform Generation Mode (Fast PWM)
	TCCR0A = _BV(WGM01) | _BV(WGM00);
	
	// TCCR0B – Timer/Counter Control Register B
	// WGM02: Waveform Generation Mode (Fast PWM)
	TCCR0B = _BV(WGM02);
	
	// OCR0A – Output Compare Register A
	// 64 * 34 = 2176
	OCR0A = 34;
	
	// TIMSK0 – Timer/Counter Interrupt Mask Register
	// TOIE0: Timer/Counter0 Overflow Interrupt Enable
	TIMSK0 = _BV(TOIE0);
	
	
	// ==== UART ===============================================================
	// Set baud rate
	UBRR0H = (unsigned char)(UBRR_VAL>>8);
	UBRR0L = (unsigned char)UBRR_VAL;
	
	// UCSRnB: USART Control and Status Register n B
	// UDRIEn: USART Data Register Empty Interrupt Enable n
	// TXENn: Transmitter Enable n
	UCSR0B = _BV(UDRIE0) | _BV(TXEN0);
	
	
	// ==== Disable input buffers (Analog/Powersave) ===========================
	// DIDR0 – Digital Input Disable Register 0
	// Bit 7:0 – ADC7D:ADC0D: ADC7:0 Digital Input Disable
	DIDR0 = 0xFF;
	
	// DIDR1 – Digital Input Disable Register 1
	// Bit 1, 0 – AIN1D, AIN0D: AIN1, AIN0 Digital Input Disable
	DIDR1 = 0x03;
	
	// DIDR2 – Digital Input Disable Register 2
	// Bit 7:0 – ADC15D:ADC8D: ADC15:8 Digital Input Disable
	DIDR2 = 0xFF;
	
	
	// ==== STDOUT =============================================================
	// Set STDOUT to an UART sink
	stdout = &uartOut;
	
	
	// ==== Debug port =========================================================
	DDRA = 0xFF;
	PORTA = 0xFF;
	
	
	// ==== Arm the interrupts =================================================
	sei();
	
	
	
	uint16_t history[HISTORY_LEN];
	uint16_t *curHistoryItem = history;
	uint8_t isInPeak = false;
	
	while(true) {
		uint16_t cap, minCap, maxCap;
		uint16_t *historyItem;
		uint8_t i;
		
		// Append the current value to the history (ring buffer)
		cap = accuireCap(SW_SAMPLES);
		*curHistoryItem = cap;
		
		if(curHistoryItem == (history + HISTORY_LEN - 1)) {
			curHistoryItem = history;
		} else {
			curHistoryItem++;
		}
		
		// Get the min/max values from the history
		historyItem = history;
		minCap = 0xFFFF;
		maxCap = 0x0000;
		
		for(i = HISTORY_LEN; i; i--) {
			if(minCap > *historyItem) {
				minCap = *historyItem;
			}
			
			if(maxCap < *historyItem) {
				maxCap = *historyItem;
			}
			
			historyItem++;
		}
		
		// Print a message if a touch is recognized
		if(maxCap * THRESHOLD > cap) {
			if(isInPeak == false) {
				printf("Blubb\n");
				isInPeak = true;
			}
		} else {
			isInPeak = false;
		}
		
		// Print some debug messages (you can copy&paste the to Excel; CSV)
		printf("%5d; %5d; %5d; %5d; %5d; %5d\n",
			cap, minCap, maxCap, maxCap - minCap, cap - minCap, maxCap - cap
		);
	}
}

