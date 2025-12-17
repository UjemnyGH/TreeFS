#ifndef _TREE_FILE_SYSTEM_COMMAND_
#define _TREE_FILE_SYSTEM_COMMAND_

#include "tfs/treefs.h"

typedef int(*PFN_CommandFn)(tfs_t *const pTfs, char **pUser, int userSize);

typedef struct command_s {
  const char *name;
  const char *shortName;
  const char* description;
  PFN_CommandFn cmd;
} command_t;

int cmd_sizeUntilNewCmd(char **pUser, int userSize, const command_t *const pCommand, int cmdTabSize);

int cmd_run(tfs_t *const pTfs, const char *name, char **pUser, int userSize, const command_t *const pCommands, int cmdTabSize);

#endif
