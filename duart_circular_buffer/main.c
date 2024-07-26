/*

v.1 uart with rx interrupts and isr using circular buffer demo

*/

#include <stdio.h>

/* MC68681 config */
#define DUART_BASE 0x900001 				/* Base of I/O port addresses */
#define MR1A (* (volatile char *) (DUART_BASE+0) ) 	/* Mode register 1A 		(R/W) */
#define MR2A (* (volatile char *) (DUART_BASE+0) ) 	/* Mode register 2A 		(R/W) */
#define SRA  (* (volatile char *) (DUART_BASE+2) ) 	/* Status Register A 		(R)   */
#define CSRA (* (char *) (DUART_BASE+2) ) 		/* Clock Select register A 	(W)   */
#define CRA  (* (char *) (DUART_BASE+4) ) 		/* Command Register A 		(W)   */
#define RBRA (* (volatile char *) (DUART_BASE+6) ) 	/* Receiver Buffer  A  	(R)   */
#define TBRA (* (char *) (DUART_BASE+6) ) 		/* Transmiter Buffer A		(W)   */
#define ACR  (* (volatile char *) (DUART_BASE+8) ) 	/* Auxiliary Control Register  (R/W) */

#define ISR   (* (volatile char *) (DUART_BASE+10) )  /* Interrupt Status Register (R)     */
#define IMR   (* (char *) (DUART_BASE+10) ) 		/* Interrupt Mask Register   (W)     */

#define CUR   (* (volatile char *) (DUART_BASE+12) ) 	/* Counter MSB               (R) */
#define CTUR  (* (char *) (DUART_BASE+12) ) 	        /* Counter/Timer Upper Reg   (W) */
#define CLR   (* (volatile char *) (DUART_BASE+14) ) 	/* Counter LSB               (R) */
#define CTLR  (* (char *) (DUART_BASE+14) ) 	        /* Counter/Timer Lower Reg   (W) */

#define MR1B  (* (volatile char *) (DUART_BASE+16) )  /* Mode Register 1B          (R/W)   */
#define MR2B  (* (volatile char *) (DUART_BASE+16) )  /* Mode Register 2B          (R/W)   */
#define SRB   (* (volatile char *) (DUART_BASE+18) )  /* Status Register B         (R)     */
#define CSRB  (* (char *) (DUART_BASE+18) ) 		/* Clock Select Register B   (W)     */
#define CRB   (* (char *) (DUART_BASE+20) ) 		/* Commands Register B       (W)     */
#define RBRB   (* (volatile  char *) (DUART_BASE+22) ) /* Reciever Buffer B         (R)     */
#define TBRB   (* (char *) (DUART_BASE+22) ) 		/* Transmitter Buffer B      (W)     */

#define IVR   (* (volatile char *) (DUART_BASE+24) )  /* Interrupt Vector Register (R/W)   */

#define IPUL   (* (volatile char *) (DUART_BASE+26) )  /* Unlatched Input Port values (R)  */
#define OPCR   (* (char *) (DUART_BASE+26) )            /* Output Port Configuration Register (W)   */

#define STRT_CNTR   (* (volatile char *) (DUART_BASE+28) )  /* tart-Counter Command (R)  */
#define OPR_SET   (* (volatile char *) (DUART_BASE+28) )  /* Output Port Bit Set Command (W)  */
#define STOP_CNTR   (* (volatile char *) (DUART_BASE+30) )  /* Stop-Counter Command (R) / Clear timer interrupt  */
#define OPR_CLR   (* (volatile char *) (DUART_BASE+30) )  /* Output Port Bit Clear Command (W)  */

#define RxRDY 0x01	/* Receiver ready bit mask */
#define TxRDY 0x04	/* Transmiter ready bit mask */

#define iRxRDY 0x02	/* Receiver ISR RxRDY bit mask */
#define iCTRDY 0x04	/* Counter/ timer ISR ready bit mask */

/* digital output on leds config */
//#define DOUT_BASE 0x800001
//#define DOUT (* (char *) (DOUT_BASE+0) ) 

/* circular buffer */
#define CBUFFER_LEN 32
char cbuffer_data[32] = {0};
short int cbuffer_head = 0;
short int cbuffer_tail = 0;
//short int cbuffer_next = 0;

void setup_duart(void);
char getchar_(void);
void putchar_(char c);
int circ_bbuf_push(char data);
int circ_bbuf_pop(char *data);

void __attribute__((interrupt))
duartInterrupt(void) {
	char c;
    if (ISR & iRxRDY) {
        c = RBRA;
		// leds out
		//if( ci > 0X19) DOUT = ci;
        // add char to circular buffer
        circ_bbuf_push(c);		
    };
	
}

/* circular buffer */
int circ_bbuf_push(char data) {
    short int cbuffer_next;

    cbuffer_next = cbuffer_head + 1;  // next is where head will point to after this write.
    if (cbuffer_next >= CBUFFER_LEN)
        cbuffer_next = 0;

    if (cbuffer_next == cbuffer_tail)  // if the head + 1 == tail, circular buffer is full
        return -1;

    cbuffer_data[cbuffer_head] = data;  // Load data and then move
    cbuffer_head = cbuffer_next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}

int circ_bbuf_pop(char *data) {
    short int cbuffer_next;

    if (cbuffer_head == cbuffer_tail)  // if the head == tail, we don't have any data
        return -1;

    cbuffer_next = cbuffer_tail + 1;  // next is where tail will point to after this read.
    if(cbuffer_next >= CBUFFER_LEN)
        cbuffer_next = 0;

    *data = cbuffer_data[cbuffer_tail];  // Read data and then move
    cbuffer_tail = cbuffer_next;              // tail to next offset.
    return 0;  // return success to indicate successful push.
}


/* duart MC68681 */

void setup_duart(void) {
	CRA = 0x30; /* Reset port A transmitter */
	CRA = 0x20; /* Reset port A receiver */
	CRA = 0x10; /* Reset port A mode register pointer */
	ACR = 0x00; /* Select Baud rate Set 1 */
	CSRA = 0xBB; /* Set both the Rx and Tx speeds to 9600 baud */
	MR1A = 0x93; /* Port A 8 bits, no parity, 1 stop bit enable RxRTS output */
	//MR2A = 0x07; /* Basic operating mode */
    MR2A = 0x37; /* Normal operating mode, enable TxRTS, TxCTS, 1 stop bit */
    IMR = 0x02; /* Enable Interrupts: RxRDYA interrupt (bit 1) */
    IVR = 0x50; /* set vector number 0x50 (80d) that is at address 0x140 (320d) (see platform.ld) */
	CRA = 0x05; /* Enable port A transmitter and receiver */
}

char getchar_(void) {
	while ( (SRA & RxRDY) == 0 );
	return RBRA;
}

void putchar_(char c) {
	while ( (SRA & TxRDY) == 0 );
	TBRA = c;
}

int main(void) {
	char c;
    setup_duart();
    printf("hello world\n\r");
    while(1) {
		if(!circ_bbuf_pop( &c)) {
			putchar_(c);
		}
    }

    return 0;
}
