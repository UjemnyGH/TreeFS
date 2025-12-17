#include "tool_core.h"
#include <stdio.h>
#include <string.h>

char *gCurrentDirectory = NULL;
char *gModifiedTFSPartition = NULL;

char *_cmd_allocAbsDir(const char* path) {
  const int size = (path[0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(path) + 3;

  char *absPath = malloc(size);
  memset(absPath, 0, size);
  if(path[0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, path);

  return absPath;
}

int cmd_help(tfs_t *const pTfs, char **pUser, int userSize) {
  (void)pTfs;

  printf("Usage: %s [OPTIONS]\n", ((char**)pUser - 2)[0]);

  const int alignText = 20;
  const int alignShortText = 10;

  for(int i = 0; i < sizeof(gCommands) / sizeof(gCommands[0]); i++) {
    printf("\t%s", gCommands[i].name);

    for(int j = 0; j < alignText - strlen(gCommands[i].name); j++)
      printf(" ");

    if(gCommands[i].shortName)
      printf("| %s", gCommands[i].shortName);

    for(int j = 0; j < (gCommands[i].shortName ? alignShortText - strlen(gCommands[i].shortName) : alignShortText + 2); j++)
      printf(" ");

    printf("-  %s\n", gCommands[i].description);
  }

  return 0x7FFFFFFF;
}

int cmd_newFS(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = (char**)pUser;

  printf("%s %s\n", user[0], user[1]);

  if(!gModifiedTFSPartition)
    gModifiedTFSPartition = (char*)malloc(strlen(user[0]) + 1);
  else
    gModifiedTFSPartition = (char*)realloc(gModifiedTFSPartition, strlen(user[0]) + 1);
  strncpy(gModifiedTFSPartition, user[0], strlen(user[0]));

  printf("Formatting %s drive: %d kB\n", user[0], atoi(user[1]));

  if(!tfs_newFileSystem(pTfs, atoi(user[1])))
    return 1;

  printf("Drive formatted\n");

  return 0;
}

int cmd_mount(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 1)
    return 0x7FFFFFFF;

  char **user = (char**)pUser;

  if(!gModifiedTFSPartition)
    gModifiedTFSPartition = (char*)malloc(strlen(user[0]) + 1);
  else
    gModifiedTFSPartition = (char*)realloc(gModifiedTFSPartition, strlen(user[0]) + 1);
  
  memset(gModifiedTFSPartition, 0, strlen(user[0]) + 1);
  strncpy(gModifiedTFSPartition, user[0], strlen(user[0]));

  printf("Mouting drive %s\n", gModifiedTFSPartition);

  if(!tfs_mount(pTfs))
    return 1;

  printf("Drive mounted\n");

  return 0;
}

int cmd_mkdir(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);

  char *absPath = _cmd_allocAbsDir(user[0]);
  tfs_uint32_t dirAddress = tfs_findAbsolutePath(pTfs, absPath);

  free(absPath);

  if(!tfs_isDirectory(pTfs, dirAddress)) {
    printf("Path does not describe directory\n");

    return 1;
  }

  tfs_createDirectory(pTfs, dirAddress, user[1]);

  return 0;
}

int cmd_mkfile(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]); 

  tfs_uint32_t dirAddress = tfs_findAbsolutePath(pTfs, absPath);

  free(absPath);

  if(!tfs_isDirectory(pTfs, dirAddress)) {
    printf("Path does not describe directory\n");

    return 1;
  }

  tfs_createFile(pTfs, dirAddress, user[1]);

  return 0;
}

int cmd_writeFile(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);
    
  tfs_uint32_t fileAddress = tfs_findAbsolutePath(pTfs, absPath);

  free(absPath);

  if(!tfs_isFile(pTfs, fileAddress)) {
    printf("Cannot open file to write!\n");

    return 1;
  }

  FILE *pcFile = fopen(user[1], "rb+");
  fseek(pcFile, 0, SEEK_END);
  tfs_uint32_t len = ftell(pcFile);
  fseek(pcFile, 0, SEEK_SET);

  tfs_uint8_t *data = calloc(len + 1, sizeof(tfs_uint8_t));

  fread(data, sizeof(tfs_uint8_t), len, pcFile);

  fclose(pcFile);

  tfs_writeFile(pTfs, fileAddress, data, len, 0);

  free(data);

  return 0;
}

int cmd_readFile(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);
    
  tfs_uint32_t fileAddress = tfs_findAbsolutePath(pTfs, absPath);

  free(absPath);

  if(!tfs_isFile(pTfs, fileAddress)) {
    printf("Cannot read directory as file!\n");

    return 1;
  }

  tfs_uint32_t fileSize = tfs_getFileSize(pTfs, fileAddress);

  tfs_uint8_t *fileBuffer = calloc(fileSize + 1, sizeof(tfs_uint8_t));

  tfs_readFile(pTfs, fileAddress, fileBuffer, fileSize, 0);

  FILE *pcFile = fopen(user[1], "wb+");
  fwrite(fileBuffer, sizeof(tfs_uint8_t), fileSize, pcFile);
  fclose(pcFile);

  free(fileBuffer);

  return 0;
}

int cmd_removeFile(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 1)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);

  tfs_uint32_t fileAddress = tfs_findAbsolutePath(pTfs, absPath); 
  tfs_result_t removed = tfs_removeFile(pTfs, fileAddress);

  free(absPath);

  return removed;
}

int cmd_removeDir(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 1)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);

  tfs_uint32_t fileAddress = tfs_findAbsolutePath(pTfs, absPath); 
  tfs_result_t removed = tfs_removeDirectory(pTfs, fileAddress);

  free(absPath);

  return removed;
}

int cmd_changeDir(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 1)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);

  char *lastDir = malloc(strlen(gCurrentDirectory) + 1);
  memset(lastDir, 0, strlen(gCurrentDirectory) + 1);
  strcpy(lastDir, gCurrentDirectory);

  gCurrentDirectory = realloc(gCurrentDirectory, (user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);

  memset(gCurrentDirectory, 0, (user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]));

  if(user[0][0] != '/') {
    strcpy(gCurrentDirectory, lastDir);
    strcat(gCurrentDirectory, "/");
  }

  strcat(gCurrentDirectory, user[0]);

  if(tfs_isFile(pTfs, tfs_findAbsolutePath(pTfs, gCurrentDirectory))) {
    printf("Cannot change directory to file! Reverting back.\n");

    gCurrentDirectory = realloc(gCurrentDirectory, strlen(lastDir) + 1);
    strcpy(gCurrentDirectory, lastDir);
  }

  free(lastDir);

  return 0;
}

int cmd_listDir(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 1)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);
 
  char names[512 * TFS_FILENAME_SIZE];

  tfs_uint32_t dirAddress = tfs_findAbsolutePath(pTfs, absPath);
  printf("READING @ %d\n", dirAddress);

  tfs_uint32_t count = tfs_listDirectory(pTfs, dirAddress, names, 512);

  printf("Listed directories in %s\n", absPath);

  for(tfs_uint32_t i = 0; i < count; i++) {
    printf("%s\n", (names + (i * TFS_FILENAME_SIZE)));
  }

  free(absPath);

  return 0;
}

int cmd_rename(tfs_t *const pTfs, char **pUser, int userSize) {
  if(userSize < 2)
    return 0x7FFFFFFF;

  char **user = ((char**)pUser);
  char *absPath = _cmd_allocAbsDir(user[0]);

  tfs_bool_t renamed = tfs_renameEntry(pTfs, tfs_findAbsolutePath(pTfs, absPath), user[1]);

  free(absPath);

  if(renamed)
    printf("File renamed to %s\n", user[1]);

  return renamed ? 0 : 1;
}
