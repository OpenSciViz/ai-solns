
#include "comboxform.h"

typedef struct {

char** commands;
int ncommands;
int dimen;


} COMMANDS;

/******************************************************************************
*
******************************************************************************/
COMMANDS* allocateCommands();

/******************************************************************************
*
******************************************************************************/
void addCommand(COMMANDS* list, char* command);

/********************************************************************************
*
********************************************************************************/
COMMANDS* readCommandsFromStream(FILE* fp);

/********************************************************************************
*
********************************************************************************/
COMMANDS* readCommandsFromString(char* string);

/******************************************************************************
*
******************************************************************************/
COMMANDS* readCommands(char* source);

/******************************************************************************
*
******************************************************************************/
COMBOXFORM* executeCommands(COMMANDS* list);
