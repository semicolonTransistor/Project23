#include "CLI.h"

CommandLine Cli(Serial,"> ");
bool CLI_Stream = false;

void CLI_Init(){
  CLI_Stream = false;
  Cli.attachPost(CLI_HandlePost);
  Cli.add("hi", CLI_HandleHi);
  Cli.add("stream", CLI_HandleStream);
  Cli.add("nostream", CLI_HandleNoStream);
}

void CLI_Update(){
  Cli.update();
}

void CLI_HandleHi(char *tokens){
  Serial.println("Hi.");
  Serial.println("Responce From Project 23");
  Serial.println("HW Ver: 0.4");
  Serial.println("SW Ver: dev");
}

void CLI_HandleSet(char *tokens){

}

void CLI_HandleStream(char *tokens){
  CLI_Stream = true;
  Serial.println("Data Stream Started.");
}

void CLI_HandleNoStream(char *tokens){
  CLI_Stream = false;
  Serial.println("Data Stream Stoped.");

}

void CLI_HandlePost(char *tokens, bool success){
  if (!success) {
    Serial.println("Unknown command. Type 'help' for help.");
  }
}
