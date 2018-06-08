#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
	char opt;
	int a,x,i,j,n;
	int new_fds[2],old_fds[2];
	char cmd[100],cwd[100],*args[10],*args2[10],*pt;
	char s1[100],s2[100];
		
	printf("******** SHELL ************\n");
	printf("A : Run an internal command\n");
	printf("B : Run an external command\n");
	printf("C : Run an external command by redirecting standard input from a file\n");
	printf("D : Run an external command by redirecting standard output to a file\n");
	printf("E : Run an external command in the background\n");
	printf("F : Run several external commands in the pipe mode\n");
	printf("G : Quit the shell\n");
	printf("***************************\n\n");
	
	while(1){		
		scanf(" %c",&opt);			// reading user choice
		switch(opt){
			case 'A': 			// internal command
				scanf("%s %s",s1,s2);
				x = fork();
				if(x==0){
					if(strcmp(s1,"mkdir")==0)
						mkdir(s2,0777);				// make directory with read,write & execute permissions for everyone
					else{
						if(strcmp(s1,"chdir")==0){
							chdir(s2);			// change directory
							getcwd(cwd,sizeof(cwd));	// get the current working directory
							printf("%s \n",cwd);
						}
						else if(strcmp(s1,"rmdir")==0)
							rmdir(s2);			// remove directory
					}
					return 0;
				}
				else waitpid(x,NULL,WCONTINUED);	
				break;				
			case 'B': 			// external command			
				scanf(" %[^\n]%*c",cmd);
				x = fork();
				if(x==0)
					execlp("/bin/sh","/bin/sh","-c",cmd,(char *)0);
				else{
					waitpid(x,NULL,WCONTINUED);	
				}
				
				break;
			case 'C': 			// input redirection
				scanf(" %[^<\n] < %[^<\n]",s1,s2);
				x = fork();
				if(x==0){
					i = open(s2,O_RDONLY);				// create new open file descriptor in read-only mode
					a = close(0);					// close stdin file descriptor
					j = dup(i);					// duplicate fd for input file inplace of stdin
					execlp("/bin/sh","/bin/sh","-c",s1,(char *)0);
				}
				else waitpid(x,NULL,WCONTINUED);	
				
				break;
			case 'D': 			// output redirection
				scanf(" %[^>\n] > %[^>\n]",s1,s2);
				x = fork();
				if(x==0){
				  i = open(s2, O_WRONLY);				// create new open file descriptor in write-only mode
 				  a = close(1);						// close stdout file descriptor
 				  j = dup(i);						// duplicate fd for output file inplace of stdout
 				  execlp("/bin/sh","/bin/sh","-c",s1,(char *)0);
				}
				else waitpid(x,NULL,WCONTINUED);	
				
				break;
			case 'E': 			// background process
				scanf(" %[^&\n] &",cmd);
				x = fork();
				if(x==0){
					execlp("/bin/sh","/bin/sh","-c",cmd,(char *)0);	// no wait in parent process
				}
				
				
				break;
			case 'F': 			// commands in pipe
				n=0;
				scanf(" %[^\n]",cmd);
				pt = strtok(cmd,"|");
				while(pt!=NULL){	// separating commands in pipe
					args[n++]=pt;
					pt=strtok(NULL,"|");
					//printf("%s\n",args[n-1]);
				}

				for(i=0;i<n;i++){
				
					if(i!=(n-1))					// create new pipe if not the last command	
						pipe(new_fds);
					x=fork();
					if(x==0){	
						// child process
						if(i!=0){				// if not the first command
							dup2(old_fds[0], 0);		// duplicate fd for input end of pipe inplace of stdin
							close(old_fds[0]);		// close fd for input pipe end
							close(old_fds[1]);		// close fd for output pipe end
						}
						if(i!=(n-1)){				// if not the last command
							close(new_fds[0]);		// close fd for input pipe end
							dup2(new_fds[1], 1);		// duplicate fd for output end of pipe inplace of stdout
							close(new_fds[1]);		// close fd for output pipe end	
						}
						/*
							// using execvp
						pt = strtok(args[i]," ");
						j=0;
						while(pt!=NULL){
							//printf("");
							args2[j++] = pt;
							pt=strtok(NULL," ");
							printf("%s",args2[j-1]);
						}
						args2[j] = NULL;
						execvp(args2[0],args2);
						*/
						execlp("/bin/sh","/bin/sh","-c",args[i],(char *)0);	// execute pipe command
						return 0;
					}
					else{	
						// parent process
						if(i!=0){				// close pipe ends if not the first command
							close(old_fds[0]);
							close(old_fds[1]);
						}
						if(i!=(n-1)){				// copy previous fds if not the last command
							old_fds[0] = new_fds[0];	
							old_fds[1] = new_fds[1];
							}
					}	
				}
				if(n>1){
					close(old_fds[0]);
					close(old_fds[1]);
				}
				
				break;
				
			case 'G': return 0;				// QUIT
			default : printf("wrong option %d\n",(int)opt); break;
		}
	}
	
	return 0;
}
