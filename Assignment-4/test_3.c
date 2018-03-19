#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {

  int fd,arr[100],no,N,i,j;
  char buf[100];
  printf("Restoring mydump-1.backup ...\n");
  if( restore_myfs("mydump-1.backup") ==-1){
      printf("Failed to restore backup\n");
      exit(0);
  } 
  printf("Done.\n");

  printf("\n Displaying contents of directory:-\n\n ");
  ls_myfs();

  printf("Reading integers from file mytest.txt ...\n");
  fd = open_myfs("mytest.txt",'r');
  if(fd==-1){
    printf("Failed to open mytest.txt\n");
    exit(0);
  }
  for(i=0;i<100;i++){
    if(read_myfs(fd,sizeof(no),&no)==-1){
      printf("Failed to read from file\n");
      exit(0);
    }
    arr[i] = no;
    printf("%d\n",no);
  }
  close_myfs(fd);
  printf("Sorting the numbers ...\n");
  for(i=0;i<100;i++)
    for(j=i+1;j<100;j++)
      if(arr[j]<arr[i]){
        N = arr[i];
        arr[i]=arr[j];
        arr[j]=N;
      }

  printf("\nDone. Writing to sorted.txt ...\n");
  fd = open_myfs("sorted.txt",'w');
  if(fd==-1){
    printf("Failed to open sorted.txt\n");
    exit(0);
  }
  for(i=0;i<100;i++){
    snprintf(buf,100,"%d",arr[i]);
    N = strlen(buf);
    buf[N] = '\n';
    if(write_myfs(fd,N+1,buf)==-1){
      printf("Failed to write to file\n");
      exit(0);
    }
  }
  close_myfs(fd);
  printf("Done.\n Displaying contents of directory:-\n\n ");
  ls_myfs();
  printf("\nDisplaying contents of sorted.txt:-\n");
  showfile_myfs("sorted.txt");
  if(clear_myfs())
   printf("\nCleared file system\n");
  return 0;
}
