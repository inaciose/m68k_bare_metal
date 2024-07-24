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
#define MR1B  (* (volatile char *) (DUART_BASE+16) )  /* Mode Register 1B          (R/W)   */
#define MR2B  (* (volatile char *) (DUART_BASE+16) )  /* Mode Register 2B          (R/W)   */
#define SRB   (* (volatile char *) (DUART_BASE+18) )  /* Status Register B         (R)     */
#define CSRB  (* (char *) (DUART_BASE+18) ) 		/* Clock Select Register B   (W)     */
#define CRB   (* (char *) (DUART_BASE+20) ) 		/* Commands Register B       (W)     */
#define RBB   (* (volatile  char *) (DUART_BASE+22) ) /* Reciever Buffer B         (R)     */
#define TBB   (* (char *) (DUART_BASE+22) ) 		/* Transmitter Buffer B      (W)     */
#define IVR   (* (volatile char *) (DUART_BASE+24) )  /* Interrupt Vector Register (R/W)   */


#define RxRDY 0x01	/* Receiver ready bit mask */
#define TxRDY 0x04	/* Transmiter ready bit mask */

/* digital output on leds config */
#define DOUT_BASE 0x800001
#define DOUT (* (char *) (DOUT_BASE+0) ) 

void setup_duart(void) {
	CRA = 0x30; /* Reset port A transmitter */
	CRA = 0x20; /* Reset port A receiver */
	CRA = 0x10; /* Reset port A mode register pointer */
	ACR = 0x00; /* Select Baud rate Set 1 */
	CSRA = 0xBB; /* Set both the Rx and Tx speeds to 9600 baud */
	MR1A = 0x93; /* Port A 8 bits, no parity, 1 stop bit enable RxRTS output */
	MR2A = 0x07; /* Normal operating mode, enable TxRTS, TxCTS, 1 stop bit */
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
    	c = getchar_();
    	putchar_(c);
    	DOUT = c;
    }
    return 0;
}
