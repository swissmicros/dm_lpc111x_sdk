#ifndef __CMD_H__
#define __CMD_H__

#include "projectconfig.h"

typedef struct
{
  char *command;
  uint8_t minArgs;
  void (*func)(uint8_t argc, char **argv);
  const char *description;
  const char *parameters;
} cmd_t;

void cmdPoll();
void cmdRx(uint8_t c);
void cmdParse(char *cmd);
void cmdInit();

extern int repeat_cmd;

#endif
