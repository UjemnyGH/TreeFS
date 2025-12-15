#ifndef _TREE_FILE_SYSTEM_COMMAND_
#define _TREE_FILE_SYSTEM_COMMAND_

typedef int(*PFN_CommandFn)(void *pUser, int userSize);

typedef struct command_s {
  const char *name;
  const char *shortName;
  const char* description;
  PFN_CommandFn cmd;
} command_t;

int cmd_sizeUntilNewCmd(void *pUser, const command_t *const pCommand, int cmdTabSize);

int cmd_run(const char *name, void *pUser, int userSize, const command_t *const pCommands, int cmdTabSize);

#endif
