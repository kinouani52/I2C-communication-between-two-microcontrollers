//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Master RX single bytes from MSP430 Slave
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  reads from the slave. This is the master code. The data from the slave
//  transmitter begins at 0 and increments with each transfer. The received
//  data is in R5 and is checked for validity. If the received data is
//  incorrect, the CPU is trapped and the P1.0 LED will stay on. The USCI_B0
//  RX interrupt is used to know when new data has been received.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.2MHz
//
//  *** to be used with "msp430g2xx3_uscib0_i2c_05.c" ***
//
//                                /|\  /|\
//               MSP430G2xx3      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P1.7/UCB0SDA|<-|---+->|P1.7/UCB0SDA  XIN|-
//            |                 |  |      |                 | 32kHz
//           -|XOUT             |  |      |             XOUT|-
//            |     P1.6/UCB0SCL|<-+----->|P1.6/UCB0SCL     |
//            |                 |         |             P1.0|--> LED
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include <msp430.h>

unsigned char RXData;
unsigned char RXCompare;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1OUT &= ~BIT0;                           // P1.0 = 0
  P1DIR |= BIT0;                            // P1.0 output
  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                      // Enable SW reset
  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
  UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
  UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
  UCB0BR1 = 0;
  UCB0I2CSA = 0x048;                        // Slave Address is 048h
  UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
  IE2 |= UCB0RXIE;                          // Enable RX interrupt
  RXCompare = 0;                            // Used to check incoming data

  Aff_Init();
  char count[] = { '0', '0', '0', '0' };

  while (1)
  {
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTXSTT;                    // I2C start condition
    while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
    UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts

    if (RXData != RXCompare)                // Trap CPU if wrong
    {
      P1OUT |= BIT0;                        // P1.0 = 1
      while (1);                            // Trap CPU
    }

    RXCompare++;                            // Increment correct RX value

    Aff_4carac(count);
            __delay_cycles(1000000);
            if (count[3] == '9')
            {
                count[3] = '0';

                if (count[2] == '9')
                    count[2] = '0';
                else
                    count[2]++;

            }
            else
                count[3]++;

  }
}

// USCI_B0 Data ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCIAB0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  RXData = UCB0RXBUF;                       // Get RX data
  __bic_SR_register_on_exit(CPUOFF);        // Exit LPM0
}
