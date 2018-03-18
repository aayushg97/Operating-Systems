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
  printf("Directory mydocs contents:\n");
  ls_myfs();
  chdir_myfs("..");
  printf("Directory myroot contents:\n");
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
    printf("P2 || Enter file to be copied under /mycode: ");
    scanf("%s",fname);
    copy_pc2myfs(fname,fname);
  }
  sleep(1);
  printf("Directory mycode contents:\n");
  ls_myfs();
  printf("Displaying contents of file %s: \n",fname);
  showfile_myfs(fname);
  chdir_myfs("..");
  chdir_myfs("mydocs");
  chdir_myfs("mytext");
  printf("Directory mytext contents:\n");
  ls_myfs();
  showfile_myfs("alphabet.txt");
  return 0;
}
