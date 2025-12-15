#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tfs/treefs.h"
#include "command.h"

char *gCurrentDirectory = NULL;
char *gModifiedTFSPartition = NULL;
tfs_t gTFS;

int cmd_help(void *pUser, int userSize);
int cmd_newFS(void *pUser, int userSize);
int cmd_mount(void *pUser, int userSize);
int cmd_mkdir(void *pUser, int userSize);
int cmd_mkfile(void *pUser, int userSize);
int cmd_writeFile(void *pUser, int userSize);
int cmd_readFile(void *pUser, int userSize);
int cmd_removeFile(void *pUser, int userSize);
int cmd_changeDir(void *pUser, int userSize);
int cmd_rename(void *pUser, int userSize);

const command_t gCommands[] = {
  {"--help", "-h", "Shows this text", cmd_help},
  {"--mount", "-m", "Mounts desired block device: -m/--mount <deviceName>", cmd_mount},
  {"--format-drive", NULL, "Makes new file system: --format-drive <deviceName> <size_kB>", cmd_newFS},
  {"--make-directory", "-mkdir", "Makes new directory: -mkdir/--make-directory <path (optional)> <directoryName>", cmd_mkdir},
  {"--make-file", "-mkf", "Create new file: -mkf/--make-file <path (optional)> <filename>", cmd_mkfile},
  {"--write", "-w", "Writes file from PC disk, max 4GB: -w/--write <path> <inputFileFromPC>", cmd_writeFile},
  {"--read", "-r", "Reads file from TreeFS, max 4GB: -r/--read <path> <outputFileOnPC>", cmd_readFile},
  {"--remove-file", "-rmf", "Removes file from TreeFS: -rmf/--remove-file <path>", cmd_removeFile},
  {"--change-directory", "-cd", "Changes relative directory: -cd/--change-directory <path>", cmd_changeDir},
  {"--rename", "-rn", "Changes name of directory or file: -rn/--rename <path> <newName>", cmd_rename}
};

int cmd_help(void *pUser, int userSize) {
  printf("Usage tfe [OPTIONS]\n");

  for(int i = 0; i < sizeof(gCommands) / sizeof(gCommands[0]); i++) {
    printf("\t%s | %s\t-\t%s\n", gCommands[i].name, gCommands[i].shortName, gCommands[i].description);
  }

  return 0x7FFFFFFF;
}

int cmd_newFS(void *pUser, int userSize) {
  char **user = (char**)pUser;

  gModifiedTFSPartition = realloc(gModifiedTFSPartition, strlen(user[0]));
  strncpy(gModifiedTFSPartition, user[0], strlen(user[0]));

  if(!tfs_newFileSystem(&gTFS, atoi(user[1])))
    return 1;

  return 0;
}

int cmd_mount(void *pUser, int userSize) {
  char **user = (char**)pUser;

  gModifiedTFSPartition = realloc(gModifiedTFSPartition, strlen(user[0]));
  strncpy(gModifiedTFSPartition, user[0], strlen(user[0]));

  if(!tfs_mount(&gTFS))
    return 1;

  return 0;
}

int cmd_mkdir(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_uint32_t dirAddress = tfs_findAbsolutePath(&gTFS, absPath);

  free(absPath);

  if(!tfs_isDirectory(&gTFS, dirAddress)) {
    printf("Path does not describe directory\n");

    return 1;
  }

  tfs_createDirectory(&gTFS, dirAddress, user[1]);

  return 0;

  return 0;
}

int cmd_mkfile(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_uint32_t dirAddress = tfs_findAbsolutePath(&gTFS, absPath);

  free(absPath);

  if(!tfs_isDirectory(&gTFS, dirAddress)) {
    printf("Path does not describe directory\n");

    return 1;
  }

  tfs_createFile(&gTFS, dirAddress, user[1]);

  return 0;
}

int cmd_writeFile(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_uint32_t fileAddress = tfs_findAbsolutePath(&gTFS, absPath);

  free(absPath);

  if(!tfs_isFile(&gTFS, fileAddress)) {
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

  tfs_writeFile(&gTFS, fileAddress, data, len, 0);

  free(data);

  return 0;
}

int cmd_readFile(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_uint32_t fileAddress = tfs_findAbsolutePath(&gTFS, absPath);

  free(absPath);

  if(!tfs_isFile(&gTFS, fileAddress)) {
    printf("Cannot read directory as file!\n");

    return 1;
  }

  tfs_uint32_t fileSize = tfs_getFileSize(&gTFS, fileAddress);

  tfs_uint8_t *fileBuffer = calloc(fileSize + 1, sizeof(tfs_uint8_t));

  tfs_readFile(&gTFS, fileAddress, fileBuffer, fileSize, 0);

  FILE *pcFile = fopen(user[1], "wb+");
  fwrite(fileBuffer, sizeof(tfs_uint8_t), fileSize, pcFile);
  fclose(pcFile);

  free(fileBuffer);

  return 0;
}

int cmd_removeFile(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_uint32_t fileAddress = tfs_findAbsolutePath(&gTFS, absPath); 

  tfs_bool_t removed = tfs_removeFile(&gTFS, fileAddress);

  free(absPath);

  return removed ? 0 : 1;
}

int cmd_changeDir(void *pUser, int userSize) {
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

  if(tfs_isFile(&gTFS, tfs_findAbsolutePath(&gTFS, gCurrentDirectory))) {
    printf("Cannot change directory to file! Reverting back.\n");

    gCurrentDirectory = realloc(gCurrentDirectory, strlen(lastDir) + 1);
    strcpy(gCurrentDirectory, lastDir);
  }

  free(lastDir);

  return 0;
}

int cmd_rename(void *pUser, int userSize) {
  char **user = ((char**)pUser);

  char *absPath = malloc((user[0][0] != '/' ? strlen(gCurrentDirectory) : 0) + strlen(user[0]) + 1);
  if(user[0][0] != '/') {
    strcpy(absPath, gCurrentDirectory);
    strcat(absPath, "/");
  }

  strcat(absPath, user[0]);

  tfs_bool_t renamed = tfs_renameEntry(&gTFS, tfs_findAbsolutePath(&gTFS, absPath), user[1]);

  free(absPath);

  if(renamed)
    printf("File renamed to %s\n", user[1]);

  return renamed ? 0 : 1;
}

void writeHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE *file = fopen(gModifiedTFSPartition, "rb+");

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fwrite(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

void readHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE *file = fopen(gModifiedTFSPartition, "rb+");

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fread(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

int main(int argc, char **argv) {
  if(argc < 2) {
    printf("Usage: %s [OPTIONS]\n\tUse -h or --help for more info!\n", argv[0]);

    return 0;
  }

  gCurrentDirectory = realloc(gCurrentDirectory, strlen("/"));
  strcpy(gCurrentDirectory, "/");

  tfs_init(&gTFS, writeHandle, readHandle);

  for(int i = 0; i < argc; i++) {
    int status = cmd_run(
      argv[i], 
      (void*)(argv + i + 1), 
      cmd_sizeUntilNewCmd(&argv[i + 1], gCommands, sizeof(gCommands) / sizeof(gCommands[0])), 
      gCommands, 
      sizeof(gCommands) / sizeof(gCommands[0])); 
  
    if(status == 0x7FFFFFFF)
      break;
  }

  free(gModifiedTFSPartition);
  free(gCurrentDirectory);

  return 0;
}
