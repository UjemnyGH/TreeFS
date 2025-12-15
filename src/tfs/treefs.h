#ifndef _TREE_FILE_SYSTEM_
#define _TREE_FILE_SYSTEM_

typedef unsigned char   tfs_uint8_t;
typedef unsigned short  tfs_uint16_t;
typedef unsigned int    tfs_uint32_t;

typedef enum tfs_bool_e {
  tfs_false =   0,
  tfs_true =    1
} tfs_bool_t;

#define TFS_NULL                0
#define TFS_UINT8_MAX           0xFF
#define TFS_UINT16_MAX          0xFFFF
#define TFS_UINT32_MAX          0xFFFFFFFF
#define TFS_1KB                 1024

#define TFS_MAGIC               0x7fe0
#define TFS_BLOCK_SIZE          TFS_1KB
#define TFS_FILENAME_SIZE       32
#define TFS_FS_INFO_BLOCK       1
#define TFS_FIRST_DATA_BLOCK    2

typedef struct tfs_fileSystemInfo_s {
  tfs_uint32_t checksum;
  tfs_uint32_t checksumXored;
  tfs_uint32_t blocksCount;
  tfs_uint16_t magic;
  tfs_uint16_t magicXored;
  tfs_uint32_t rootDirectoryAddress;
} tfs_fileSystemInfo_t;

typedef struct tfs_header_s {
  tfs_uint32_t checksum;
  
  tfs_uint32_t flags;
  // Only for files, only parent/top most block store file size, blocks that have flag <tfs_header_flags_continuation> set doesnt use size fields
  tfs_uint32_t sizeLow;
  // Size high can be fully supported, but current implementation of reading and writting files doesn support it as I mainly targetted this library for
  // STM32. I would add full support for 64 bit systems
  tfs_uint32_t sizeHigh;

  // Root : parentBlock = self
  tfs_uint32_t parentBlock;
  tfs_uint32_t nextBlock;

  // Describes offset in size of tfs_directoryEntry_t to entry in parentBlock directory (must be a directory!!!), for files stored only in top most block header
  tfs_uint32_t entryOffset;
  tfs_uint32_t entriesInBlock;

  tfs_uint8_t RESERVED[32];
} tfs_header_t;

typedef struct tfs_directoryEntry_s {
  tfs_uint32_t blockAddress;
  tfs_uint16_t type;
  tfs_uint8_t nameLength;
  tfs_uint8_t name[TFS_FILENAME_SIZE];

  tfs_uint8_t RESERVED[9];
} tfs_directoryEntry_t;

typedef enum tfs_header_flags_e {
  tfs_header_flags_free = 0x0,
  tfs_header_flags_directory = 0x1,
  tfs_header_flags_file = 0x2,
  tfs_header_flags_continuation = 0x4,
  tfs_header_flags_blockChecksum = 0x8,
  tfs_header_flags_headerChecksum = 0x10,
  tfs_header_flags_usedBlocksBitmap = 0x20
} tfs_header_flags_t;

typedef enum tfs_directoryEntry_type_e {
  tfs_directoryEntry_type_free = 0,
  tfs_directoryEntry_type_file = 1,
  tfs_directoryEntry_type_directory = 2
} tfs_directoryEntry_type_t;

typedef void(*PFN_tfsRWCallback)(void *buffer, tfs_uint32_t blockAddress);

typedef struct tfs_s {
  PFN_tfsRWCallback write;
  PFN_tfsRWCallback read;
  tfs_uint32_t rootDirBlock;
} tfs_t;

typedef enum tfs_checksum_e {
  tfs_noChecksum = 0,
  tfs_blockChecksum = 1,
  tfs_headerChecksum = 2
} tfs_checksum_t;

/*
 * Return true if block is directory, false otherwise
 */
tfs_bool_t tfs_isDirectory(tfs_t *const pTfs, tfs_uint32_t blockAddress);

/*
 * Return true if block is file block, false otherwise
 */
tfs_bool_t tfs_isFile(tfs_t *const pTfs, tfs_uint32_t blockAddress);

/*
 * Return true if block is continuation of a block, false otherwise
 */
tfs_bool_t tfs_isContinuation(tfs_t *const pTfs, tfs_uint32_t blockAddress);

/*
 * Initialize integration layer for file system
 */
void tfs_init(tfs_t *const pTfs, PFN_tfsRWCallback write, PFN_tfsRWCallback read);

/*
 * Make new file system on drive described in init functions
 */
tfs_bool_t tfs_newFileSystem(tfs_t *const pTfs, tfs_uint32_t deviceSizekB);

/*
 * Mount file system
 */
tfs_bool_t tfs_mount(tfs_t *const pTfs);

/*
 * Find file or directory in directory\
 * return block address to dir/file or root if not found
 */
tfs_uint32_t tfs_findInDirectory(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name);

/*
 * Find directory or file starting from root
 * return block address to dir/file or root if not found
 */
tfs_uint32_t tfs_findAbsolutePath(tfs_t *const pTfs, const char *path);

/*
 * Creates new directory in parent dir
 * return block address of new directory
 */
tfs_uint32_t tfs_createDirectory(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name);

/*
 * Creates new file in parent dir
 * return block address of new file
 */
tfs_uint32_t tfs_createFile(tfs_t *const pTfs, tfs_uint32_t parentDirectoryBlockAddress, const char *name);

/*
 * Reads file
 * return amount of read bytes
 */
tfs_uint32_t tfs_readFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress, void *const buffer, tfs_uint32_t size, tfs_uint32_t offset);

/*
 * Writes file
 * return amount of wrotee bytes
 */
tfs_uint32_t tfs_writeFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress, const void *const data, tfs_uint32_t size, tfs_uint32_t offset);

/*
 * Removes file from directory
 */
tfs_bool_t tfs_removeFile(tfs_t *const pTfs, tfs_uint32_t fileBlockAddress);

tfs_bool_t tfs_removeDirectory(tfs_t *const pTfs, tfs_uint32_t directoryBlockAddress);

#endif
