#ifndef CLI_H
#define CLI_H

#include <Arduino.h>
#include <CommandLine.h>

extern bool CLI_Stream;

void CLI_Init();
void CLI_Update();
void CLI_HandleHi(char *tokens);
void CLI_HandleSet(char *tokens);
void CLI_HandleGet(char *tokens);
void CLI_HandleStream(char *tokens);
void CLI_HandleNoStream(char *tokens);
void CLI_HandlePost(char *tokens, bool success);



#endif
