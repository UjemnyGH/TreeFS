# TreeFS
#### Your files survive... even if your HDD doesn't!
TreeFS was designed mainly for embedded use, or on solid state drives

## History
I basically wanted a file system that I can use on embedded devices, so I started implementing ext2 for bare metal, 
but it have many unnecessary "features", like timestamps and permissions. I didnt need that on STM32, so I started 
remaking it into my own file system, but my own file system was also not that well suited. <br/> 
Then I thought, what even is a file system? <br/> 
It have files and directories. Directories can basically store more directories and files, so directory is like tree
node. Thats when idea appeared to make TreeFS, a tree based file system, with no useless features (at base at least, 
I left some bytes for future expansion). I made TreeFS project at 11PM on the Saturday of 13.12.2025, by the end of 
Sunday (14.12.2025) I had working almost file system. On the Wednesday (17.12.2025) I had working CLI and working 
file system, fully tested and checked to be idiot-proof, as I know I would be able to pull up some "root removing"
activities. <br/>
<br/>
So thats where base TreeFS currently sits on. It needs 2kB of RAM, where 1kB is used as a operation buffer and 1kB 
is just a buffer for other non related stuff (I wouldnt recommend running it on 1kB systems as it would take entire 
memory, thats why 2kB minimum requirement). It doesnt use dynamic allocation, it doesnt use any STD C libraries. <br/>
Basically every line of code for base file system is written from the ground up by me, though out by me and designed 
by me, even stuff like memcpy (`_tfs_copyMemory` in tfs/treefs.c), memset (`_tfs_setMemory` in tfs/treefs.c), memcmp 
(`_tfs_compareMemory` in tfs/treefs.c) and strlen (`_tfs_lengthString` in tfs/treefs.c) were written by me. Entire 
file system uses checksum to check if block is valid. Entire file system is decentralised. If error occurs, only 1
block can be possibly corrupted, which should be easy to find.

## TL;DR, What this even is?
- It is a file system based on tree
- Everything have parent 
- Everything is a node
- Everything takes 1 block and 1 block exactly
- Directories can have more than 1 child

## Future plans
- Add fill large files support feature
- Add recovery API
- Add wear leveling feature
- Add timestamps feature
- Add permissions feature
- Make FUSE to use it on linux more easily
- Make full driver for it

Basically with all the features it would somewhat meet POSIX requirements. But base doesnt need to meet them, base 
must just work and be usable on virtually any system anyone can think of (as long as it have 2kB of RAM)!

## Usage
For now you need to compile it yourself using cmake
> mkdir build <br/>
> cd build <br/>
> cmake .. <br/>
> cmake --build .

You should have then `tfs` executable, which can modify TreeFS drive from CLI. Currently CLI features are not very rich.

## Documentation
Yea, what about it, currently there is none, but I would make it in future, describing every function and how to use it

## Current features
- Creating directories and files
- Reading and writing files
- Removing files
- Removing directory
- Renaming directories and files
- Listing directories
- And more ;) 

## Features to add
- Removing directories

### TreeFS library is under [src/tfs/](src/tfs/)

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
