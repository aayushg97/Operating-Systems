#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {
  char fname[28];
  create_myfs(10);
  status_myfs();
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
