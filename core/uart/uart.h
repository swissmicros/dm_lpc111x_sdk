#ifndef __UART_H__ 
#define __UART_H__

#include "projectconfig.h"

// Buffer used for circular fifo
typedef struct _uart_buffer_t
{
  uint8_t ep_dir;
  //volatile uint8_t len;
  volatile uint8_t wr_ptr;
  volatile uint8_t rd_ptr;
  uint8_t buf[CFG_UART_BUFSIZE];
} uart_buffer_t;

// UART Protocol control block
typedef struct _uart_pcb_t
{
  BOOL initialised;
  uint32_t status;
  uint32_t pending_tx_data;
  uart_buffer_t rxfifo;
} uart_pcb_t;

void UART_IRQHandler(void);
uart_pcb_t *uartGetPCB();
void uartInit(uint32_t Baudrate);
void uartSend(uint8_t *BufferPtr, uint32_t Length);
void uartSendByte (uint8_t byte);

// Rx Buffer access control
void uartRxBufferInit();
uint8_t uartRxBufferRead();
void uartRxBufferWrite(uint8_t data);
void uartRxBufferClearFIFO();
uint8_t uartRxBufferDataPending();
bool uartRxBufferReadArray(byte_t* rx, size_t* len);

#endif
