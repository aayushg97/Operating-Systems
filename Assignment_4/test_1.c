#include <stdio.h>
#include "myfs.h"

int main(int argc, char const *argv[]) {
  int i,N;
  char fname[28];
  printf("Creating MRFS of 10 MB.\n");
  create_myfs(10);
  printf("Done. Displaying status information:-\n");
  status_myfs();
   printf("\nEnter number of files to you want to copy: ");
   scanf("%d",&N);

   for (i=0;i<N;i++) {
     printf("Enter file name: ");
     scanf("%s",fname);
     copy_pc2myfs(fname,fname);
     printf("File copied.\n");
   }

   printf("Displaying contents of the directory:\n");
   ls_myfs();
   printf("Enter file to be deleted:");
   scanf("%s",fname);
   rm_myfs(fname);
   printf("Displaying contents of the directory after deletion:\n");
   ls_myfs();
  return 0;
}
