#ifndef MYFS_H
#define MYFS_H

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MEGA (1024*1024)
#define INODE 1024
#define BLOCK_SZ 256
#define MAX_OPEN_FILES 100

typedef struct {
  short file_type;
  short access_permission;
  int file_size;
  time_t last_modified;
  time_t last_accessed;
  int block_ptr[10];
} inode;

typedef struct{
  int inode_no;
  int offset;
} file;

typedef struct{
  int no;
  int nbytes;
  void *empty;
} block;

void * myfs,* myblocks;
int * iptr, cwd, ofile_index;
char *cptr, *inode_bmap, *block_bmap;
inode * myinode;
file *myfiles[MAX_OPEN_FILES];
int init[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

short bin2short(char *bin){
  short access=0;
  int i;
  for(i=0;i<9;i++)
    access = 2*access + (bin[i]-'0');
  return access;
}

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

file* create_file_entry(int inode_no, int offset){
  file *f = (file*)malloc(sizeof(file));
  f->inode_no = inode_no;
  f->offset = offset;
  return f;
}

int get_freeInode(){
  int i;
  for (i = 0; i < iptr[1]; i++) {
    if(inode_bmap[i]==0){
      inode_bmap[i]=1;
      iptr[2]++;
      return i;
    }
  }
  return -1;
}

int get_freeDB(){
  int i;
  for (i = 0; i < iptr[3]; i++) {
    if(block_bmap[i]==0){
      block_bmap[i]=1;
      //iptr[4]++;
      return i;
    }
  }
  return -1;
}

// initialise newly created file or folder inode
void newInode(int inode_no,short file_type,short file_size){
  myinode[inode_no].file_type = file_type;
  myinode[inode_no].file_size = file_size;
  time(&myinode[inode_no].last_accessed);
  time(&myinode[inode_no].last_modified);
  myinode[inode_no].access_permission = bin2short("111000000");   // default access_permission
  memcpy(myinode[inode_no].block_ptr,init,sizeof(init));
}

block newDB(int inode_no){
  block db;
  int fsize,x,y,z,db_no,sub_blocks;
  sub_blocks = BLOCK_SZ/sizeof(int);
  fsize = myinode[inode_no].file_size;
  //printf("fsz %d\n",fsize );
  x = fsize/BLOCK_SZ;
  y = fsize%BLOCK_SZ;
  if(x<8){
    if(y==0){
      //printf("New block! %d\n",myinode[inode_no].block_ptr[x]);
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
    //  printf("db1 %d\n",db_no );
      myinode[inode_no].block_ptr[9] = db_no;
    }
    if((x-sub_blocks-8)%sub_blocks==0 && y==0){
        db_no = get_freeDB();
    //    printf("db2 %d\n",db_no );
        *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int))) = db_no;
    }
    if(y==0){
      db_no = get_freeDB();
    //  printf("db3 %d\n",db_no );
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int)));
      *((int*)(myblocks + z*BLOCK_SZ + ((x-sub_blocks-8)%sub_blocks)*sizeof(int) )) = db_no;
    }
    z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + ((x-sub_blocks-8)/sub_blocks)*sizeof(int)));
    db.no = *((int*)(myblocks + z*BLOCK_SZ));
  //  printf("DB SENT %d\n",db.no);
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

void add_file2dir(int folder_no, char *fname,int file_no){
  block db;
  db = newDB(folder_no);
  if(db.nbytes < 32){
    printf("Unexpected error!\n");
  }
  strcpy((char*)db.empty,fname);
  *((int*)(db.empty+28)) = file_no;
  myinode[folder_no].file_size += 32;
  // last_accessed folder
}

int getFileInode(char* fname,int rem_file){
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
    if(retval!=-1)
      return retval;
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
  return retval;
}

int create_myfs (int size){
  int i;
  myfs = malloc(size*MEGA);
  if(myfs == NULL)
    return -1;

  iptr = (int *)myfs;
  iptr[0] = size;            // total file sys size
  iptr[1] = INODE;           // max no of inodes supported
  iptr[2] = 0;               // actual no of inodes used
  //printf("%d\n",( MEGA*size - ( 5*sizeof(int) + INODE*( sizeof(char)+sizeof(inode) ) ) ) / ( sizeof(char)+BLOCK_SZ ) );
  // calculating total no of data-blocks
  iptr[3] = ( MEGA*size - ( 5*sizeof(int) + INODE*( sizeof(char)+sizeof(inode) ) ) ) / ( sizeof(char)+BLOCK_SZ );
  iptr[4] = 0;               // no of blocks used
  //printf("Hilo\n");
  cptr = (char*)(iptr+5);
  inode_bmap = cptr;
  block_bmap = cptr+INODE;
  //printf("%d\n",(INODE+iptr[3]));
  // initialise bitmap for both free inodes and diskblocks
  for ( i = 0; i < (INODE+iptr[3]); i++)
    cptr[i]=0;
//  printf("Hilo3\n");
  myinode = (inode*)(cptr+i);   // myinode points to start of inode list
  myblocks = (void*)(myinode+INODE);
  // printf("Hilo4\n");
  // create home directory
  cwd = get_freeInode();        // current working directory set to home
  newInode(cwd,1,0);
  /*
  myfiles[0] = create_file_entry(0,0);
  ofile_index = 1;            // stores the 1 + index of the last entry in file table
  */
  return 0;
}

int copy_pc2myfs(char *source, char *dest){
  struct stat ast;
  int fsize, fd, inode_no;
  block db;
  fd = open(source,O_RDONLY);
  fstat(fd,&ast);
  fsize = ast.st_size;
  //printf("ffff %d\n",fsize);
  if(iptr[2] == iptr[1]){
    printf("All inodes used: %d\n",iptr[2]);
    return -1;
  }
  if( (iptr[3]-iptr[4])*BLOCK_SZ < fsize ){
    printf("Not enough space! Required = %d B | Available = %d B\n",fsize,(iptr[3]-iptr[4])*BLOCK_SZ);
    return -1;
  }

  // check for extra large files here

  inode_no = get_freeInode();
  newInode(inode_no,0,0);

  while(fsize>0){
    db = newDB(inode_no);
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
  add_file2dir(cwd,dest,inode_no);
  return 0;
}

int ls_myfs(){
  char name[28];
  int inode_no,x,y,i,j;
  printf("| name\t| size(B)\t| type\t| last_modified\t| last_accessed\t| access_permission |\n");
  printf("----------------------------------------------------------------------------------\n");

  //no_files = (myinode[cwd].file_size)/32;
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

int rm_myfs(char *filename){
  update_super();
  printf("deleting %s\n",filename);
  //printf("blocs = %d\n",iptr[4]);
  int inode_no,x,y,z,i,j,sub_blocks;
  inode_no = getFileInode(filename,1);
  if(inode_no==-1)
    return -1;
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
    update_super();
    printf("blocs = %d\n",iptr[4]);
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
      update_super();
      printf("blocs = %d\n",iptr[4]);
  }
  else if(x<=(sub_blocks*sub_blocks + sub_blocks + 8)){
    iptr[4] += 3;
    for(i=0;i<=9;i++)
      block_bmap[myinode[inode_no].block_ptr[i]] = 0;
    update_super();
//    printf("a blocs = %d\n",iptr[4]);

    for(i=0;i<(sub_blocks);i++)
      block_bmap[*((int*)(myblocks + myinode[inode_no].block_ptr[8]*BLOCK_SZ + i*sizeof(int)))] = 0;
      update_super();
    //  printf("b blocs = %d\n",iptr[4]);

    for(i=0;i<(x-sub_blocks-8)/sub_blocks;i++){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
    //  printf("Z=%d\n",z);
      block_bmap[z] = 0;
      for(j=0;j<sub_blocks;j++){
      //  printf("bval %c \n",block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))]+'0' );
      //  printf("Block No %d\n",*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int))) );
        block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
        update_super();
      //  printf("c blocs = %d\n",iptr[4]);
      }
    }
    if((x-sub_blocks-8)%sub_blocks >0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      block_bmap[z] = 0;
      for(j=0;j<((x-sub_blocks-8)%sub_blocks);j++)
        block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
        if(y>0)
          block_bmap[*((int*)(myblocks + z*BLOCK_SZ + j*sizeof(int)))] = 0;
          update_super();
        //  printf("d blocs = %d\n",iptr[4]);

    }
    else if(y>0){
      z = *((int*)(myblocks + myinode[inode_no].block_ptr[9]*BLOCK_SZ + i*sizeof(int)));
      block_bmap[z] = 0;
      block_bmap[*((int*)(myblocks + z*BLOCK_SZ ))] = 0;
      update_super();
    //  printf("e blocs = %d\n",iptr[4]);

    }
  }
  update_super();
  //printf("blocs = %d\n",iptr[4]);
  inode_bmap[inode_no] = 0;
  return 0;
}

int status_myfs(){
  int i;
  update_super();
  double used=0,freespace;
  printf("Block used: %d / %d\n",iptr[4],iptr[3]);
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

}

int copy_myfs2pc(char *source, char *dest){
  int fd,x,y,z,z_,inode_no,sub_blocks,i,j;
  if(!strcmp(dest,"stdout"))
    fd = 1;
  else
    fd = open(dest,O_WRONLY|O_CREAT,0777);
  inode_no = getFileInode(source,0);
  if(inode_no==-1)
    return -1;
  //printf("Humpty Dumpty\n");
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

int showfile_myfs(char *filename){
  return copy_myfs2pc(filename,"stdout");
}

int mkdir_myfs(char *dirname){
  int inode_no;
  inode_no = get_freeInode();
  if(inode_no==-1)
    return -1;
  newInode(inode_no,1,0);
  add_file2dir(cwd,dirname,inode_no);
  add_file2dir(inode_no,"..",cwd);
  return 0;
}

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
        if(myinode[file_no].file_type == 1 && strcmp("..",fname)!=0)
          rmdir_myfs(fname);
        if(myinode[file_no].file_type == 0)
          rm_myfs(fname);
        cwd = temp_cwd;
      }
    }
  }
  else if(x==8){
    //
  }
  rm_myfs(dirname);
  return 0;
}

#endif
