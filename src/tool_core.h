#ifndef _TREE_FILE_SYSTEM_TOOL_CORE_
#define _TREE_FILE_SYSTEM_TOOL_CORE_

#include "command.h"
#include <stdlib.h>

extern char *gCurrentDirectory;
extern char *gModifiedTFSPartition;

int cmd_help(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_newFS(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_mount(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_mkdir(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_mkfile(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_writeFile(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_readFile(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_removeFile(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_removeDir(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_changeDir(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_listDir(tfs_t *const pTfs, char **pUser, int userSize);
int cmd_rename(tfs_t *const pTfs, char **pUser, int userSize);

static const command_t gCommands[] = {
  {"--help", "-h", "Shows this text", cmd_help},
  {"--mount", "-m", "Mounts desired block device: -m/--mount <deviceName>", cmd_mount},
  {"--format-drive", NULL, "Makes new file system: --format-drive <deviceName> <size_kB>", cmd_newFS},
  {"--make-directory", "-mkdir", "Makes new directory: -mkdir/--make-directory <path (optional)> <directoryName>", cmd_mkdir},
  {"--make-file", "-mkf", "Create new file: -mkf/--make-file <path (optional)> <filename>", cmd_mkfile},
  {"--write", "-w", "Writes file from PC disk, max 4GB: -w/--write <path> <inputFileFromPC>", cmd_writeFile},
  {"--read", "-r", "Reads file from TreeFS, max 4GB: -r/--read <path> <outputFileOnPC>", cmd_readFile},
  {"--remove-file", "-rmf", "Removes file from TreeFS: -rmf/--remove-file <path>", cmd_removeFile},
  {"--remove-directory", "-rmdir", "Removes directory from TreeFS: -rmdir/--remove-directory <parh>", cmd_removeDir},
  {"--change-directory", "-cd", "Changes relative directory: -cd/--change-directory <path>", cmd_changeDir},
  {"--list-directory", "-ls", "List all files and directories in directory: -ls/--list-directory <path>", cmd_listDir},
  {"--rename", "-rn", "Changes name of directory or file: -rn/--rename <path> <newName>", cmd_rename}
};

#endif
