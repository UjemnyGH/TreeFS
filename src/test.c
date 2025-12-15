#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "tfs/treefs.h"

const char *tfsBlockName = "test.tfs";

const char file1Content[] = "Hello world from TreeFS\n\tFileTest - 1\n\tThis message shouldnt take more than 1 block of data";

const char file2COntent[] = 
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi vehicula libero id purus venenatis, in blandit quam auctor. Nam facilisis lacinia nulla at ultrices. Fusce sed commodo ipsum. In non nisl libero. Suspendisse at lacus justo. Curabitur iaculis lacus facilisis dictum commodo. Etiam vitae imperdiet dui, non posuere lorem. Integer efficitur, neque et consectetur pellentesque, tellus eros feugiat odio, non fermentum velit diam eget tortor. Ut quis aliquam est. Maecenas tincidunt egestas feugiat."
"Aenean commodo risus nec enim vehicula, at hendrerit magna venenatis. Etiam quis nisl imperdiet, gravida nulla nec, suscipit enim. Nullam porta massa nec diam imperdiet dignissim. Donec vulputate vulputate massa, in sollicitudin arcu ultricies vel. Vivamus eu varius arcu, in luctus felis. Pellentesque rhoncus euismod ligula vitae luctus. Sed bibendum eu purus eget gravida. Mauris iaculis suscipit placerat. Fusce vel consequat eros, vitae mollis velit. Integer eu vestibulum erat, nec mattis dolor. Vestibulum aliquam turpis vel ligula lobortis, quis feugiat sapien maximus. Aliquam bibendum at dui quis commodo. Integer ac lorem a massa mattis molestie. Fusce in tincidunt nulla, vel congue arcu. Ut ultrices semper augue, at aliquet eros malesuada id."
"Etiam turpis tortor, pharetra a consectetur vel, vulputate ac lacus. Fusce quis est sed leo egestas semper ac viverra neque. Phasellus porta mi at elit volutpat, non eleifend odio lacinia. Cras dapibus rhoncus elementum. Fusce facilisis elementum leo sed ornare. Nam finibus blandit tortor id viverra. Sed vehicula ligula lorem, id semper nisi tempor eu. Curabitur lacinia porttitor leo. Ut faucibus neque leo, nec porta justo rhoncus et. Maecenas lacinia vulputate ullamcorper. Proin porta ut augue ut interdum. Nunc sodales magna lorem, a vulputate urna imperdiet nec. Pellentesque eu erat pellentesque, venenatis metus a, rutrum tortor. Donec varius lectus lacus, eget ultrices erat ornare nec."
"Sed eu ex faucibus, dapibus nibh tempus, pulvinar nisi. Integer placerat velit augue, sit amet bibendum urna fringilla id. Ut interdum aliquam rutrum. Nulla iaculis erat massa, in consequat quam posuere id. In hac habitasse platea dictumst. Phasellus scelerisque augue non elit ultrices, non ornare orci porta. Integer non sem in nunc aliquet ultrices vel eu justo. Nulla condimentum sem nec lectus molestie, quis efficitur erat luctus. Phasellus eu justo quis purus tincidunt dignissim. In hac habitasse platea dictumst. Donec facilisis nec risus ac cursus. Duis justo libero, commodo eu ultrices id, vestibulum ac orci. Fusce et tincidunt eros, quis lobortis dolor."
"Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum semper, metus in efficitur porttitor, risus mauris vulputate justo, eget commodo enim lectus vel dui. Vestibulum eu leo in metus blandit tincidunt. Cras volutpat dignissim nibh, ac tempor velit interdum eu. Nunc rutrum id erat vel hendrerit. Proin at dignissim neque. Vivamus ut arcu ac lectus tempor lacinia. In nibh sapien, tempor sed ullamcorper a, placerat et nunc. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Cras at dolor pellentesque, volutpat felis a, feugiat odio. Nunc at mattis turpis. Proin imperdiet dapibus mauris, ut varius elit. Curabitur turpis lorem, tristique et malesuada et, porta at libero. Donec euismod massa purus, et ornare lorem vestibulum ut. Nulla eget risus eu arcu ullamcorper auctor id non velit."
"Fusce turpis tellus, bibendum at consectetur et, ultricies quis dui. Phasellus sed ipsum justo. Fusce non rhoncus mauris, vel consequat eros. Duis vestibulum aliquet rutrum. Nullam blandit enim at libero tincidunt ullamcorper. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nam efficitur risus vel arcu suscipit tempor. Nam at suscipit tortor. Vestibulum ex nulla, ullamcorper in hendrerit nec, rhoncus ut magna. In sollicitudin laoreet augue in fringilla. Nulla accumsan urna vel nisi scelerisque, in commodo eros finibus. Sed molestie enim a ornare malesuada. Aliquam condimentum porttitor gravida."
"Praesent a nibh elit. Nullam eu diam in neque iaculis elementum vel eu ex. Mauris fringilla metus nisl, eget pharetra libero eleifend ut. Duis sed bibendum nisi. Sed cursus id est eu ultricies. Cras convallis magna a fringilla imperdiet. Etiam egestas venenatis nibh sit amet sollicitudin."
"Sed efficitur felis ac massa sodales feugiat. Proin hendrerit varius ligula eu iaculis. Aliquam quis ligula quis augue lobortis tempus vehicula vitae neque. In tincidunt ante id efficitur gravida. Morbi sit amet aliquet justo. Pellentesque dapibus sapien vitae ornare bibendum. Sed lectus augue, efficitur euismod blandit eu, tincidunt a metus. Ut venenatis, purus vel commodo mollis, metus nunc fermentum arcu, a pellentesque magna lacus in felis. In vitae arcu eget justo ornare lobortis vitae gravida enim. Proin eget tellus sit amet lectus cursus pellentesque. Mauris sed dui mauris. Nulla viverra eu est eu mollis. Quisque et interdum lectus. Donec sit amet pharetra risus, eget placerat tortor."
"Integer egestas neque quis eros dapibus, sed porttitor nunc auctor. Donec malesuada molestie risus, sit amet ultrices sem. Praesent volutpat dignissim sapien, et vestibulum erat mollis non. Integer lacinia suscipit ultricies. Donec condimentum id velit elementum auctor. Sed eget tellus tempus, sagittis turpis condimentum, semper ipsum. Nunc ac ex vitae neque imperdiet rhoncus quis eget mauris. Vivamus vestibulum elementum tempus."
"Maecenas quis imperdiet lorem. Cras condimentum lacus vitae justo facilisis, ac porttitor tortor commodo. Donec iaculis massa ac eros viverra, sed aliquet diam imperdiet. Integer sit amet ultricies libero, eu posuere orci. Nulla et tempus eros. Sed in dolor mollis, ultrices felis et, congue orci. Nulla tincidunt laoreet porttitor. Fusce orci ex, fringilla et lectus sed, ultricies tincidunt dolor. Cras dignissim, dui at euismod commodo, ligula dui pharetra dui, aliquet commodo ex ligula quis ante. Nulla at tellus ac ex fermentum blandit. Sed pretium, augue non commodo mollis, ipsum ante mattis dolor, sit amet malesuada ligula lacus et quam. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nulla ultrices ipsum erat, porttitor convallis justo lacinia in. Aliquam quis nisi felis."
"Donec elit nisi, vehicula eu consequat eget, tincidunt in orci. Fusce scelerisque sagittis molestie. Mauris fermentum tristique lorem ac gravida. Quisque in fermentum nibh, dignissim cursus felis. Aenean id dolor vitae nisl ultricies venenatis in vel lectus. Integer pulvinar varius molestie. Nullam sed luctus enim. Nulla pharetra, justo eu sagittis commodo, purus elit eleifend nunc, nec luctus ligula arcu condimentum lorem. Aliquam laoreet, purus id iaculis dapibus, elit diam vulputate purus, ut tincidunt nisl nulla ac quam. Curabitur euismod sem ac turpis vehicula, in convallis purus rhoncus. Nulla dignissim lectus massa, id condimentum ante dignissim non. Integer eleifend at tortor vel tincidunt."
"Nulla dictum leo in viverra elementum. Cras id rutrum nulla. Morbi eget enim a turpis maximus faucibus. Duis pretium odio ac leo pulvinar, quis volutpat lorem efficitur. Phasellus ultrices dolor enim, vel facilisis purus semper et. Proin vel ipsum tristique, pretium felis id, rutrum leo. Nam sed eros congue quam suscipit volutpat. Donec quam magna, ultricies vel feugiat nec, consequat ut urna. Ut scelerisque molestie ante, vel tempor sem feugiat id. Donec ac lacus est. Morbi at imperdiet diam. Vestibulum consectetur ligula neque, sit amet blandit libero tristique est. ";

void test_writeHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE* file = fopen(tfsBlockName, "rb+");

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fwrite(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

void test_readHandle(void *buffer, tfs_uint32_t blockAddress) {
  FILE* file = fopen(tfsBlockName, "rb+");

  fseek(file, blockAddress * TFS_BLOCK_SIZE, SEEK_SET);
  fread(buffer, sizeof(tfs_uint8_t), TFS_BLOCK_SIZE, file);

  fclose(file);
}

tfs_t tfs;

int test() {
  tfs_init(&tfs, test_writeHandle, test_readHandle);

  FILE *f = fopen(tfsBlockName, "wb+");
  fputc(0, f);
  fclose(f);

  if(!tfs_newFileSystem(&tfs, 3812 * TFS_1KB)) {
    printf("[ERROR]: Cannot create file system!\n");

    return 1;
  }

  printf("[INFO]: Made new file system under %s\n", tfsBlockName);

  tfs_uint32_t testDir1 = tfs_createDirectory(&tfs, tfs.rootDirBlock, "testDir1");
  tfs_createDirectory(&tfs, tfs.rootDirBlock, "testDir2");
  tfs_createDirectory(&tfs, testDir1, "testDirInsideDir");
  tfs_createDirectory(&tfs, tfs_findAbsolutePath(&tfs, "/testDir1"), "testDirInsDirAbsPath");

  tfs_createFile(&tfs, tfs.rootDirBlock, "file1.txt");
  tfs_createFile(&tfs, testDir1, "fileInDir1.txt");
  tfs_createFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir2"), "fileDirInAbsPath1.txt");
  tfs_createFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir1/testDirInsDirAbsPath"), "fileDirInAbsPath2.txt");

  printf("[INFO]: Created test directory structure\n");

  tfs_writeFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir2/fileDirInAbsPath1.txt"), file2COntent, sizeof(file2COntent), 0);

  tfs_writeFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir1/testDirInsDirAbsPath/fileDirInAbsPath2.txt"), (void*)file1Content, sizeof(file1Content), 0);

  printf("[INFO]: Wrote file\n");

  char readBuffer[512] = {0};
  tfs_readFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir1/testDirInsDirAbsPath/fileDirInAbsPath2.txt"), (void*)readBuffer, 512, 0);

  printf("Read file:\n%s\n", readBuffer);

  char lorem[8192];
  tfs_readFile(&tfs, tfs_findAbsolutePath(&tfs, "/testDir2/fileDirInAbsPath1.txt"), lorem, sizeof(lorem), 0);

  printf("%s\n", lorem);

  return 0;
}
