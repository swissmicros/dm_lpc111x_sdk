#include <string.h>

#include "uart.h"
#include "core/cpu/cpu.h"

#ifdef CFG_PRINTF_UART
  #include "core/cmd/cmd.h"
#endif

/**************************************************************************/
/*!
    UART protocol control block, which is used to safely access the
    RX FIFO buffer from elsewhere in the code.  This should be accessed
    through 'uartGetPCB()'.
*/
/**************************************************************************/
static uart_pcb_t pcb;

/**************************************************************************/
/*!
    IRQ to handle incoming data, etc.
*/
/**************************************************************************/
void UART_IRQHandler(void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;

  IIRValue = UART_U0IIR;
  IIRValue &= ~(UART_U0IIR_IntStatus_MASK); /* skip pending bit in IIR */
  IIRValue &= UART_U0IIR_IntId_MASK;        /* check bit 1~3, interrupt identification */

  // 1.) Check receiver line status
  if (IIRValue == UART_U0IIR_IntId_RLS)
  {
    LSRValue = UART_U0LSR;
    // Check for errors
    if (LSRValue & (UART_U0LSR_OE | UART_U0LSR_PE | UART_U0LSR_FE | UART_U0LSR_RXFE | UART_U0LSR_BI))
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      pcb.status = LSRValue;
      Dummy = UART_U0RBR;       /* Dummy read on RX to clear interrupt, then bail out */
      return;
    }
    // No error and receive data is ready
    if (LSRValue & UART_U0LSR_RDR_DATA)
    {
      /* If no error on RLS, normal ready, save into the data buffer. */
      /* Note: read RBR will clear the interrupt */
      uartRxBufferWrite(UART_U0RBR);
    }
  }

  // 2.) Check receive data available
  else if (IIRValue == UART_U0IIR_IntId_RDA)
  {
    // Add incoming text to UART buffer
    uartRxBufferWrite(UART_U0RBR);
  }

  // 3.) Check character timeout indicator
  else if (IIRValue == UART_U0IIR_IntId_CTI)
  {
    /* Bit 9 as the CTI error */
    pcb.status |= 0x100;
  }

  // 4.) Check THRE (transmit holding register empty)
  else if (IIRValue == UART_U0IIR_IntId_THRE)
  {
    /* Check status in the LSR to see if valid data in U0THR or not */
    LSRValue = UART_U0LSR;
    if (LSRValue & UART_U0LSR_THRE)
    {
      pcb.pending_tx_data = 0;
    }
    else
    {
      pcb.pending_tx_data= 1;
    }
  }
  return;
}

/**************************************************************************/
/*!
    @brief  Get a pointer to the UART's protocol control block, which can
            be used to control the RX FIFO buffer and check whether UART
            has already been initialised or not.

    @section Example

    @code 
    // Make sure that UART is initialised
    uart_pcb_t *pcb = uartGetPCB();
    if (!pcb->initialised)
    {
      uartInit(CFG_UART_BAUDRATE);
    }
    @endcode

*/
/**************************************************************************/
uart_pcb_t *uartGetPCB()
{
    return &pcb;
}

/**************************************************************************/
/*! 
    @brief Initialises UART at the specified baud rate.

    @param[in]  baudRate
                The baud rate to use when configuring the UART.
*/
/**************************************************************************/
void uartInit(uint32_t baudrate)
{
  uint32_t fDiv;
  volatile uint32_t regVal = 0;

  NVIC_DisableIRQ(UART_IRQn);

  // Clear protocol control blocks
  memset(&pcb, 0, sizeof(uart_pcb_t));
  pcb.pending_tx_data = regVal; // = 0;
  //uartRxBufferInit();

  /* Set 1.6 UART RXD */
  IOCON_PIO1_6 &= ~IOCON_PIO1_6_FUNC_MASK;
  IOCON_PIO1_6 |= IOCON_PIO1_6_FUNC_UART_RXD;

  /* Set 1.7 UART TXD */
  IOCON_PIO1_7 &= ~IOCON_PIO1_7_FUNC_MASK;
  IOCON_PIO1_7 |= IOCON_PIO1_7_FUNC_UART_TXD;

  /* Enable UART clock */
  SCB_SYSAHBCLKCTRL |= (SCB_SYSAHBCLKCTRL_UART);
  SCB_UARTCLKDIV = SCB_UARTCLKDIV_DIV1;     /* divided by 1 */

  /* 8 bits, no Parity, 1 Stop bit */
  UART_U0LCR = (UART_U0LCR_Word_Length_Select_8Chars |
                UART_U0LCR_Stop_Bit_Select_1Bits |
                UART_U0LCR_Parity_Disabled |
                UART_U0LCR_Parity_Select_OddParity |
                UART_U0LCR_Break_Control_Disabled |
                UART_U0LCR_Divisor_Latch_Access_Enabled);

  /* Baud rate */
#if CFG_UART_BAUDRATE == 921600 || CFG_UART_BAUDRATE == 576000
  fDiv = 3;
#else
  fDiv = ((current_clock*SCB_SYSAHBCLKDIV/SCB_UARTCLKDIV)/16)/baudrate;
  UART_U0FDR = 0x10; // Fix baudrate after serial programming
#endif

#if CFG_UART_BAUDRATE == 921600
  UART_U0FDR = 0xc1;
#endif
#if CFG_UART_BAUDRATE == 576000
  UART_U0FDR = 0x43;
#endif
#if CFG_UART_BAUDRATE == 115200
  if ( fDiv == 6)
    UART_U0FDR = 0xc1;
#endif
#if CFG_UART_BAUDRATE == 230400
  if ( fDiv == 3)
    UART_U0FDR = 0xc1;
#endif

  UART_U0DLM = fDiv >> 8;
  UART_U0DLL = fDiv & 0xff;


  /* Set DLAB back to 0 */
  UART_U0LCR = (UART_U0LCR_Word_Length_Select_8Chars |
                UART_U0LCR_Stop_Bit_Select_1Bits |
                UART_U0LCR_Parity_Disabled |
                UART_U0LCR_Parity_Select_OddParity |
                UART_U0LCR_Break_Control_Disabled |
                UART_U0LCR_Divisor_Latch_Access_Disabled);
  
  /* Enable and reset TX and RX FIFO. */
  UART_U0FCR = (UART_U0FCR_FIFO_Enabled | 
                UART_U0FCR_Rx_FIFO_Reset | 
                UART_U0FCR_Tx_FIFO_Reset); 

  /* Read to clear the line status. */
  regVal = UART_U0LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */
  while (( UART_U0LSR & (UART_U0LSR_THRE|UART_U0LSR_TEMT)) != (UART_U0LSR_THRE|UART_U0LSR_TEMT) );
  while ( UART_U0LSR & UART_U0LSR_RDR_DATA )
  {
    /* Dump data from RX FIFO */
    regVal = UART_U0RBR;
  }

  /* Set the initialised flag in the protocol control block */
  pcb.initialised = 1;

  /* Enable the UART Interrupt */
  NVIC_EnableIRQ(UART_IRQn);
  UART_U0IER = UART_U0IER_RBR_Interrupt_Enabled | UART_U0IER_RLS_Interrupt_Enabled;

  return;
}

/**************************************************************************/
/*! 
    @brief Sends the contents of supplied text buffer over UART.

    @param[in]  bufferPtr
                Pointer to the text buffer
    @param[in]  bufferPtr
                The size of the text buffer

    @section Example

    @code 
    // Set 5-character text buffer
    uint8_t uartBuffer[5] = { 'T', 'e', 's', 't', '\n' };
    // Send contents of uartBuffer
    uartSend((uint8_t *)uartBuffer, 5);
    @endcode

*/
/**************************************************************************/
void uartSend (uint8_t *bufferPtr, uint32_t length)
{
  while (length != 0)
  {
    /* THRE status, contain valid data */
    while ( !(UART_U0LSR & UART_U0LSR_THRE) );
    UART_U0THR = *bufferPtr;

    bufferPtr++;
    length--;
  }

  return;
}

/**************************************************************************/
/*! 
    @brief Sends a single byte over UART.

    @param[in]  byte
                Byte value to send

    @section Example

    @code 
    // Send 0xFF over UART
    uartSendByte(0xFF);
    // Send 'B' over UART (note single quotes)
    uartSendByte('B');
    @endcode

*/
/**************************************************************************/
void uartSendByte (uint8_t byte)
{
  /* THRE status, contain valid data */
  while ( !(UART_U0LSR & UART_U0LSR_THRE) );
  UART_U0THR = byte;

  return;
}



