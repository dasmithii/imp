#ifndef IMP_COMMANDS_H_
#define IMP_COMMANDS_H_
#include <stdbool.h>




void Imp_printUsage();
void Imp_useRoot(char *path);
void Imp_launchREPL();
void Imp_executeFile(char *path, int argc, char **argv);
void Imp_fetch(char *source);
void Imp_index(char *dest, char *src);
void Imp_remove(char *id);
void Imp_printEnvironment();




#endif