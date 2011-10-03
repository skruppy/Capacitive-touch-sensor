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
