#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {

  int fd,*fds,no,N,i;
  char fname[28]="mytest-xx.txt";
  printf("Creating MRFS of 10 MB.\n");
  create_myfs(10);

  printf("Writing 100 random integers to file mytest.txt ...\n");
  fd = open_myfs("mytest.txt",'w');
  for(i=0;i<100;i++){
    no = rand();
    if(write_myfs(fd,sizeof(no),&no)==-1)
      printf("Failed to write\n");
  }
  close_myfs(fd);
  printf("Done.\n Displaying contents of directory:-\n\n ");
  ls_myfs();
  printf("\nEnter number of copies(max 50): ");
  scanf("%d",&N);
  fds = (int*)malloc((N+1)*sizeof(int));
  fd = open_myfs("mytest.txt",'r');

  printf("Creating %d copies ...\n",N);
  for(i=1;i<=N;i++){
    fname[7] = (i<10)? '0': '0'+(i/10);
    fname[8] = '0'+(i%10);
    fds[i] = open_myfs(fname,'w');
  }
  while(!eof_myfs(fd)){
    read_myfs(fd,sizeof(no),&no);
    for(i=1;i<=N;i++){
        if(write_myfs(fds[i],sizeof(no),&no)==-1)
          printf("Failed to create copy\n");
      }
    }
  for(i=1;i<=N;i++)
    close_myfs(fds[i]);
  printf("Done.\n Displaying contents of directory:-\n\n ");
  ls_myfs();
  printf("\n Dumping MFRS to PC ...\n");
  dump_myfs("mydump-1.backup");
  printf("Done.\n");
  if(clear_myfs())
   printf("\nCleared file system\n");
 
  return 0;
}
