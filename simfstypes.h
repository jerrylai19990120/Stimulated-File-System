typedef struct file_entry {
  char name[12];          // An empty name means the fentry is not in use.
  unsigned short size;
  short firstblock;       // A -1 indicates that no file blocks have been allocated.
} fentry;

typedef struct file_node {
  short blockindex;       // A negative value indicates that this block is not use
  short nextblock;        // A -1 indicates there is no next block of data.
} fnode;


#define MAXFILES  4
#define MAXBLOCKS 6
#define BLOCKSIZE 128
