#ifndef MYFS_H
#define MYFS_H

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>

#define MEGA (1024*1024)
#define INODE 1024
#define BLOCK_SZ 256
#define MAX_OPEN_FILES 1000

typedef struct {
  short file_type;              // 0-file or 1-directory
  short access_permission;
  int file_size;
  time_t last_modified;
  time_t last_accessed;
  int block_ptr[10];            // block_no of data-blocks for inode
} inode;

// open file data structure
typedef struct{
  int inode_no;
  int offset;
  char mode;
} file;

// data-block
typedef struct{
  int no;       // data-block number
  int nbytes;   // remaining bytes in block
  void *empty;  // pointer to the first empty byte
} block;

// Global variables
void * myfs;          // pointer to file system
void * myblocks;      // pointer to first data block
sem_t * sem,*sem2;    // semaphores
int * iptr;           // integer pointer to super-block
int cwd;              // inode of current-working-directory
int open_files;       // number of open files
char *inode_bmap, *block_bmap;  // bitmap of inodes and data-blocks
inode * myinode;      // pointer to first inode
file **myfiles;       // global file table
int shmid1,shmid2;    // shmids for myfs and file table
/*
 * Converts 9 bit char access permission to short type to reduce memory
 * usage and returns it
*/
short bin2short(char *bin){
  short access=0;
  int i;
  for(i=0;i<9;i++)
    access = 2*access + (bin[i]-'0');
  return access;
}

// updates number of data blocks and inodes used
void update_super(){
  int i;
  iptr[2]=0;
  iptr[4]=0;

  for(i=0;i<iptr[1];i++)
    if(inode_bmap[i]==1)
      iptr[2]++;

  for(i=0;i<iptr[3];i++)
    if(block_bmap[i]==1)
      iptr[4]++;
}

// adds open file entry to global table
int create_file_entry(int inode_no, int offset, char mode){
  int i;
  if(open_files>=MAX_OPEN_FILES)
    return -1;
  file *f = (file*)malloc(sizeof(file));
  f->inode_no = inode_no;
  f->offset = offset;
  f->mode = mode;
  for(i=0;i<MAX_OPEN_FILES;i++){
    if(myfiles[i]==NULL){
      myfiles[i]=f;
      open_files++;
      return i;
    }
  }
  return -1;
}

int get_freeInode(){
  int i,retval=-1;
  sem_wait(sem);
  for (i = 0; i < iptr[1]; i++) {
    if(inode_bmap[i]==0){
      inode_bmap[i]=1;
      retval = i;
      break;
    }
  }
  sem_post(sem);
  return retval;
}

int get_freeDB(){
  int i,retval=-1;
  sem_wait(sem);
  for (i = 0; i < iptr[3]; i++) {
    if(block_bmap[i]==0){
      block_bmap[i]=1;
      retval = i;
      break;
    }
  }
  sem_post(sem);
  return retval;
}

// initialise newly created file or folder inode
void newInode(int inode_no,short file_type,short file_size){
  int init[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};   // to initialise data-blocks for inode
  myinode[inode_no].file_type = file_type;
  myinode[inode_no].file_size = file_size;
  time(&myinode[inode_no].last_accessed);
  time(&myinode[inode_no].last_modified);
  myinode[inode_no].access_permission = bin2short("111000000");   // default owner access_permission
  memcpy(myinode[inode_no].block_ptr,init,sizeof(init));
}

// returns next data block for given inode
block newDB(int inode_no){
  block db;
  int fsize,x,y,z,db_no,sub_blocks;
  sub_blocks = BLOCK_SZ/sizeof(int);
  fsize = myinode[inode_no].file_size;
  x = fsize/BLOCK_SZ;
  y = fsize%BLOCK_SZ;
  if(x<8){
    if(y==0){
      db_no = get_freeDB();
      myinode[inode_no].block_ptr[x] = db_no;
    }
      db.no = myinode[inode_no].block_ptr[x];
  }
  else if(x<(sub_blocks+8)){
    if(x==8 && y==0){
      db_no = get_freeDB();
      myinode[inode_no].block_ptr[x] = db_no;
    }
    if(y==0){
      db_no = get_freeDB();
      *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int))) = db_no;
    }
    db.no = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int)));  //divide by sizeof(void)
  }
  else if(x<(sub_blocks*sub_blocks + sub_blocks + 8)){
    if(x==(sub_blocks+8) && y==0){
      db_no = get_freeDB();
      myinode[inode_no].block_ptr[9] = db_no;
    }
    if((x-sub_blocks-8)%sub_blocks==0 && y==0){
        db_no = get_freeDB();
        *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int))) = db_no;
    }
    if(y==0){
      db_no = get_freeDB();
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int)));
      *((int*)(myblocks + z*BLOCK_SZ + ((x-sub_blocks-8)%sub_blocks)*sizeof(int) )) = db_no;
    }
    z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int)));
    db.no = *((int*)(myblocks + z*BLOCK_SZ + ((x-sub_blocks-8)%sub_blocks)*sizeof(int) ));
  }
  else{
    printf("Too big file! Exhausted allocatable file blocks.\n");
    db.no = -1;
    db.nbytes = 0;
    db.empty = NULL;
    return db;
  }
  db.nbytes = BLOCK_SZ - y;
  db.empty = (void*)(myblocks+db.no*BLOCK_SZ+y);
  return db;
}

// insert file entry to the directory
void add_file2dir(int folder_no, char *fname,int file_no){
  sem_wait(sem2);
  block db;
  db = newDB(folder_no);
  if(db.nbytes < 32){
    printf("Unexpected error!\n");
  }
  strcpy((char*)db.empty,fname);        // adding entry
  *((int*)(db.empty+28)) = file_no;
  myinode[folder_no].file_size += 32;   // incrementing file size
  sem_post(sem2);
  // last_accessed folder
}

// get inode_no of file in current directory and optionally remove its entry
int getFileInode(char* fname,int rem_file){
  sem_wait(sem2);
  int x,y,i,j,retval=-1;
  x = (myinode[cwd].file_size)/BLOCK_SZ;
  y = (myinode[cwd].file_size)%BLOCK_SZ;
  if(x<=8){
    for(i=0;i<x;i++){
      for(j=0;j<BLOCK_SZ;j+=32){
        if(!strcmp(fname,(char*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ +j))){
          retval = *((int*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ + j + 28));
          if(rem_file){
            myinode[cwd].file_size -=32;
            for(;i<x;i++){
              for(;j<BLOCK_SZ;j+=32){
                if(j==BLOCK_SZ-32){
                  strcpy((char*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ +j),(char*)(myblocks + myinode[cwd].block_ptr[i+1]*BLOCK_SZ ));
                  *((int*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ + j + 28)) = *((int*)(myblocks + myinode[cwd].block_ptr[i+1]*BLOCK_SZ + 28));
                  continue;
                }
                strcpy((char*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ +j),(char*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ +j+32));
                *((int*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ + j + 28)) = *((int*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ + j + 60));
              }
            }
          }
          break;
        }
      }
      if(retval!=-1) break;
    }
    if(retval!=-1){
      sem_post(sem2);
      return retval;
    }
    if(x==8 && y>0){
      //
    }
    else if(y>0){
      for(j=0;j<y;j+=32){
        if(!strcmp(fname,(char*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ +j))){
          retval = *((int*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ + j + 28));
          if(rem_file){
            myinode[cwd].file_size -=32;
            for(;j<y-32;j+=32){
              strcpy((char*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ +j),(char*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ +j+32));
              *((int*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ + j + 28)) = *((int*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ + j + 60));

            }
          }
          break;
        }
      }
    }
  }
  sem_post(sem2);
  return retval;
}

// clearing file system
int clear_myfs(){
  shmdt (myfs);
  shmctl (shmid1, IPC_RMID, 0);
  shmdt (myfiles);
  shmctl (shmid2, IPC_RMID, 0);
  sem_unlink ("mutex");
  sem_close(sem);
  sem_unlink ("mutex2");
  sem_close(sem2);
  return 1;
}

// creating file system
int create_myfs (int size){
  int i;
  key_t key1,key2;
  char * cptr;

  sem_unlink ("mutex");
  sem_unlink ("mutex2");
  /* initialize semaphores for shared processes */
    sem = sem_open ("mutex", O_CREAT | O_EXCL, 0644, 1);
    sem2 = sem_open ("mutex2", O_CREAT | O_EXCL, 0644, 1);

  // creating shared memory segment for myfs
  key1 = 1234;
  shmid1 = shmget(key1,size*MEGA,IPC_CREAT|0666);
  if(shmid1<0){
    perror("shmget1\n");
    return -1;
  }
  myfs = shmat(shmid1,NULL,0);
  iptr = (int *)myfs;
  iptr[0] = size;            // total file sys size
  iptr[1] = INODE;           // max no of inodes supported
  iptr[2] = 0;               // actual no of inodes used

  // calculating total no of data-blocks
  iptr[3] = ( MEGA*size - ( 6*sizeof(int) + INODE*( sizeof(char)+sizeof(inode) ) ) ) / ( sizeof(char)+BLOCK_SZ );
  iptr[4] = 0;               // no of blocks used
  cptr = (char*)(iptr+6);
  inode_bmap = cptr;
  block_bmap = cptr+INODE;

  // initialise bitmap for both free inodes and diskblocks
  for ( i = 0; i < (INODE+iptr[3]); i++)
    cptr[i]=0;
  myinode = (inode*)(cptr+i);       // myinode points to start of inode list
  myblocks = (void*)(myinode+INODE);// myblocks points to start of data-blocks

  // create home directory
  cwd = get_freeInode();        // current working directory set to root
  newInode(cwd,1,0);
  iptr[5] = cwd;                // store root directory in super-block
  open_files = 0;               // initialise number of open files

  // shared segment for global file table
  key2 = 8080;
  shmid2 = shmget(key2,sizeof(file*)*MAX_OPEN_FILES,0666|IPC_CREAT);
  if(shmid2<0){
    perror("shmget2\n");
    return -1;
  }
  myfiles = (file**)shmat(shmid2,NULL,0);

  // initialise global open file table
  for(i=0;i<MAX_OPEN_FILES;i++)
    myfiles[i]=NULL;
  return 0;
}

// copy files from pc to mrfs
int copy_pc2myfs(char *source, char *dest){
  struct stat ast;
  int fsize, fd, inode_no;
  block db;
  fd = open(source,O_RDONLY);     // open pc file in read mode
  if(fd<0)
    return -1;

  fstat(fd,&ast);
  fsize = ast.st_size;
  if(iptr[2] == iptr[1]){
    printf("All inodes used: %d\n",iptr[2]);
    return -1;
  }
  if( (iptr[3]-iptr[4])*BLOCK_SZ < fsize ){
    printf("Not enough space! Required = %d B | Available = %d B\n",fsize,(iptr[3]-iptr[4])*BLOCK_SZ);
    return -1;
  }
  // check for extra large files here

  inode_no = get_freeInode();       // create inode for mrfs file
  newInode(inode_no,0,0);

  // copy file blockwise
  while(fsize>0){
    db = newDB(inode_no);
    // check semaphore
    if(db.empty==NULL)
      return -1;
    if(fsize<=db.nbytes){
      read(fd,db.empty,fsize);
      myinode[inode_no].file_size += fsize;
      fsize = 0;
    }
    else{
      read(fd,db.empty,db.nbytes);
      fsize -= db.nbytes;
      myinode[inode_no].file_size += db.nbytes;
    }
  }
  add_file2dir(cwd,dest,inode_no);        // add file to current directory
  return 0;
}

// display contents of current directory
int ls_myfs(){
  char name[28];
  int inode_no,x,y,i,j;
  printf("| name\t| size(B)\t| type\t| last_modified\t| last_accessed\t| access_permission |\n");
  printf("----------------------------------------------------------------------------------\n");

  x = (myinode[cwd].file_size)/BLOCK_SZ;
  y = (myinode[cwd].file_size)%BLOCK_SZ;

  if(x<=8){
    for(i=0;i<x;i++){
      for(j=0;j<BLOCK_SZ;j+=32){
        strcpy(name,(char*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ +j));
        inode_no = *((int*)(myblocks + myinode[cwd].block_ptr[i]*BLOCK_SZ + j + 28));
        printf("|%s\t| %d\t| %d\t| %.19s\t| %.19s\t| %d |\n",name,myinode[inode_no].file_size,
        myinode[inode_no].file_type, ctime(&myinode[inode_no].last_modified),ctime(&myinode[inode_no].last_accessed),
        myinode[inode_no].access_permission);
      }
    }
    if(x==8 && y>0){
      //
    }
    else if(y>0){
      for(j=0;j<y;j+=32){
        strcpy(name,(char*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ +j));
        inode_no = *((int*)(myblocks + myinode[cwd].block_ptr[x]*BLOCK_SZ + j + 28));
        printf("|%s\t| %d\t| %d\t| %.19s\t| %.19s\t| %d |\n",name,myinode[inode_no].file_size,
        myinode[inode_no].file_type, ctime(&myinode[inode_no].last_modified),ctime(&myinode[inode_no].last_accessed),
        myinode[inode_no].access_permission);
      }
    }
  }
}

// remove file from mrfs
int rm_myfs(char *filename){
  int inode_no,x,y,z,i,j,sub_blocks;
  inode_no = getFileInode(filename,1);      // remove file from directory-list
  if(inode_no==-1)
    return -1;
  sem_wait(sem);
  x = myinode[inode_no].file_size/BLOCK_SZ;
  y = myinode[inode_no].file_size%BLOCK_SZ;
  sub_blocks = BLOCK_SZ/sizeof(int);
  if(x<=8){
    for(i=0;i<x;i++)
      block_bmap[myinode[inode_no].block_ptr[i]] = 0;
    if(x==8 && y>0){
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int)))] = 0;
      block_bmap[myinode[inode_no].block_ptr[8]] = 0;
      iptr[4]++;
    }
    else if(y>0)
      block_bmap[myinode[inode_no].block_ptr[x]] = 0;
  }
  else if(x<=(sub_blocks+8)){
    iptr[4]++;
    for(i=0;i<=8;i++)
      block_bmap[myinode[inode_no].block_ptr[i]] = 0;
    for(i=0;i<(x-8);i++)
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + i*sizeof(int)))] = 0;
    if(x==(sub_blocks+8) && y>0){
      iptr[4] +=2;
      block_bmap[myinode[inode_no].block_ptr[9]]=0;
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ))]=0;
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ));
      block_bmap[*((int*)(myblocks + z*BLOCK_SZ ))] = 0;
    }
    else if(y>0)
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int)))] = 0;
  }
  else if(x<=(sub_blocks*sub_blocks + sub_blocks + 8)){
    iptr[4] += 3;
    for(i=0;i<=9;i++)
      block_bmap[myinode[inode_no].block_ptr[i]] = 0;

    for(i=0;i<(sub_blocks);i++)
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + i*sizeof(int)))] = 0;

    for(i=0;i<(x-sub_blocks-8)/sub_blocks;i++){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      block_bmap[z] = 0;
      for(j=0;j<sub_blocks;j++){
        block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
      }
    }
    if((x-sub_blocks-8)%sub_blocks >0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      block_bmap[z] = 0;
      for(j=0;j<((x-sub_blocks-8)%sub_blocks);j++)
        block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
        if(y>0)
          block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
    }
    else if(y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      block_bmap[z] = 0;
      block_bmap[*((int*)(myblocks + z*BLOCK_SZ ))] = 0;
    }
  }
  inode_bmap[inode_no] = 0;
  sem_post(sem);
  return 0;
}

// print status of file system
int status_myfs(){
  int i;
  update_super();
  double used=0,freespace;
  for(i=0;i<iptr[1];i++){
    if(inode_bmap[i]==1){
      used += myinode[i].file_size;
    }
  }
  freespace = (iptr[3]*BLOCK_SZ - used)/MEGA;
  printf("************ FILE SYSTEM STATISTICS ************\n");
  printf("Total File System       :\t%d MB\n",iptr[0]);
  printf("Space Occupied          :\t%lf MB\n",(iptr[0]-freespace));
  printf("Number of files/folders :\t%d\n",iptr[2]);
  printf("Space Available         :\t%lf MB\n",freespace);
  printf("Block used              :\t%d / %d\n",iptr[4],iptr[3]);
}

// copy file from mrfs to pc
int copy_myfs2pc(char *source, char *dest){
  int fd,x,y,z,z_,inode_no,sub_blocks,i,j;
  if(!strcmp(dest,"stdout"))
    fd = 1;
  else
    fd = open(dest,O_WRONLY|O_CREAT,0777);
  inode_no = getFileInode(source,0);
  if(inode_no==-1)
    return -1;
  x = myinode[inode_no].file_size/BLOCK_SZ;
  y = myinode[inode_no].file_size%BLOCK_SZ;
  sub_blocks = (BLOCK_SZ)/sizeof(int);

  if(x<=8){
    for(i=0;i<x;i++)
      write(fd,(myblocks + myinode[inode_no].block_ptr[i]*BLOCK_SZ),BLOCK_SZ);
    if(x==8 && y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ));
      write(fd,(myblocks + z*BLOCK_SZ),y);
    }
    else if(y>0)
      write(fd,(myblocks + myinode[inode_no].block_ptr[i]*BLOCK_SZ),y);
  }
  else if(x<=(sub_blocks+8)){
    for(i=0;i<8;i++)
      write(fd,(myblocks + myinode[inode_no].block_ptr[i]*BLOCK_SZ),BLOCK_SZ);
    for(i=0;i<(x-8);i++){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + i*sizeof(int)));
      write(fd,(myblocks + z*BLOCK_SZ),BLOCK_SZ);
    }
    if(x==(sub_blocks+8) && y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ));
      z = *((int*)(myblocks + z*BLOCK_SZ));
      write(fd,(myblocks + z*BLOCK_SZ),y);
    }
    else if(y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int)));
      write(fd,(myblocks + z*BLOCK_SZ),y);
    }
  }
  else if(x<=(sub_blocks*sub_blocks + sub_blocks + 8)){
    for(i=0;i<8;i++)
      write(fd,(myblocks + myinode[inode_no].block_ptr[i]*BLOCK_SZ),BLOCK_SZ);
    for(i=0;i<sub_blocks;i++){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + i*sizeof(int)));
      write(fd,(myblocks + z*BLOCK_SZ),BLOCK_SZ);
    }
    for(i=0;i<(x-sub_blocks-8)/sub_blocks;i++){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      printf("Z=%d\n",z);
      for(j=0;j<sub_blocks;j++){
        z_ = *((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)));
        write(fd,(myblocks+z_*BLOCK_SZ),BLOCK_SZ);
      }
    }
    if((x-sub_blocks-8)%sub_blocks >0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      for(j=0;j<((x-sub_blocks-8)%sub_blocks);j++){
        z_ = *((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)));
        write(fd,(myblocks+z_*BLOCK_SZ),BLOCK_SZ);
      }
      if(y>0){
        z_ = *((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)));
        write(fd,(myblocks+z_*BLOCK_SZ),y);
      }
    }
    else if(y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      z_ = *((int*)(myblocks + z*BLOCK_SZ ));
      write(fd,(myblocks+z_*BLOCK_SZ),y);
    }
  }
  if(strcmp(dest,"stdout"))
    close(fd);
}

// display file on terminal
int showfile_myfs(char *filename){
  return copy_myfs2pc(filename,"stdout");
}

// make new directory
int mkdir_myfs(char *dirname){
  int inode_no;
  inode_no = get_freeInode();
  if(inode_no==-1)
    return -1;
  newInode(inode_no,1,0);
  add_file2dir(cwd,dirname,inode_no);     // add created directory to current directory's list
  add_file2dir(inode_no,"..",cwd);        // add current directory as parent in created directory's list
  return 0;
}

// change directory
int chdir_myfs(char *dirname){
  int inode_no;
  inode_no = getFileInode(dirname,0);
  if(inode_no==-1){
    printf("Directory doesn't exist!\n");
    return -1;
  }
  cwd = inode_no;
  return 0;
}

// remove directory from current directory
int rmdir_myfs(char *dirname){
  int folder_no,file_no,temp_cwd,x,y,i,j,sz;
  char fname[28];
  folder_no = getFileInode(dirname,0);
  if(folder_no==-1)
    return -1;
  x = (myinode[folder_no].file_size)/BLOCK_SZ;
  y = (myinode[folder_no].file_size)%BLOCK_SZ;

  if(x<8){
    for(i=0;i<=x;i++){
      sz = (i==x)? y : BLOCK_SZ;
      for(j=0;j<sz;j+=32){
        strcpy(fname,(char*)(myblocks + myinode[folder_no].block_ptr[i]*BLOCK_SZ +j));
        file_no = *((int*)(myblocks + myinode[folder_no].block_ptr[i]*BLOCK_SZ + j + 28));
        temp_cwd = cwd;
        cwd = folder_no;
        if(myinode[file_no].file_type == 1 && strcmp("..",fname)!=0)      // if folder then recursively delete
          rmdir_myfs(fname);
        if(myinode[file_no].file_type == 0)                       // if file then remove
          rm_myfs(fname);
        cwd = temp_cwd;
      }
    }
  }
  else if(x==8){
    //
  }
  rm_myfs(dirname);           // remove directory from current directory
  return 0;
}

// open file
int open_myfs(char *filename, char mode){
  int inode_no,newfile,retval;
  inode_no = getFileInode(filename,0);
  if(inode_no == -1){         // file not already present
    if(mode=='r')
      return -1;
    if(mode=='w'){
      newfile = get_freeInode();
      newInode(newfile,0,0);
      add_file2dir(cwd,filename,newfile);
    }
  }
  else{                       // file present in directory
    if(mode=='r')
      newfile = inode_no;
    if(mode=='w'){
      rm_myfs(filename);
      newfile = get_freeInode();
      newInode(newfile,0,0);
      add_file2dir(cwd,filename,newfile);
    }
  }
  sem_wait(sem);
  retval = create_file_entry(newfile,0,mode);
  sem_post(sem);
  return retval;
}

// close file
int close_myfs(int fd){
  if(fd<0 || fd>=MAX_OPEN_FILES || myfiles[fd]==NULL)
    return -1;
  myfiles[fd] = NULL; //semaphore
  open_files--;
  return 0;
}

// read from open file
int read_myfs(int fd, int nbytes, char *buff){
  int x,y,z,sub_blocks,inode_no,db_no,wrbyte;
  if(fd<0 || fd>=MAX_OPEN_FILES || myfiles[fd]==NULL || myfiles[fd]->mode!='r')
    return -1;
  inode_no = myfiles[fd]->inode_no;
  if(nbytes == 0 || myfiles[fd]->offset == myinode[inode_no].file_size)
    return 0;

  sub_blocks = BLOCK_SZ/sizeof(int);
  x = myfiles[fd]->offset/BLOCK_SZ;
  y = myfiles[fd]->offset%BLOCK_SZ;
  z = myinode[inode_no].file_size - myfiles[fd]->offset;

  wrbyte = (nbytes<z)? nbytes : z;
  wrbyte = (wrbyte<(BLOCK_SZ-y))? wrbyte : (BLOCK_SZ-y);

  if(x<8)
    db_no = myinode[inode_no].block_ptr[x];
  else if(x<(sub_blocks+8))
    db_no = *((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + (x-8)*sizeof(int)));
  else if(x<(sub_blocks*sub_blocks + sub_blocks + 8)){
    z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int)));
    db_no = *((int*)(myblocks + z*BLOCK_SZ + ((x-sub_blocks-8)%sub_blocks)*sizeof(int) ));
  }
  else{
    //
  }
  memcpy(buff,(myblocks + db_no*BLOCK_SZ + y),wrbyte);
  myfiles[fd]->offset += wrbyte;
  return (wrbyte + read_myfs(fd,(nbytes-wrbyte),buff+wrbyte));
}

// write to open file
int write_myfs(int fd, int nbytes, char *buff){
  int inode_no, initial_offset;
  block db;

  if(fd<0 || fd>=MAX_OPEN_FILES || myfiles[fd]==NULL || myfiles[fd]->mode!='w')
    return -1;

  inode_no = myfiles[fd]->inode_no;
  initial_offset = myfiles[fd]->offset;

  while(nbytes>0){
    db = newDB(inode_no);
    if(db.empty==NULL)
      return (myfiles[fd]->offset - initial_offset);
    if(nbytes<=db.nbytes){
      memcpy(db.empty,buff,nbytes);
      myinode[inode_no].file_size += nbytes;
      myfiles[fd]->offset += nbytes;
      nbytes = 0;
    }
    else{
      memcpy(db.empty,buff,db.nbytes);
      nbytes -= db.nbytes;
      buff += db.nbytes;
      myinode[inode_no].file_size += db.nbytes;
      myfiles[fd]->offset += db.nbytes;
    }
  }
  return (myfiles[fd]->offset - initial_offset);
}

// check end-of-file
int eof_myfs(int fd){
  if(fd<0 || fd>=MAX_OPEN_FILES || myfiles[fd]==NULL)
    return -1;
  return (myfiles[fd]->offset == myinode[myfiles[fd]->inode_no].file_size)? 1:0;
}

// dump mrfs to disk
int dump_myfs(char *dumpfile){
  int fd;
  fd = open(dumpfile,O_WRONLY|O_CREAT,0777);
  if(fd==-1)
    return -1;
  if(write(fd,myfs,iptr[0]*MEGA) == -1)
    return -1;
  return close(fd);
}

// restore mrfs from disk
int restore_myfs(char *dumpfile){
  struct stat ast;
  int fd,fsize,i,shmid1,shmid2;
  char *cptr;
  key_t key1,key2;

  sem_unlink ("mutex");
  sem_unlink ("mutex2");
  /* initialize semaphores for shared processes */
    sem = sem_open ("mutex", O_CREAT | O_EXCL, 0644, 1);
    sem2 = sem_open ("mutex2", O_CREAT | O_EXCL, 0644, 1);
  fd = open(dumpfile,O_RDONLY);
  if(fd==-1)
    return -1;
  fstat(fd,&ast);
  fsize = ast.st_size;

  key1 = 1234;
  shmid1 = shmget(key1,fsize,IPC_CREAT|0666);
  if(shmid1<0){
    perror("shmget1\n");
    return -1;
  }
  myfs = shmat(shmid1,NULL,0);
  read(fd,myfs,fsize);
  iptr = (int *)myfs;
  cptr = (char*)(iptr+6);
  inode_bmap = cptr;
  block_bmap = cptr+INODE;
  myinode = (inode*)(cptr+iptr[1]+iptr[3]);   // myinode points to start of inode list
  myblocks = (void*)(myinode+INODE);
  cwd = iptr[5];
  open_files = 0;

  key2 = 8080;
  shmid2 = shmget(key2,sizeof(file*)*MAX_OPEN_FILES,0666|IPC_CREAT);
  if(shmid2<0){
    perror("shmget2\n");
    return -1;
  }
  myfiles = (file**)shmat(shmid2,NULL,0);

  for(i=0;i<MAX_OPEN_FILES;i++)
    myfiles[i]=NULL;
  return 0;
}

// change access mode of file
int chmod_myfs(char *name, int mode){
  int inode_no;
  inode_no = getFileInode(name,0);
  if(inode_no==-1)
    return -1;
  myinode[inode_no].access_permission = (short)mode;
  return 0;
}
#endif
