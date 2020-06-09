#include <stdio.h>
#include <stdlib.h>
#include "simfs.h"


/* Print the contents of the file system file in a readable form. */

void
printfs(char *filename) {
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    char block[BLOCKSIZE + 1];
    int i;

    FILE *fp = openfs(filename, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }
    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_read = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    if (bytes_to_read != 0  && fseek(fp, bytes_to_read, SEEK_CUR) != 0) {
        fprintf(stderr, "Error: seek failed during print\n");
        closefs(fp);
        exit(1);
    }

    printf("File entry structures:\n");

    for (i = 0; i < MAXFILES; i++) {
        printf("[%d] \"%s\"\t%hu\t%hd\n",
               i,
               files[i].name,
               files[i].size,
               files[i].firstblock);
    }

    printf("\nFile node structures:\n");
    for (i = 0; i < MAXBLOCKS; i++) {
        printf("[%d] %hd\t%hd\n",
               i,
               fnodes[i].blockindex,
               fnodes[i].nextblock);
    }

    /* Write the raw file data to standard out */
    printf("\nFile blocks:\n");
    while (fread(block, 1, BLOCKSIZE, fp) == BLOCKSIZE) {
        fwrite(block, BLOCKSIZE, 1, stdout);
    }
    if (ferror(fp)) {
        fprintf(stderr, "Error: could not read data block\n");
        closefs(fp);
        exit(1);
    }

    printf("\n");
    closefs(fp);
}
