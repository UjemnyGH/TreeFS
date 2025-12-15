#include <string.h>
#include "command.h"

int cmd_sizeUntilNewCmd(void *pUser, const command_t *const pCommand, int cmdTabSize) {
  int size = 0;
  int found = 0;

  while(!found) {
    for(int i = 0; i < cmdTabSize; i++) {
      if(strcmp(((char**)pUser)[size], pCommand[i].name) == 0 || strcmp(((char**)pUser)[size], pCommand[i].shortName) == 0) {
        found = 1;

        return size;
      }
    }

    size++;
  }

  return size;
}

int cmd_run(const char *name, void *pUser, int userSize, const command_t *const pCommands, int cmdTabSize) {
  for(int i = 0; i < cmdTabSize; i++) {
    if((pCommands[i].name && strcmp(name, pCommands[i].name) == 0) || (pCommands[i].shortName && strcmp(name, pCommands[i].shortName) == 0))
      return pCommands[i].cmd(pUser, userSize);
  }

  return 0;
}
