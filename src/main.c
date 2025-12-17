#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tfs/treefs.h"
#include "tool_core.h"
#include "test.h"

void writeHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE *file = fopen(gModifiedTFSPartition, "rb+");

  if(!file) {
    printf("Cannot open %s\n", gModifiedTFSPartition);
    
    return;
  }

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fwrite(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

void readHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE *file = fopen(gModifiedTFSPartition, "rb+");

  if(!file) {
    printf("Cannot open %s\n", gModifiedTFSPartition);
    
    return;
  }

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fread(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

int main(int argc, char **argv) {
  /// TreeFS test function
  //test(); 
  //return 0;

  if(argc < 2) {
    printf("Usage: %s [OPTIONS]\n\tUse -h or --help for more info!\n", argv[0]);

    return 0;
  }
 
  tfs_t tfs = {0};
  tfs_init(&tfs, writeHandle, readHandle);

  gCurrentDirectory = realloc(gCurrentDirectory, strlen("/"));
  strcpy(gCurrentDirectory, "/");

  for(int i = 0; i < argc; i++) {
    int status = cmd_run(
      &tfs,
      argv[i], 
      argv + (i + 1), 
      cmd_sizeUntilNewCmd((i + 1 < argc) ? (void*)(argv + i + 1) : NULL, argc - i - 1, gCommands, sizeof(gCommands) / sizeof(gCommands[0])), 
      gCommands, 
      sizeof(gCommands) / sizeof(gCommands[0])); 
  
    if(status == 0x7FFFFFFF)
      break;

    if(status != 0)
      printf("[ERROR]: Something happen at %s, error code: %d\n", argv[i], status);
  }

  free(gModifiedTFSPartition);
  free(gCurrentDirectory);

  return 0;
}
