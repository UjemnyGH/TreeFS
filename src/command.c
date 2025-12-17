#include <string.h>
#include "command.h"

int cmd_sizeUntilNewCmd(char **pUser, int userSize, const command_t *const pCommand, int cmdTabSize) {
  int size = 0;
  int found = 0;

  if(pUser == NULL)
    return size;

  while(!found) {
    for(int i = 0; i < cmdTabSize; i++) {
      if((pCommand[i].name && strncmp(((char**)pUser)[size], pCommand[i].name, strlen(pCommand[i].name)) == 0)
        || (pCommand[i].shortName && strncmp(((char**)pUser)[size], pCommand[i].shortName, strlen(pCommand[i].shortName)) == 0)) {
        found = 1;

        return size;
      }
    }

    size++;

    if(size >= userSize)
      break;
  }

  return size;
}

int cmd_run(tfs_t *const pTfs, const char *name, char **pUser, int userSize, const command_t *const pCommands, int cmdTabSize) {
  for(int i = 0; i < cmdTabSize; i++) {
    if(((pCommands[i].name && strcmp(name, pCommands[i].name) == 0) || (pCommands[i].shortName && strcmp(name, pCommands[i].shortName) == 0))
      && pCommands[i].cmd)
      return pCommands[i].cmd(pTfs, pUser, userSize);
  }

  return 0;
}
