#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {
  char fname[28];
  int i,fd;
  char ch;
  printf("Creating MRFS of 10 MB.\n");
  create_myfs(10);
  printf("Done. Displaying status information:-\n");
  status_myfs();
  mkdir_myfs("mycode");
  mkdir_myfs("mydocs");
  chdir_myfs("mydocs");
  mkdir_myfs("mytext");
  mkdir_myfs("mypapers");
  printf("\nDirectory mydocs contents:\n");
  ls_myfs();
  chdir_myfs("..");
  printf("\nDirectory myroot contents:\n\n");
  ls_myfs();
  if(fork()==0){
    //p1
    printf("P1 writing A-Z in /mydocs/mytext/alphabet.txt\n");
    chdir_myfs("mydocs");
    chdir_myfs("mytext");
    fd = open_myfs("alphabet.txt",'w');
    for(i=0;i<26;i++){
      ch = 'A'+i;
      write_myfs(fd,sizeof(ch),&ch);
    }
    close_myfs(fd);
    printf("P1 exiting ...\n");
    exit(0);
  }else{
    //p2
    chdir_myfs("mycode");
    printf("\nP2 says Enter file to be copied under /mycode:\n");
    scanf("%s",fname);
    if(copy_pc2myfs(fname,fname)==-1)
      printf("Failed to copy.\n");
    else
      printf("Copied.\n");
  }
  sleep(1);
  printf("\nDirectory mycode contents:\n");
  ls_myfs();
  printf("\nDisplaying contents of file %s: \n\n",fname);
  showfile_myfs(fname);
  chdir_myfs("..");
  chdir_myfs("mydocs");
  chdir_myfs("mytext");
  printf("\nDirectory mytext contents:\n");
  ls_myfs();
  printf("\nDisplaying contents of file alphabet.txt:\n");
  showfile_myfs("alphabet.txt");
  if(clear_myfs())
   printf("\nCleared file system\n");
  return 0;
}
