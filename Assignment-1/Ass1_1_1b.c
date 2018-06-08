#include <stdio.h>
#include <unistd.h>

int main(){
  int x;
  char cmd[100];

  while(1){
    scanf(" %[^\n]%*c",cmd);
    if(strcmp(cmd,"quit")==0)
      break;
    x = fork();
    if(x==0){
      execlp("dash","dash","-c",cmd,0);   // /bin/sh
    }
  }

  return 0;
}
