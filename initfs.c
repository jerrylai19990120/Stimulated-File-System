#include <stdio.h>
#include <stdlib.h>
#include "simfs.h"


/* Create a simulated file system structure in the file specified by
 * filename.  This function overwrites whatever was in the file
 * filename.
 */

void
initfs(char *filename) {

    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    int i;
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    FILE *fp = openfs(filename, "w");

    /* Initialize the metadata structures */

    for(i = 0; i < MAXFILES; i++) {
        files[i].name[0] = '\0';
        files[i].size = 0;
        files[i].firstblock = -1;
    }

    for(i = 0; i < MAXBLOCKS; i++) {
        fnodes[i].blockindex = -i;
        fnodes[i].nextblock = -1;
    }
    for (i = 1;  i < MAXBLOCKS && i <= (bytes_used - 1) / BLOCKSIZE; i++) {
        fnodes[i].blockindex = i;
    }

    /* Write the metadata to the file. */

    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    /* Write buffer bytes to the file to fill the current block. */
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    char zerobuf[BLOCKSIZE] = {0};
    if (bytes_to_write != 0  && fwrite(zerobuf, bytes_to_write, 1, fp) < 1) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    closefs(fp);
}
