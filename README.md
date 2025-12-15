# TreeFS, your files survive... even if your HDD doesn't
TreeFS was designed mainly for embedded use, or on solid state drives

## Current features
- Creating directories and files
- Reading and writing files
- Removing files

## Features to add
- Removing directories

### TreeFS library is under [src/tfs/](src/tfs/)

## What this even is?
- It is a file system based on tree
- Everything have parent 
- Everything is a node
- Everything takes 1 block and 1 block exactly
- Directories can have more than 1 child

## How it works?
Simply!<br/>
- Addressing is per block, not per byte.
- There is 1 file system info block at address 1.
- There are block bitmaps that describe usage of every block, bitmaps start at address 2 and appear every 7680 blocks. 
- Every bit in block bitmap describes usage of block below, one block bitmap governs 7679 blocks below, first bit is reserved as 1, as block bitmap is self used.
- Every block have 64 byte header. Header describes that block purpose.
- Every directory entry is 48 bytes long, have 32 bytes for name, 4 bytes for block address, and 12 bytes for other info.
- Every file if a block that can have continuation and must have pointer in form directory entry.
- Every block is 1kB in size, 64 bytes of header, 960 bytes of data.
<br/>

#### In more visual form it looks like this:<br/>
| Structure | Block address |
| --- | --- |
| File System Info Block | 1 |
| Block Bitmap | 2 |
| Files/Directory blocks | 3 |
| ... | 4 - 7681 |
| Block Bitmap | 7682 |
| ... | 7863 - 15542 |

Block Bitmap:
| Name | Bytes |
| --- | --- |
| Header | 0 - 63 |
| Bitmap | 64 - 1024 |

Directory block:
| Name | Bytes |
| --- | --- |
| Header | 0 - 63 |
| Directory entry | 64 - 111 |
| Directory entry | 112 - 159 |
| ... | 160 - 1024 |

File block:
| Name | Bytes |
| --- | --- |
| Header | 0 - 63 |
| File data | 64 - 1024 |

#### How those structure look?
##### File System Info Block struct:
| Field | Bytes |
| --- | --- |
| Checksum | 0 - 3 |
| Xored Checksum | 4 - 7 |
| Block count | 8 - 11 |
| Magic number | 12 - 13 |
| Xored magic number | 14 - 15 |
| Root directory block address | 16 - 19 |
| RESERVED | 20 - 1023 |

File system struct takes entire block, so 1024 bytes

##### Header struct:
| Field | Bytes |
| --- | --- |
| Checksum | 0 - 3 |
| Flags | 4 - 7 |
| Size low | 8 - 11 |
| Size high | 12 - 15 |
| Parent block | 16 - 19 |
| Next block | 20 - 23 |
| Entry offset | 24 - 27 |
| Entries in block | 28 - 31 |
| RESERVED | 32 - 63 |

Header struct takes 64 bytes

Header flags field can take values:
| Value | Name | Description |
| --- | --- | --- |
| 0 | Free | Block is free |
| 0x1 | Directory | Block is used as directory metadata, file or used block bitmap flags cannot be set here |
| 0x2 | File | Block is used to store file data, directory or used block bitmap flags cannot be set here |
| 0x4 | Continuation | Describes block as continuation of other block, it is set along with either directory flag or file flag, block bitmap doesnt care |
| 0x8 | Block checksum | On modification, block would have a checksum calculated out of entire block of data |
| 0x10 | Header checksum | On modification, block would have a checksum calculated only out of header data |
| 0x20 | Used blocks bitmap | Describes block as bitmap of used blocks below, cannot be set along with directory or file flags | 

##### Directory entry struct:
| Field | Bytes |
| --- | --- |
| Block address | 0 - 3 |
| Type | 4 - 5 |
| Name length | 6 |
| Name | 7 - 38 |
| RESERVED | 39 - 47 |

Directory entry takes 48 bytes

Directory entry type field can take values:
| Value | Name | Description |
| --- | --- | --- |
| 0 | Free | Directory entry is free to use |
| 1 | File | Directory entry contain pointer to file |
| 2 | Directory | Directory entry contain pointer to other directory |
