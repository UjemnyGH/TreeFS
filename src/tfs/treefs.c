#include "treefs.h"

void* _tfs_copyMemeory(void *const pDst, const void *const pSrc, tfs_uint32_t size) {
  for(tfs_uint32_t i = 0; i < size; i++)
    ((tfs_uint8_t*)pDst)[i] = ((tfs_uint8_t*)pSrc)[i];

  return pDst;
}

void* _tfs_setMemory(void *const pMem, tfs_uint8_t value, tfs_uint32_t size) {
  for(tfs_uint32_t i = 0; i < size; i++)
    ((tfs_uint8_t*)pMem)[i] = value;

  return pMem;
}

int _tfs_compareMemory(const void *const pMem1, const void *const pMem2, tfs_uint32_t size) {
  for(tfs_uint32_t i = 0; i < size; i++) {
    if(((tfs_uint8_t*)pMem1)[i] != ((tfs_uint8_t*)pMem2)[i])
      return ((tfs_uint8_t*)pMem1)[i] - ((tfs_uint8_t*)pMem2)[i];
  }

  return 0;
}

tfs_uint32_t _tfs_lengthString(const char *str) {
  tfs_uint32_t len = 0;

  while(str[len])
    len++;

  return len;
}

tfs_uint32_t _tfs_calcChcksum(const tfs_uint8_t *const data, tfs_uint32_t size) {
  tfs_uint32_t sum = 0;

  for(tfs_uint32_t i = 0; i < size; i++)
    sum ^= ((tfs_uint32_t)data[i]) << ((i % 4) * 8);

  return sum;
}

void _tfs_clearHeap() {
  for(tfs_uint32_t i = 0; i < TFS_BLOCK_SIZE / sizeof(tfs_uint32_t); i++)
    ((tfs_uint32_t*)gBlockHeap)[i] = 0;
}

tfs_uint32_t _tfs_getHeapHeaderChecksum() {
  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  if((header->flags & tfs_header_flags_headerChecksum) && !(header->flags & tfs_header_flags_blockChecksum))
    return _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), sizeof(*header) - sizeof(tfs_uint32_t));
  else if(header->flags & tfs_header_flags_blockChecksum)
    return _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), TFS_BLOCK_SIZE - sizeof(tfs_uint32_t));

  return 0;
}

void _tfs_resolveHeapHeaderChecksum() {
  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  header->checksum = 0;

  if((header->flags & tfs_header_flags_headerChecksum) && !(header->flags & tfs_header_flags_blockChecksum))
    header->checksum = _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), sizeof(*header) - sizeof(tfs_uint32_t));
  else if(header->flags & tfs_header_flags_blockChecksum)
    header->checksum = _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), TFS_BLOCK_SIZE - sizeof(tfs_uint32_t));
}

void _tfs_resolveChecksum(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  if(blockAddress == 0)
    return;

  _tfs_clearHeap();

  if(blockAddress == TFS_FS_INFO_BLOCK) { 
    pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

    tfs_fileSystemInfo_t *fsi = ((tfs_fileSystemInfo_t*)gBlockHeap);

    fsi->checksum = _tfs_calcChcksum(gBlockHeap + (2 * sizeof(tfs_uint32_t)), TFS_BLOCK_SIZE - (2 * sizeof(tfs_uint32_t)));
    fsi->checksumXored = fsi->checksum ^ TFS_UINT32_MAX;

    pTfs->write(gBlockHeap, TFS_FS_INFO_BLOCK);

    return;
  }

  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  header->checksum = 0;

  if((header->flags & tfs_header_flags_headerChecksum) && !(header->flags & tfs_header_flags_blockChecksum))
    header->checksum = _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), sizeof(*header) - sizeof(tfs_uint32_t));
  else if(header->flags & tfs_header_flags_blockChecksum)
    header->checksum = _tfs_calcChcksum(gBlockHeap + sizeof(tfs_uint32_t), TFS_BLOCK_SIZE - sizeof(tfs_uint32_t));

  pTfs->write(gBlockHeap, blockAddress);
}

tfs_uint32_t _tfs_findFirstFreeBlock(tfs_t *const pTfs) {
  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

  tfs_fileSystemInfo_t *fsInfo = ((tfs_fileSystemInfo_t*)gBlockHeap);
  const tfs_uint32_t blocksCount = fsInfo->blocksCount;
  const tfs_uint32_t blockBitmapSize = TFS_BLOCK_SIZE - sizeof(tfs_header_t);
  const tfs_uint32_t blockBitmapCount = blocksCount / blockBitmapSize;
  
  // Read all block bitmaps, one by one 
  for(tfs_uint32_t i = 0; i < blockBitmapCount; i++) {
    pTfs->read(gBlockHeap, TFS_FIRST_DATA_BLOCK + (i * blockBitmapSize * 8));

    // Read all the bits in that block bitmap, to reduce initiali overhand, use 32 bits instead of 8
    for(tfs_uint32_t j = 0; j < blockBitmapSize; j++) {
      if(((tfs_uint8_t*)gBlockHeap)[j + sizeof(tfs_header_t)] == 0xFF)
        continue;

      // Find first free block (0 bit) and return calculated block address
      for(tfs_uint8_t b = 0; b < 8; b++) {
        if(!(((tfs_uint8_t*)gBlockHeap)[j + sizeof(tfs_header_t)] & (1 << b))) {
          const tfs_uint32_t potentialBlock = TFS_FIRST_DATA_BLOCK + (i * blockBitmapSize * 8) + j * 8 + b;

          // NO FIXING HERE, IT BRAKES FORMATTING!!!
          //pTfs->read(gBlockHeap, potentialBlock);
          //const tfs_uint32_t blockFlags = ((tfs_header_t*)gBlockHeap)->flags;

          // Check is that block is really free
          //if(!(blockFlags & (tfs_header_flags_file | tfs_header_flags_directory | tfs_header_flags_usedBlocksBitmap)))
            return potentialBlock;

          /*pTfs->read(gBlockHeap, TFS_FIRST_DATA_BLOCK + (i * blockBitmapSize * 8));

          // If block was not free, repair bitmap
          if(blockFlags & (tfs_header_flags_file | tfs_header_flags_directory | tfs_header_flags_usedBlocksBitmap)) {
            gBlockHeap[sizeof(tfs_header_t) + j] |= (1 << b);
            
            _tfs_resolveHeapHeaderChecksum();
            pTfs->write(gBlockHeap, TFS_FIRST_DATA_BLOCK + (i * blockBitmapSize * 8));
          }*/
        }
      }
    }
  }

  return 0;
}

tfs_bool_t _tfs_allocateBlock(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  if(blockAddress < TFS_FIRST_DATA_BLOCK)
    return tfs_false;

  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

  tfs_fileSystemInfo_t *fsInfo = ((tfs_fileSystemInfo_t*)gBlockHeap);
  const tfs_uint32_t blockBitmapSize = TFS_BLOCK_SIZE - sizeof(tfs_header_t);
  const tfs_uint32_t blockBitmapCount = fsInfo->blocksCount / blockBitmapSize;
  const tfs_uint32_t blockBitmap = (blockAddress - TFS_FS_INFO_BLOCK) / (blockBitmapSize * 8);
  const tfs_uint32_t blockBitOffset = (blockAddress - TFS_FIRST_DATA_BLOCK) % (blockBitmapSize * 8);

  if(blockBitmap >= blockBitmapCount)
    return tfs_false;

  pTfs->read(gBlockHeap, TFS_FIRST_DATA_BLOCK + (blockBitmap * blockBitmapSize));

  if(!(((tfs_header_t*)gBlockHeap)->flags & tfs_header_flags_usedBlocksBitmap))
    return tfs_false;

  gBlockHeap[sizeof(tfs_header_t) + (blockBitOffset / 8)] |= (1 << (blockBitOffset % 8));

  _tfs_resolveHeapHeaderChecksum();
  pTfs->write(gBlockHeap, TFS_FIRST_DATA_BLOCK + (blockBitmap * blockBitmapSize));

  return tfs_true;
}

tfs_bool_t _tfs_freeBlock(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  if(blockAddress < TFS_FIRST_DATA_BLOCK)
    return tfs_false;

  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

  tfs_fileSystemInfo_t *fsInfo = ((tfs_fileSystemInfo_t*)gBlockHeap);
  const tfs_uint32_t blockBitmapSize = TFS_BLOCK_SIZE - sizeof(tfs_header_t);
  const tfs_uint32_t blockBitmapCount = fsInfo->blocksCount / blockBitmapSize;
  const tfs_uint32_t blockBitmap = (blockAddress - TFS_FS_INFO_BLOCK) / (blockBitmapSize * 8);
  const tfs_uint32_t blockBitOffset = (blockAddress - TFS_FIRST_DATA_BLOCK) % (blockBitmapSize * 8);

  if(blockBitmap >= blockBitmapCount)
    return tfs_false;

  pTfs->read(gBlockHeap, TFS_FIRST_DATA_BLOCK + (blockBitmap * blockBitmapSize));

  if(!(((tfs_header_t*)gBlockHeap)->flags & tfs_header_flags_usedBlocksBitmap))
    return tfs_false;

  gBlockHeap[sizeof(tfs_header_t) + (blockBitOffset / 8)] &= ~(1 << (blockBitOffset % 8));

  _tfs_resolveHeapHeaderChecksum();
  pTfs->write(gBlockHeap, TFS_FIRST_DATA_BLOCK + (blockBitmap * blockBitmapSize));

  return tfs_true;
}

tfs_bool_t _tfs_makeRootDirectory(tfs_t *const pTfs) {
  tfs_uint32_t rootDirAddr = _tfs_findFirstFreeBlock(pTfs);
  
  if(!rootDirAddr)
    return tfs_false;

  tfs_bool_t allocState = _tfs_allocateBlock(pTfs, rootDirAddr);

  if(!allocState)
    return tfs_false;

  pTfs->read(gBlockHeap, rootDirAddr);
  _tfs_clearHeap();

  tfs_header_t *root = ((tfs_header_t*)gBlockHeap);
  root->flags = tfs_header_flags_directory | tfs_header_flags_blockChecksum;
  root->parentBlock = rootDirAddr;
  root->nextBlock = 0;
  root->entryOffset = 0;
  root->entriesInBlock = 2;

  tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(*root)));
  dir[0].type = tfs_directoryEntry_type_directory;
  dir[0].blockAddress = rootDirAddr;
  dir[0].nameLength = 1;
  dir[0].name[0] = '.';

  dir[1].type = tfs_directoryEntry_type_directory;
  dir[1].blockAddress = rootDirAddr;
  dir[1].nameLength = 2;
  dir[1].name[0] = '.';
  dir[1].name[1] = '.';

  _tfs_resolveHeapHeaderChecksum();

  pTfs->write(gBlockHeap, rootDirAddr);

  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);
  ((tfs_fileSystemInfo_t*)gBlockHeap)->rootDirectoryAddress = rootDirAddr;
  pTfs->write(gBlockHeap, TFS_FS_INFO_BLOCK);
  _tfs_resolveChecksum(pTfs, TFS_FS_INFO_BLOCK);

  return tfs_true;
}

/**
 * Creates new directory under parent directory, returns new directiory block address or root address on error
 */
tfs_uint32_t _tfs_createFileOrDir(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name, tfs_bool_t createFile, tfs_checksum_t usedChecksum) {
  // NOTE: There could be collision/duplicates resolving, but current code picks first entry as a used one and calcualtes offsets based on that, every duplicate
  // is added on the bottom and is ignored by code, not that harmless, for now I leave it as is

  // Calculate max entites per block
  const tfs_uint32_t maxEntriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);

  // Set current working block to parent 
  tfs_uint32_t currentBlock = parentDirectoryBlockAddress;

  // Read current working block
  pTfs->read(gBlockHeap, currentBlock);

  // Setup block header pointer
  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  const tfs_uint32_t parentChecksumFlags = header->flags & (tfs_header_flags_blockChecksum | tfs_header_flags_headerChecksum);
  // Save entry offset for later possible use
  const tfs_uint32_t parentEntryOffset = header->entryOffset;
  // Set current entry offset to 0
  tfs_uint32_t currentEntryOffset = 0;

  // Check for correctness
  if(!(header->flags & tfs_header_flags_directory) || (header->flags & tfs_header_flags_continuation))
    return pTfs->rootDirBlock;

  if(header->checksum != _tfs_getHeapHeaderChecksum())
    return pTfs->rootDirBlock;

  // Iterate until there is free space for new directory
  while(header->entriesInBlock >= maxEntriesPerBlock && header->nextBlock) {
    // Update currennt working block
    currentBlock = header->nextBlock;
    // Update current entry offset
    currentEntryOffset += maxEntriesPerBlock;
    pTfs->read(gBlockHeap, currentBlock);

    if(!(header->flags & (tfs_header_flags_directory | tfs_header_flags_continuation)))
      return pTfs->rootDirBlock;

    if(header->checksum != _tfs_getHeapHeaderChecksum())
      return pTfs->rootDirBlock;
  }

  // Check if we dont have space for new entry in current block
  if(header->entriesInBlock >= maxEntriesPerBlock) {
    // Update current entry offset
    currentEntryOffset += maxEntriesPerBlock;

    // Allocate new block with safety checks
    tfs_uint32_t nextBlockAddr = _tfs_findFirstFreeBlock(pTfs);

    if(!nextBlockAddr)
      return pTfs->rootDirBlock;

    tfs_bool_t nextBlockAllocated = _tfs_allocateBlock(pTfs, nextBlockAddr);

    if(!nextBlockAllocated)
      return pTfs->rootDirBlock;

    pTfs->read(gBlockHeap, currentBlock);
    // Set that new block as next block
    header->nextBlock = nextBlockAddr;
    // Recompute checksum
    _tfs_resolveHeapHeaderChecksum();
    // Write current block as is
    pTfs->write(gBlockHeap, currentBlock);
    // Save parent for next block and update current to new block
    const tfs_uint32_t parent = currentBlock;
    currentBlock = header->nextBlock;

    // CLear heap to clear entire block
    _tfs_clearHeap();

    // Set necessary flags for new block and parent
    header->flags |= tfs_header_flags_directory | tfs_header_flags_continuation | parentChecksumFlags;
    header->nextBlock = 0;
    header->parentBlock = parent;
    header->entriesInBlock = 0;
    header->entryOffset = parentEntryOffset;
  }

  for(tfs_uint32_t i = 0; i < maxEntriesPerBlock; i++) {
    tfs_directoryEntry_t *dir = (tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t));

    if(dir[i].nameLength == 0 && dir[i].type == tfs_directoryEntry_type_free) {
      // Change free dir to be occupied by new dir info, add entry to that block and save it
      header->entriesInBlock++;
      dir[i].type = (createFile ? tfs_directoryEntry_type_file : tfs_directoryEntry_type_directory);
      dir[i].nameLength = (_tfs_lengthString(name) > 32 ? 32 : _tfs_lengthString(name));
      _tfs_copyMemeory(dir[i].name, name, dir[i].nameLength);
      pTfs->write(gBlockHeap, currentBlock);
      
      // Allocate free memory block, this uses global block heap, thats why we saved before
      tfs_uint32_t newDirBlockAddr = _tfs_findFirstFreeBlock(pTfs);

      if(!newDirBlockAddr)
        return pTfs->rootDirBlock;

      tfs_bool_t newDirBlockAllocated = _tfs_allocateBlock(pTfs, newDirBlockAddr);

      if(!newDirBlockAllocated)
        return pTfs->rootDirBlock;

      // Read what we saved and update directory block pointer, recalc checksum and save it
      pTfs->read(gBlockHeap, currentBlock);
      dir[i].blockAddress = newDirBlockAddr;
      _tfs_resolveHeapHeaderChecksum();
      pTfs->write(gBlockHeap, currentBlock);

      // Clear heap for new directory
      _tfs_clearHeap();

      // Fill header with directory info
      header->parentBlock = parentDirectoryBlockAddress;
      header->nextBlock = 0;
      header->entryOffset = currentEntryOffset + i;
      header->flags = (createFile ? tfs_header_flags_file : tfs_header_flags_directory) | (usedChecksum << 3);
      
      if(!createFile)
        header->entriesInBlock = 2;
    
      if(!createFile) {
        // Fill directory self and parent pointers with info
        dir[0].blockAddress = newDirBlockAddr;
        dir[0].type = tfs_directoryEntry_type_directory;
        dir[0].nameLength = 1;
        dir[0].name[0] = '.';

        dir[1].blockAddress = parentDirectoryBlockAddress;
        dir[1].type = tfs_directoryEntry_type_directory;
        dir[1].nameLength = 2;
        dir[1].name[0] = '.';
        dir[1].name[1] = '.';
      }

      // Calculate block checksum
      _tfs_resolveHeapHeaderChecksum();

      // Then save block
      pTfs->write(gBlockHeap, newDirBlockAddr);

      // And return new directory block address
      return newDirBlockAddr;
    }
  }

  return pTfs->rootDirBlock;
}

void _tfs_removeAssociatedBlocks(tfs_t *const pTfs, tfs_uint32_t topMostBlockAddress) { 
  if(topMostBlockAddress == pTfs->rootDirBlock || topMostBlockAddress == TFS_FS_INFO_BLOCK || topMostBlockAddress == 0)
    return;

  tfs_uint32_t current = topMostBlockAddress;

  // Remove all blocks associated with that file
  while(current) {
    if(current)
      pTfs->read(gBlockHeap, current);

    tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

    header->parentBlock = 0;
    header->flags = tfs_header_flags_free;
    header->entryOffset = 0;
    header->checksum = 0;
    header->entriesInBlock = 0;
    header->sizeLow = 0;
    header->sizeHigh = 0;

    tfs_uint32_t tempCurrent = current;

    current = header->nextBlock;
    // We can remove next block, that would delete possibility for recovery
    //header->nextBlock = 0;

    pTfs->write(gBlockHeap, tempCurrent);

    _tfs_freeBlock(pTfs, tempCurrent); 
  }
}

tfs_result_t _tfs_removeBlockDirectoryEntry(tfs_t *const pTfs, tfs_uint32_t topMostBlockAddress) {
  if(topMostBlockAddress == pTfs->rootDirBlock || topMostBlockAddress == TFS_FS_INFO_BLOCK || topMostBlockAddress == 0)
    return tfs_result_success;

  // Read first file block
  pTfs->read(gBlockHeap, topMostBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  const tfs_uint32_t offset = header->entryOffset;
  const tfs_uint32_t entriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);
  const tfs_uint32_t skippedBlocks = offset / entriesPerBlock;
  const tfs_uint32_t entryOffset = offset % entriesPerBlock;

  tfs_uint32_t entryBlockAddr = header->parentBlock;

  // Read parent of first file block (directory block should be as parent block)
  if(entryBlockAddr) {
    pTfs->read(gBlockHeap, entryBlockAddr);

    // Skip to entry
    for(tfs_uint32_t i = 0; i < skippedBlocks; i++) {
      if(!header->nextBlock)
        return tfs_result_nextBlockMissing;

      entryBlockAddr = header->nextBlock;
      pTfs->read(gBlockHeap, entryBlockAddr);
    }

    // Remove entry
    tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));
    if(dir[entryOffset].blockAddress != topMostBlockAddress)
      return tfs_result_addressMismatch;

    header->entriesInBlock--;
    dir[entryOffset].blockAddress = 0;
    dir[entryOffset].nameLength = 0;
    dir[entryOffset].type = tfs_directoryEntry_type_free;
    _tfs_setMemory(dir[entryOffset].name, 0, TFS_FILENAME_SIZE);

    _tfs_resolveHeapHeaderChecksum();

    pTfs->write(gBlockHeap, entryBlockAddr);
  }

  return tfs_result_success;
}

tfs_bool_t _tfs_isSelfEntry(tfs_directoryEntry_t *const pDir) {
  return (pDir->nameLength == 1 && pDir->name[0] == '.') ? tfs_true : tfs_false;
}

tfs_bool_t _tfs_isParentEntry(tfs_directoryEntry_t *const pDir) {
  return (pDir->nameLength == 2 && pDir->name[0] == '.' && pDir->name[1] == '.') ? tfs_true : tfs_false;
}

tfs_bool_t tfs_isDirectory(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  return (header->flags & tfs_header_flags_directory ? tfs_true : tfs_false);
}

tfs_bool_t tfs_isFile(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  return (header->flags & tfs_header_flags_file ? tfs_true : tfs_false);
}

tfs_bool_t tfs_isContinuation(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  return (header->flags & tfs_header_flags_continuation ? tfs_true : tfs_false);
}

tfs_bool_t tfs_isDirectoryEmpty(tfs_t *const pTfs, tfs_uint32_t directoryBlockAddress) {
  pTfs->read(gBlockHeap, directoryBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  // Check if it is even directory
  if(!(header->flags & tfs_header_flags_directory))
    return tfs_false;

  tfs_bool_t directoryEmpty = tfs_true;
  tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));

  // Check all directory blocks
  do {
    if(header->entriesInBlock > 2 
      || (!_tfs_isSelfEntry(&dir[0]) && dir[0].type != tfs_directoryEntry_type_free) 
      || (!_tfs_isParentEntry(&dir[1]) && dir[1].type != tfs_directoryEntry_type_free)) {
      directoryEmpty = tfs_false;
      
      break;
    }

    if(header->nextBlock)
      pTfs->read(gBlockHeap, header->nextBlock);
  } while(header->nextBlock);

  return directoryEmpty;
}

tfs_uint32_t tfs_getFileSize(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress) {
  pTfs->read(gBlockHeap, fileBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  return header->sizeLow;
}

void tfs_init(tfs_t *const pTfs, PFN_tfsRWCallback write, PFN_tfsRWCallback read) {
  pTfs->write = write;
  pTfs->read = read;
}

tfs_bool_t tfs_newFileSystem(tfs_t *const pTfs, tfs_uint32_t deviceSizekB) {
  _tfs_clearHeap();

  tfs_fileSystemInfo_t* fsi = ((tfs_fileSystemInfo_t*)gBlockHeap);
  fsi->blocksCount = deviceSizekB / (TFS_BLOCK_SIZE / TFS_1KB);
  fsi->magic = TFS_MAGIC;
  fsi->magicXored = TFS_MAGIC ^ TFS_UINT16_MAX;
  fsi->checksum = _tfs_calcChcksum(gBlockHeap + (2 * sizeof(tfs_uint32_t)), TFS_BLOCK_SIZE - (2 * sizeof(tfs_uint32_t)));
  fsi->checksumXored = fsi->checksum ^ TFS_UINT32_MAX;

  pTfs->write(gBlockHeap, TFS_FS_INFO_BLOCK);

  _tfs_clearHeap(); 

  const tfs_uint32_t blocksPerBitmap = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) * 8;
  const tfs_uint32_t bitmapBlocksCount = (deviceSizekB / (TFS_BLOCK_SIZE / TFS_1KB)) / blocksPerBitmap;
  
  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);
  header->checksum = 0;
  header->flags = 0;
  header->sizeLow = 0;
  header->sizeHigh = 0;
  header->parentBlock = 0;
  header->nextBlock = 0;
  header->entryOffset = 0;
  header->entriesInBlock = 0;
  header->flags = tfs_header_flags_usedBlocksBitmap | tfs_header_flags_blockChecksum;
  gBlockHeap[sizeof(*header)] = (1 << 0);
    
  _tfs_resolveHeapHeaderChecksum();

  for(tfs_uint32_t i = 0; i < bitmapBlocksCount; i++) { 
    // We want to fit every single block below in that bitmap, speeds up allocation process
    pTfs->write(gBlockHeap, TFS_FIRST_DATA_BLOCK + (i * blocksPerBitmap));
  }

  tfs_bool_t rootStatus = _tfs_makeRootDirectory(pTfs);

  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

  pTfs->rootDirBlock = ((tfs_fileSystemInfo_t*)gBlockHeap)->rootDirectoryAddress;
  
  return rootStatus;
}

tfs_bool_t tfs_mount(tfs_t *const pTfs) {
  pTfs->read(gBlockHeap, TFS_FS_INFO_BLOCK);

  tfs_fileSystemInfo_t *fsInfo = ((tfs_fileSystemInfo_t*)gBlockHeap);

  if(fsInfo->magic != TFS_MAGIC || (fsInfo->magic ^ TFS_UINT16_MAX) != fsInfo->magicXored)
    return tfs_false;

  if(fsInfo->checksum != _tfs_calcChcksum(gBlockHeap + (2 * sizeof(tfs_uint32_t)), TFS_BLOCK_SIZE - (2 * sizeof(tfs_uint32_t))) 
    || (fsInfo->checksum ^ TFS_UINT32_MAX) != fsInfo->checksumXored)
    return tfs_false;

  pTfs->rootDirBlock = fsInfo->rootDirectoryAddress;

  return tfs_true;
}

tfs_uint32_t tfs_findInDirectory(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name) {
  // Calculate max entries per block
  const tfs_uint32_t maxEntriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);

  // Read parent block
  pTfs->read(gBlockHeap, parentDirectoryBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  // Check if block is correct
  if(!(header->flags & tfs_header_flags_directory))
    return pTfs->rootDirBlock;

  if(header->checksum != _tfs_getHeapHeaderChecksum())
    return pTfs->rootDirBlock;

  do {
    // Iterate through all entries in block
    for(tfs_uint32_t i = 0; i < maxEntriesPerBlock; i++) {
      tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t) + (i * sizeof(tfs_directoryEntry_t))));

      if(dir->type == tfs_directoryEntry_type_free || dir->blockAddress == 0 || dir->nameLength == 0)
        continue;

      // Check if entry was found
      if(_tfs_lengthString(name) == dir->nameLength) {
        if(_tfs_compareMemory(name, dir->name, dir->nameLength) == 0)
          return dir->blockAddress;
      }
    }

    // If directory have next block, go to it
    if(header->nextBlock)
      pTfs->read(gBlockHeap, header->nextBlock);

    // If it is not directory continuation, stop
    if((header->flags & (tfs_header_flags_directory | tfs_header_flags_continuation)) != (tfs_header_flags_directory | tfs_header_flags_continuation))
      return pTfs->rootDirBlock;

    // Checksums mush match
    if(header->checksum != _tfs_getHeapHeaderChecksum())
      return pTfs->rootDirBlock;
  } while(header->nextBlock);

  return pTfs->rootDirBlock;
}

tfs_uint32_t tfs_findAbsolutePath(tfs_t *const pTfs, const char *path) {
  if(!path || path[0] == '\0')
    return pTfs->rootDirBlock;

  const char* p = path;
  char entryName[TFS_FILENAME_SIZE];
  tfs_uint32_t current = pTfs->rootDirBlock;

  while(*p == '/')
    ++p;

  while(*p) {
    tfs_uint8_t entryNameLen = 0;

    while(*p && *p != '/' && entryNameLen < sizeof(entryName))
      entryName[entryNameLen++] = *p++;

    entryName[entryNameLen] = '\0';

    while(*p == '/')
      ++p;

    if(entryNameLen == 0)
      break;

    if(entryNameLen == 1 && entryName[0] == '.')
      continue;

    current = tfs_findInDirectory(pTfs, current, entryName);
  }

  return current;
}

tfs_uint32_t tfs_listDirectory(tfs_t *const pTfs, const tfs_uint32_t directoryBlockAddress, char *namesOut, tfs_uint32_t maxNames) {
  pTfs->read(gBlockHeap, directoryBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  const tfs_uint32_t entriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);
  tfs_uint32_t current = directoryBlockAddress;
  tfs_uint32_t readNamesCount = 0;

  while(current && readNamesCount < maxNames) {
    if(current)
      pTfs->read(gBlockHeap, current);

    tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));

    for(tfs_uint32_t i = 0; i < entriesPerBlock && readNamesCount < maxNames; i++) {
      if(dir[i].nameLength > 0) {
        _tfs_copyMemeory(namesOut + (readNamesCount * TFS_FILENAME_SIZE), dir[i].name, dir[i].nameLength);

        readNamesCount++;
      }
    }

    current = header->nextBlock;
  }

  return readNamesCount;
}

// Returns block address of dir or file
tfs_uint32_t tfs_createDirectory(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name) {
  return _tfs_createFileOrDir(pTfs, parentDirectoryBlockAddress, name, tfs_false, tfs_headerChecksum);
}

tfs_uint32_t tfs_createFile(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name) {
  return _tfs_createFileOrDir(pTfs, parentDirectoryBlockAddress, name, tfs_true, tfs_headerChecksum);
}

// Returns read bytes
tfs_uint32_t tfs_readFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress, void *const buffer, tfs_uint32_t size, tfs_uint32_t offset) {
  pTfs->read(gBlockHeap, fileBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  if(!(header->flags & tfs_header_flags_file))
    return 0;

  if(header->checksum != _tfs_getHeapHeaderChecksum())
    return 0;

  if(header->sizeLow < size - offset)
    size = header->sizeLow - offset;

  const tfs_uint32_t dataPerBlock = TFS_BLOCK_SIZE - sizeof(tfs_header_t);
  const tfs_uint32_t blocksToSkip = offset / dataPerBlock;
  // We want to read at least 1 block
  const tfs_uint32_t blockCount = (size / dataPerBlock) + 1;
  const tfs_uint32_t startByteOffset = offset % dataPerBlock;

  tfs_uint32_t current = fileBlockAddress;
  tfs_uint32_t bytesRead = 0;

  // Skip blocks to desired offset
  for(tfs_uint32_t i = 0; i < blocksToSkip; i++) {
    if(!header->nextBlock)
      return 0;

    current = header->nextBlock;
    pTfs->read(gBlockHeap, current);

    if((header->flags & (tfs_header_flags_file | tfs_header_flags_continuation)) != (tfs_header_flags_file | tfs_header_flags_continuation))
      return 0;
  }

  // Read all desired blocks
  for(tfs_uint32_t i = 0; i < blockCount; i++) {
    // Calculate offset and size of read bytes
    tfs_uint32_t copyOffset = (i == 0 ? startByteOffset : 0);
    tfs_uint32_t copySize = ((size - bytesRead) > dataPerBlock ? (dataPerBlock - copyOffset) : (size - bytesRead - copyOffset));

    _tfs_copyMemeory(buffer + bytesRead, (void*)(gBlockHeap + sizeof(tfs_header_t) + copyOffset), copySize);

    // Add size to total
    bytesRead += copySize;

    // Check if we are below last read block, if yes, then it should contain next block address 
    if(i < (blockCount - 1) && !header->nextBlock)
      return bytesRead;

    // Set next block as current and read it
    current = header->nextBlock;
    pTfs->read(gBlockHeap, current);
  }

  return bytesRead;
}

tfs_uint32_t tfs_writeFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress, const void *const data, tfs_uint32_t size, tfs_uint32_t offset) {
  pTfs->read(gBlockHeap, fileBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  if(!(header->flags & tfs_header_flags_file))
    return 0;

  if(header->checksum != _tfs_getHeapHeaderChecksum())
    return 0;

  const tfs_uint32_t parentChecksumFlags = header->flags & (tfs_header_flags_blockChecksum | tfs_header_flags_headerChecksum);

  const tfs_uint32_t dataPerBlock = TFS_BLOCK_SIZE - sizeof(tfs_header_t);
  const tfs_uint32_t blocksToSkip = offset / dataPerBlock;
  // We need at least 1 block
  const tfs_uint32_t blockCount = (size / dataPerBlock) + 1;
  const tfs_uint32_t startByteOffset = offset % dataPerBlock;

  tfs_uint32_t current = fileBlockAddress;
  tfs_uint32_t bytesWrote = 0;

  // Skip blocks, if block is not allocated, allocate it, can contain garbage
  for(tfs_uint32_t i = 0; i < blocksToSkip; i++) {
    pTfs->read(gBlockHeap, current);

    tfs_uint32_t newBlockAddr = header->nextBlock;

    if(!newBlockAddr) {
      newBlockAddr = _tfs_findFirstFreeBlock(pTfs);

      if(!newBlockAddr)
        return 0;

      if(!_tfs_allocateBlock(pTfs, newBlockAddr))
        return 0;

      pTfs->read(gBlockHeap, newBlockAddr);
      
      header->flags = tfs_header_flags_file | tfs_header_flags_continuation | parentChecksumFlags;
      // Entry offset is only necessary in top most parent
      header->entryOffset = 0;
      header->parentBlock = current;
      header->nextBlock = 0;
      header->checksum = 0;
      header->entriesInBlock = 0;
      header->sizeLow = 0;
      header->sizeHigh = 0;

      _tfs_resolveHeapHeaderChecksum();
      pTfs->write(gBlockHeap, newBlockAddr);

      // Modify parent block
      pTfs->read(gBlockHeap, current);
      header->nextBlock = newBlockAddr;
      _tfs_resolveHeapHeaderChecksum();
      pTfs->write(gBlockHeap, current);
    }

    current = newBlockAddr;
  }

  // Write all desired blocks
  for(tfs_uint32_t i = 0; i < blockCount; i++) {
    pTfs->read(gBlockHeap, current);

    // Calculate offset and size of read bytes
    tfs_uint32_t copyOffset = (i == 0 ? startByteOffset : 0);
    tfs_uint32_t copySize = ((size - bytesWrote) > dataPerBlock ? (dataPerBlock - copyOffset) : (size - bytesWrote - copyOffset));

    _tfs_copyMemeory((void*)(gBlockHeap + sizeof(tfs_header_t) + copyOffset), data + bytesWrote, copySize);
    
    _tfs_resolveHeapHeaderChecksum();
    pTfs->write(gBlockHeap, current);

    // Add size to total
    bytesWrote += copySize;

    tfs_uint32_t newBlockAddr = header->nextBlock;

    // Is it is not last block and next block doesnt exist, allocate it
    if(i < (blockCount - 1) && !newBlockAddr) {
      newBlockAddr = _tfs_findFirstFreeBlock(pTfs);

      if(!newBlockAddr)
        break;

      if(!_tfs_allocateBlock(pTfs, newBlockAddr))
        break;

      pTfs->read(gBlockHeap, newBlockAddr);
      
      header->flags = tfs_header_flags_file | tfs_header_flags_continuation | parentChecksumFlags;
      // Entry offset is only necessary in top most parent
      header->entryOffset = 0;
      header->parentBlock = current;
      header->nextBlock = 0;
      header->checksum = 0;
      header->entriesInBlock = 0;
      header->sizeLow = 0;
      header->sizeHigh = 0;

      _tfs_resolveHeapHeaderChecksum();
      pTfs->write(gBlockHeap, newBlockAddr);
   
      // Modify parent block
      pTfs->read(gBlockHeap, current);
      header->nextBlock = newBlockAddr;
      _tfs_resolveHeapHeaderChecksum();
      pTfs->write(gBlockHeap, current);
    }

    current = newBlockAddr;
  }

  // Change size of file
  pTfs->read(gBlockHeap, current);

  if(!header->nextBlock) {
    _tfs_clearHeap();
    pTfs->read(gBlockHeap, fileBlockAddress);
    header->sizeLow = (blocksToSkip * dataPerBlock) + size + (bytesWrote % size);
    _tfs_resolveHeapHeaderChecksum();
    pTfs->write(gBlockHeap, fileBlockAddress);
  }

  return bytesWrote;
}

tfs_bool_t tfs_renameEntry(tfs_t *const pTfs, tfs_uint32_t blockAddress, const char *newName) {
  // Read block
  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  // Check if it can have name
  if(!(header->flags & (tfs_header_flags_file | tfs_header_flags_directory)))
    return tfs_false;

  // Check if it is not orphaned
  if(!header->parentBlock)
    return tfs_false;

  // Cannot change name of root as it does not have name
  if(header->parentBlock == pTfs->rootDirBlock && blockAddress == pTfs->rootDirBlock)
    return tfs_false;

  const tfs_uint32_t offset = header->entryOffset;
  const tfs_uint32_t entriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);
  const tfs_uint32_t skippedBlocks = offset / entriesPerBlock;
  const tfs_uint32_t entryOffset = offset % entriesPerBlock;

  tfs_uint32_t currentBlock = header->parentBlock;

  // Read parent
  pTfs->read(gBlockHeap, currentBlock);

  // Skip unnecessay blocks
  for(int i = 0; i < skippedBlocks; i++) {
    if(!header->nextBlock)
      return tfs_false;
    
    currentBlock = header->nextBlock;
    pTfs->read(gBlockHeap, currentBlock);
  }

  tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));

  // Change entry name
  dir[entryOffset].nameLength = _tfs_lengthString(newName);
  _tfs_setMemory(dir[entryOffset].name, 0, sizeof(dir[entryOffset].name));
  _tfs_copyMemeory(dir[entryOffset].name, newName, dir[entryOffset].nameLength);

  _tfs_resolveHeapHeaderChecksum();
  pTfs->write(gBlockHeap, currentBlock);

  return tfs_true;
}

tfs_result_t tfs_removeFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress) {
  // Read first file block
  pTfs->read(gBlockHeap, fileBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  if(!(header->flags & tfs_header_flags_file))
    return tfs_result_wrongBlockType;

  tfs_result_t entryRemovalResult = _tfs_removeBlockDirectoryEntry(pTfs, fileBlockAddress);

  if(entryRemovalResult != tfs_result_success)
    return entryRemovalResult;

  _tfs_removeAssociatedBlocks(pTfs, fileBlockAddress); 

  return tfs_result_success;
}

tfs_result_t tfs_removeDirectory(tfs_t *const pTfs, tfs_uint32_t directoryBlockAddress) {
  if(directoryBlockAddress == pTfs->rootDirBlock || directoryBlockAddress == TFS_FS_INFO_BLOCK || directoryBlockAddress == 0)
    return tfs_result_wrongOperationOnRequiredBlock;

  if(!tfs_isDirectoryEmpty(pTfs, directoryBlockAddress))
    return tfs_result_directoryNotEmpty;

  pTfs->read(gBlockHeap, directoryBlockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  if(!(header->flags & tfs_header_flags_directory))
     return tfs_result_wrongBlockType;

  tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));

  // Remove self and parent entries if they exist, they are created always in the first block of a directory
  for(tfs_uint32_t i = 0; i < 2/*header->entriesInBlock*/; i++) {
    if((dir[i].nameLength == 2 && _tfs_compareMemory(dir[i].name, "..", 2) == 0) || (dir[i].nameLength == 1 && dir[i].name[0] == '.')) {
      dir[i].nameLength = 0;
      dir[i].blockAddress = 0;
      dir[i].type = tfs_directoryEntry_type_free;
      _tfs_setMemory(dir[i].name, 0, sizeof(dir[i].name));
    }
  }

  pTfs->write(gBlockHeap, directoryBlockAddress);

  if(!tfs_isDirectoryEmpty(pTfs, directoryBlockAddress))
    return tfs_result_directoryNotEmpty;

  tfs_result_t entryRemovalResult = _tfs_removeBlockDirectoryEntry(pTfs, directoryBlockAddress);

  if(entryRemovalResult != tfs_result_success)
    return entryRemovalResult;

  _tfs_removeAssociatedBlocks(pTfs, directoryBlockAddress);

  return tfs_result_success;
}

tfs_result_t tfs_remove(tfs_t *const pTfs, tfs_uint32_t blockAddress) {
  // If this is file (checked inside) try to remove it
  tfs_result_t fileRemovalResult = tfs_removeFile(pTfs, blockAddress);

  // Is file was removed sucessfully, skip rest
  if(fileRemovalResult == tfs_result_success)
    return tfs_result_success;

  pTfs->read(gBlockHeap, blockAddress);

  tfs_header_t *header = ((tfs_header_t*)gBlockHeap);

  // We know at this point that we are not dealing with file, so check is it a directory
  if(!(header->flags & tfs_header_flags_directory))
    return tfs_result_wrongBlockType;

  const tfs_uint32_t offset = header->entryOffset;
  const tfs_uint32_t entriesPerBlock = (TFS_BLOCK_SIZE - sizeof(tfs_header_t)) / sizeof(tfs_directoryEntry_t);
  const tfs_uint32_t skippedBlocks = offset / entriesPerBlock;
  const tfs_uint32_t entryOffset = offset % entriesPerBlock;

  // At this point it must be a directory, check is it empty, if yes, remove it and skip rest
  if(tfs_isDirectoryEmpty(pTfs, blockAddress))
    return tfs_removeDirectory(pTfs, blockAddress);

  // If directory is not empty, everything mys be done manually
  // So at first remove directory entry to orphan cocurrent blocks
  _tfs_removeBlockDirectoryEntry(pTfs, blockAddress);

  tfs_uint32_t current = blockAddress;

  // Set top most parent to 0 to finish loop
  pTfs->read(gBlockHeap, blockAddress);
  header->parentBlock = 0;
  pTfs->write(gBlockHeap, blockAddress);

  while(current) {
    pTfs->read(gBlockHeap, current);
    tfs_directoryEntry_t *dir = ((tfs_directoryEntry_t*)(gBlockHeap + sizeof(tfs_header_t)));

    tfs_uint32_t parent = header->parentBlock;

    for(tfs_uint32_t i = 0; i < entriesPerBlock; i++) {
      // If we find directory, move inside it and read it
      if(dir[i].type == tfs_directoryEntry_type_directory) {
        current = dir[i].blockAddress;

        if(tfs_removeDirectory(pTfs, current) == tfs_result_success)
          current = parent;

        break;
      }
      // If we find file, remove it
      else if(dir[i].type == tfs_directoryEntry_type_file) {
        tfs_removeFile(pTfs, dir[i].blockAddress);

        pTfs->read(gBlockHeap, current);
      }
    }
  }

  return tfs_result_success;
}
