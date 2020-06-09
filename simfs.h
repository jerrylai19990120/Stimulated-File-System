#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);

/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
void createfile(char* filename, char* fsname);
void deletefile(char* filename, char* fsname);
void writefile(char* filename, int start, int length, char* fsname);
void readfile(char* filename, int start, int length, char* fsname);