/* This file contains functions that are not part of the visible "interface".
 * They are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"
//#include "simfstypes.h"

/* Internal helper functions first.
 */
fentry* read_fentry(char* fsname, FILE* fp){
  fentry* files = malloc(sizeof(fentry)*MAXFILES);

  fseek(fp, 0, SEEK_SET);
  if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
    fprintf(stderr, "read_fentry: could not read file entries\n");
    exit(1);
  }
 
  return files;
}

char* read_block(char* fsname, FILE* fp, int block_num){
  char* block = malloc(sizeof(char)*BLOCKSIZE);
  fseek(fp, BLOCKSIZE*block_num, SEEK_SET);
  if(fread(block, 1, BLOCKSIZE, fp)==0){
    fprintf(stderr, "read_block: cannot read blocks.\n");
    exit(1);
  }
  return block;
}

char* data_output(char* block, int start, int length){
  if(start+length>BLOCKSIZE){
     char* data = malloc(sizeof(char)*BLOCKSIZE);
  int index = 0;
  for(int i=start;i<BLOCKSIZE;i++){
    data[index] = block[i];
    index++;
  }
  return data;
  }else{
     char* data = malloc(sizeof(char)*length);
  int index = 0;
  for(int i=start;i<start+length;i++){
    data[index] = block[i];
    index++;
  }
  return data;
  }
}

fnode* read_fnode(char* fsname, FILE* fp){
  fnode* fnodes = malloc(sizeof(fnode)*MAXBLOCKS);
  fseek(fp, MAXFILES*16, SEEK_SET);
  if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
    fprintf(stderr, "read_fnode: could not read fnodes\n");
    exit(1);
  }
  return fnodes;
}

void write_fentry(char* fsname, fentry* files, FILE* fp){
  fseek(fp, 0, SEEK_SET);
  if(fwrite(files, sizeof(fentry)*MAXFILES, 1, fp)==0){
    fprintf(stderr, "Cannot write fnodes.");
    exit(1);
  }
  
}

void write_fnode(char* fsname, fnode* fnodes, FILE* fp){
  fseek(fp, MAXFILES*16, SEEK_SET);
  if(fwrite(fnodes, sizeof(fnode)*MAXBLOCKS, 1, fp)==0){
    fprintf(stderr, "Cannot write fnodes.");
    exit(1);
  }
  
}

void write_block(char* fsname, char* block, FILE* fp, int block_num){
  
  fseek(fp, BLOCKSIZE*block_num, SEEK_SET);
  
  if(fwrite(block, BLOCKSIZE, 1, fp)==0){
    fprintf(stderr, "write_block: cannot write block.\n");
    exit(1);
  }
}

int seek_avail_fentry(fentry* files){
  int num = -1;
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, "") == 0){
      num = i;
      break;
    }
  } 
  
  return num;
}

int seek_avail_fnode(fnode* fnodes){
  int num = -1;
  for(int i=0;i<MAXBLOCKS;i++){
    if(fnodes[i].blockindex < 0){
      num = i;
      break;
    }
  } 
  
  return num;
}

int* files_block_index(fentry* files){
  int num = 0;
  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock != -1){
      num++;
    }
  }
  int* indices = malloc(sizeof(int)*num);
  int count = 0;
  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock != -1){
      indices[count] = (int)files[i].firstblock;
      count++;
    }
  }
  return indices;
}

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if((fp = fopen(filename, mode)) == NULL) {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void
closefs(FILE *fp)
{
    if(fclose(fp) != 0) {
        perror("closefs");
        exit(1);
    }
}

/* File system operations: creating, deleting, reading, and writing to files.
 */
void createfile(char*filename, char* fsname){
  if((int)strlen(filename)>11){
    fprintf(stderr, "File name is too long.\n");
    exit(1);
  }
  FILE* ptr = openfs(fsname, "r");
  fentry* files = read_fentry(fsname, ptr);
  fnode* fnodes = read_fnode(fsname, ptr);
  closefs(ptr);
  int avail_file = seek_avail_fentry(files);
  if(avail_file == -1){
    exit(1);
  }
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, filename)==0){
      printf("%s %s\n", files[i].name, filename);
      fprintf(stderr, "createfile: Duplicates not allowed.\n");
      exit(1);
    }
  }
  strncpy(files[avail_file].name, filename, 12);
 
  FILE* fp = openfs(fsname, "r+");
  write_fentry(fsname, files, fp);
  write_fnode(fsname, fnodes, fp);

  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock>0){
      char* data = read_block(fsname, fp, files[i].firstblock);
      write_block(fsname, data, fp, files[i].firstblock);
      free(data);
    }
  }
  free(files);
  free(fnodes);
  closefs(fp);
}

void deletefile(char*filename, char* fsname){
  FILE* fp = openfs(fsname, "r");
  fentry* files = read_fentry(fsname, fp);
  fnode* fnodes = read_fnode(fsname, fp);
  closefs(fp);
  int file_deleted = 0;
  int block_num = -1;
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, filename)==0){
      files[i].name[0] = '\0';
      files[i].size = 0;
      block_num = files[i].firstblock;
      files[i].firstblock = -1;
      file_deleted++;
      break;
    }
  }
  for(int i=0;i<MAXBLOCKS;i++){
    if(fnodes[i].blockindex == block_num){
      fnodes[i].blockindex = -i;
      break;
    }
  }
  char* buffer = malloc(sizeof(char)*BLOCKSIZE+1);
  buffer[BLOCKSIZE] = '\0';
  for(int i=0;i<BLOCKSIZE;i++){
    buffer[i] = '0';
  }
  FILE* ptr = openfs(fsname, "r+");
  write_block(fsname, buffer, ptr, block_num);
  write_fentry(fsname, files, ptr);
  write_fnode(fsname, fnodes, ptr);
  closefs(ptr);
  free(fnodes);
  free(files);
  free(buffer);
  if(file_deleted==0){
    fprintf(stderr, "Error: file does not exist.");
    exit(1);
  }
}

void readfile(char*filename, int start, int length, char* fsname){
  if(start==0&&length==0){
    fprintf(stderr, "Cannot read.\n");
    exit(1);
  }
  FILE* ptr = openfs(fsname, "r");
  fentry* files = read_fentry(fsname, ptr);
  fnode* fnodes = read_fnode(fsname, ptr);
  int block_num = -1;
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, filename)==0){
      block_num = files[i].firstblock;
      if(length<0 || length>files[i].size){
        fprintf(stderr, "Invalid length.\n");
        exit(1);
      }
      if(start<0 || start>files[1].size){
        fprintf(stderr, "Invalid start.\n");
        exit(1);
      }
      if(start+length > files[i].size){
        fprintf(stderr, "Invalid start and length.\n");
        exit(1);
      }
      break;
    }
  }
  if(block_num == -1){
    fprintf(stderr, "There is no data to read.\n");
    exit(1);
  }
  char* block = read_block(fsname, ptr, block_num);
  char* data = data_output(block, start, length);
  if(start+length>BLOCKSIZE){
      for(int i=0;i<BLOCKSIZE;i++){
         printf("%c", data[i]);
      }
  }else{
     for(int i=0;i<length;i++){
       printf("%c", data[i]);
     }
  }
 
  closefs(ptr);
  FILE* ptr2 = openfs(fsname, "r+");
  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock>0){
      char* data = read_block(fsname, ptr2, files[i].firstblock);
      write_block(fsname, data, ptr2, files[i].firstblock);
      free(data);
    }
  }
  write_fentry(fsname, files, ptr2);
  write_fnode(fsname, fnodes, ptr2);
  closefs(ptr2);
  free(block);
  free(data);
  free(fnodes);
  free(files);
}

void writefile(char*filename, int start, int length, char* fsname){
  if(start==0&&length==0){
    fprintf(stderr, "Cannot write.\n");
    exit(1);
  }
  FILE* ptr = openfs(fsname, "r");
  fnode* fnodes = read_fnode(fsname, ptr);
  fentry* files = read_fentry(fsname, ptr);
  char* block = malloc(sizeof(char)*BLOCKSIZE+1);
  
  closefs(ptr);
  int store = 0;
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, filename)==0){
      store = files[i].firstblock;
      if(length<0){
        fprintf(stderr, "Invalid length.\n");
        exit(1);
      }
      if(length==0){
        exit(1);
      }
      if(start<0 || start>files[i].size){
        fprintf(stderr, "Invalid start.\n");
        exit(1);
      }
    }
  }
 
  int avail_fnode;
  if(store == -1){
    avail_fnode = seek_avail_fnode(fnodes);
  }else if(store == 0){
    exit(1);
  }else{
    avail_fnode = store;
  }
  
  if(avail_fnode < 0){
    exit(1);
  }
  
  for(int i=0;i<MAXFILES;i++){
    if(strcmp(files[i].name, filename)==0){
      files[i].firstblock = avail_fnode;
      if(length+start>BLOCKSIZE){
        files[i].size = BLOCKSIZE;
        fread(block, 1, (length+start)-(length+start-BLOCKSIZE)-start, stdin);
      }else{
        if(files[i].size < start+length){
          files[i].size = start+length;
        }
        fread(block, 1, length, stdin);
        block[BLOCKSIZE] = '\0';
        for(int i=length;i<BLOCKSIZE;i++){
          block[i] = '0';
        }
      }
      break;
    }
  }
  
  int num = 0;
  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock>0){
      num++;
    }
  }
  if(store==-1){
    FILE* fp2 = openfs(fsname, "r+");
    if(num>=2){
      fnodes[avail_fnode].blockindex = avail_fnode;
      for(int i=0;i<MAXFILES;i++){
        if(files[i].firstblock>0){
          if(files[i].firstblock==avail_fnode){
            write_block(fsname, block, fp2, avail_fnode);
          }
          char* data = read_block(fsname, fp2, files[i].firstblock);
          write_block(fsname, data, fp2, files[i].firstblock);
          free(data);
        }
      }
    }else{
      fnodes[avail_fnode].blockindex = avail_fnode;
      write_block(fsname, block, fp2, avail_fnode);
    }
    
    write_fentry(fsname, files, fp2);
    write_fnode(fsname, fnodes, fp2);
    free(block);
    free(fnodes);
    free(files);
    closefs(fp2);
    exit(0);
  }
  fnodes[avail_fnode].blockindex = avail_fnode;
  FILE* fp = openfs(fsname, "r+");
  write_fentry(fsname, files, fp);
  write_fnode(fsname, fnodes, fp);
  char* buffer = malloc(sizeof(char)*BLOCKSIZE+1);
  buffer[BLOCKSIZE] = '\0';
  for(int i=0;i<MAXFILES;i++){
    if(files[i].firstblock>0){
      char* data = read_block(fsname, fp, files[i].firstblock);
      if(files[i].firstblock==avail_fnode){
        for(int j=0;j<BLOCKSIZE;j++){
          buffer[j] = data[j];
        }
      }
      write_block(fsname, data, fp, files[i].firstblock);
      free(data);
    }
  }
  int count = 0;
  if(length+start>BLOCKSIZE){
     for(int i=start;i<BLOCKSIZE;i++){
	      buffer[i] = block[count];
	      count++;
     }
  }else{
    for(int i=start;i<length+start;i++){
        buffer[i] = block[count];
        count++;	
    }
  }	  
 
  write_block(fsname, buffer, fp, avail_fnode);
  free(buffer);
  free(block);
  free(fnodes);
  free(files);
  closefs(fp);
}
// Signatures omitted; design as you wish.
