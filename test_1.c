#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {
  char fname[28];
  int i;
  create_myfs(10);
  status_myfs();
  int fd = open_myfs("mytest.txt",'w');
  ls_myfs();
  status_myfs();
  char arr[100];
  for(i=0;i<100;i++)
    arr[i]=('a'+i%26);
  printf("Check 1\n");
  write_myfs(fd,sizeof(arr),(char*)arr);
  printf("Check 2\n");
  close_myfs(fd);
  printf("Check 3\n");
  status_myfs();

  fd = open_myfs("mytest.txt",'r');
  printf("Check 4\n");
  int fd1 = open_myfs("mytest-1.txt",'w');
  printf("Check 5\n");
  ls_myfs();
  status_myfs();

  char ch;
  while(!eof_myfs(fd)){
    read_myfs(fd,1,&ch);
    write_myfs(fd1,1,&ch);
  }

  close_myfs(fd);
  close_myfs(fd1);

  showfile_myfs("mytest-1.txt");
  ls_myfs();
  status_myfs();

  /*
  mkdir_myfs("dir1");
  ls_myfs();
  status_myfs();
  chdir_myfs("dir1");
  copy_pc2myfs("a.c","copy1.c");
  ls_myfs();
  status_myfs();
  chdir_myfs("..");
  rmdir_myfs("dir1");
  ls_myfs();
  status_myfs();
  /*copy_pc2myfs("b.c","copy2.c");
  //status_myfs();
  copy_pc2myfs("c.c","copy3.c");
  //status_myfs();
  copy_pc2myfs("new","copy4.c");
  //status_myfs();
  copy_pc2myfs("d.c","copy5.c");
  //status_myfs();/*
  copy_pc2myfs("e.c","copy6.c");
  //status_myfs();
  copy_pc2myfs("f.c","copy7.c");
  //status_myfs();
  copy_pc2myfs("g.c","copy8.c");
  //status_myfs();
  copy_pc2myfs("h.c","copy9.c");
  //status_myfs();
  copy_pc2myfs("first.sql","copy10.c");
  //status_myfs();
  copy_pc2myfs("second.sql","copy11.c");
  //status_myfs();
  copy_pc2myfs("seven.sql","copy12.c");
  //status_myfs();
  ls_myfs();
  printf("Enter file to be deleted:");
  scanf("%s",fname);
  rm_myfs(fname);*/
  //showfile_myfs("copy1.c");
  //copy_myfs2pc("copy4.c","copied_back");
  //ls_myfs();
  //status_myfs();
  return 0;
}
