#include <stdio.h>
#include <string.h>

#include "core/systick/systick.h"

#include "cmd.h"
#include "project/cmd_tbl.h"

#ifdef CFG_PRINTF_UART
#include "core/uart/uart.h"
#endif

#define CR CFG_PRINTF_NEWLINE

static uint8_t msg[CFG_INTERFACE_MAXMSGSIZE];
static uint8_t *msg_ptr;

/**************************************************************************/
/*! 
    @brief  Polls the relevant incoming message queue to see if anything
            is waiting to be processed.
*/
/**************************************************************************/
void cmdPoll()
{
  #if defined CFG_PRINTF_UART
  while (uartRxBufferDataPending())
  {
    uint8_t c = uartRxBufferRead();
    cmdRx(c);
  }
  #endif
}

/**************************************************************************/
/*! 
    @brief  Handles a single incoming character.  If a new line is 
            detected, the entire command will be passed to the command
            parser.  If a text character is detected, it will be added to
            the message buffer until a new line is detected (up to the
            maximum queue size, CFG_INTERFACE_MAXMSGSIZE).

    @param[in]  c
                The character to parse.
*/
/**************************************************************************/
void cmdRx(uint8_t c)
{
  // read out the data in the buffer and echo it back to the host.
  switch(c) {
  case '\r':
  case '\n':
    // terminate the msg and reset the msg ptr. then send
    // it to the handler for processing.
    *msg_ptr = '\0';
    printf(CR);
    cmdParse((char *)msg);
    msg_ptr = msg;
    break;
  case '\b':
  case '\x7f':
    if (msg_ptr > msg) {
      printf("%c", c);
      msg_ptr--;
    }
    break;
  default:
    printf("%c", c);
    *msg_ptr++ = c;
    break;
  }
}

/**************************************************************************/
/*! 
    @brief  Displays the command prompt.  The text that appears is defined
            in projectconfig.h.
*/
/**************************************************************************/
static void cmdMenu()
{
  printf(CFG_INTERFACE_PROMPT);
}

/**************************************************************************/
/*! 
    @brief  Parse the command line. This function tokenizes the command
            input, then searches for the command table entry associated
            with the commmand. Once found, it will jump to the
            corresponding function.

    @param[in]  cmd
                The entire command string to be parsed
*/
/**************************************************************************/

#define MAXARG_SIZE 8


static cmd_t *cur_cmd = NULL;
int repeat_cmd = 0;

void cmdParse(char *cmd)
{
  size_t argc=0;
  uint8_t ma=0;
  char *argv[MAXARG_SIZE];
  char *s = cmd;

  if (repeat_cmd) {
    argv[argc++]=cmd;
    cur_cmd->func(argc,argv);
    goto toend;
  }

#if 0 // DEBUG
  {
    char *s = cmd;
    for(; *s; s++) {
      printf(" %02x",*s & 0xff);
    }
    printf(CR);
  }
#endif

  while(*s && argc<MAXARG_SIZE)
  {
    while(*s==' ') s++; // skip spaces
    if(!*s) break;
    argv[argc++]=s;
    if(ma==0xff) { // Don't parse when requested in command
      ma=0; break;
    }
    while(*s && *s!=' ') s++; // Skip to space or end of str
    if(*s==' '){ *s=0; s++;}  // Space to end of str to separate this part

    if(argc==1) {
      int i;
      // Find a command to know what to do next
      for (cur_cmd = cmd_tbl,i=0; i<CMD_COUNT; i++,cur_cmd++)
      {
        if (!strcmp(argv[0], cur_cmd->command)) break;
      }
      if(i==CMD_COUNT) {
        // Command not found
        cmd_help(0,NULL);
        cmdMenu();
        return;
      }
      //ma=cmd_tbl[i].minArgs;
      ma=cur_cmd->minArgs;
    }
  }

  if (argc==0)
    goto toend;

  if ( ((argc == 2) && !strcmp (argv[1], "?")) ||
       ( argc <= ma ) )
  {
    // Something is wrong ... display help
    printf ("%s" CR CR, cur_cmd->description);
    printf ("%s" CR,    cur_cmd->parameters);
  }
  else
  {
    // Dispatch command to the appropriate function
    cur_cmd->func(argc,argv);
  }
toend:
  if (!repeat_cmd)
    cmdMenu();
  return;
}


/**************************************************************************/
/*! 
    @brief Initialises the command line using the appropriate interface
*/
/**************************************************************************/
void cmdInit()
{
  #if defined CFG_INTERFACE && defined CFG_INTERFACE_UART
  // Check if UART is already initialised
  uart_pcb_t *pcb = uartGetPCB();
  if (!pcb->initialised)
  {
    uartInit(CFG_UART_BAUDRATE);
  }
  #endif

  // init the msg ptr
  msg_ptr = msg;

  // Show the menu
  cmdMenu();
}

/**************************************************************************/
/*! 
    'help' command handler
*/
/**************************************************************************/
void cmd_help(uint8_t argc, char **argv)
{
  size_t i;
  printf("Firmware %s_" CFG_FIRMWARE_VERSION_REVISION CR, CFG_FIRMWARE_VERSION);
  //printf("Command    Description%s", CFG_PRINTF_NEWLINE);
  printf(CFG_PRINTF_NEWLINE);

  // Display full command list
  for (i=0; i < CMD_COUNT; i++)
  {
     printf ("%-10s %s%s", cmd_tbl[i].command, cmd_tbl[i].description, CFG_PRINTF_NEWLINE);
  }
  printf(CR "Type '<command> ?' for params" CR);
  printf("Uptime %us" CR, (unsigned int)systickGetSecondsActive());
}
