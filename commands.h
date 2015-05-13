#ifndef IMP_COMMANDS_H_
#define IMP_COMMANDS_H_
#include <stdbool.h>
#define IMP_DEFAULT_ROOT "/usr/local/imp"


void Imp_debugMode(bool status);
void Imp_useRoot(char *path);
char *Imp_root();

void Imp_launchREPL();
void Imp_executeString(char *code);
void Imp_executeFile(char *path);
void Imp_fetchPackage(char *source);
void Imp_indexPackage(char *source, char *destination);
void Imp_removePackage(char *id);
void Imp_updatePackage(char *id);
void Imp_installProject(char *root);
void Imp_printEnvironment();


#endif