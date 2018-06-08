#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MEMSZ 6*sizeof(int)
#define MAX_PRIME 1000

int prime(int *arr){
	int i,j,flag,cnt=0;	
	for(i=2;i<=MAX_PRIME;i++){
		flag=1;
		for(j=2;j<i;j++){
			if(i%j==0){
				flag=0;
				break;
			}
		}
		if(flag)
			arr[cnt++]=i;
	}
	return cnt;
}

int main(){
	
	int nc,np,x,i,j,no,arr[6000],n;
   	key_t key;
    	int *shm, *s, shmid;
    	struct shmid_ds *buf;
    	struct timeval tv;
    	double start_time,current_time;
    	time_t rawtime;
	
	printf("Number of producers: ");
	scanf("%d",&np);	
	printf("Number of consumers: ");
	scanf("%d",&nc);
	
	key = 1234;
	shmid = shmget(key, MEMSZ, IPC_CREAT |0666);
	shm = shmat(shmid, NULL, 0);					//attaching shared memory
	shmctl(shmid, SHM_LOCK, buf);
	
	for(i=0;i<6;i++)
		shm[i]=0;
	
	n = prime(arr);
		
	gettimeofday(&tv, NULL);	
	start_time =  tv.tv_sec + tv.tv_usec/1000000.0;
	time(&rawtime);
	
	printf("START TIME: %s\n",ctime(&rawtime));
		
	// creation of producer processes
	for(i=0;i<np;i++){
		x = fork();
		if(x==0){
		   while(1){	
			srand(time(NULL)+100*i);
			no = arr[rand()%n];
			sleep((rand()%5) + 1);
			if(shm[0]>=5)
				printf("Buffer full ... Waiting for read.\n");
			while(shm[0]>=5);
			for(j=1;j<6;j++){
				if(shm[j]==0){
					shm[j]=no;
					shm[0]+=1;
					gettimeofday(&tv, NULL);
					current_time =  tv.tv_sec + tv.tv_usec/1000000.0;
					printf("Producer %d: %d, time: %lf\n",i+1,no,(current_time-start_time));
					break;
					}
			}
		    }		
			exit(0);
		}	
	}
	
	// creation of consumer processes	
	for(i=0;i<nc;i++){
		x = fork();
		if(x==0){
			while(1){	
			srand(time(NULL)+100*i);
			sleep((rand()%5) + 1);
			if(shm[0]==0)
				printf("Buffer empty ... Waiting for write.\n");
			while(shm[0]==0);				
			for(j=1;j<6;j++){
				if(shm[j]!=0){
					shm[0]-=1;
					gettimeofday(&tv, NULL);
					current_time =  tv.tv_sec + tv.tv_usec/1000000.0;;
					printf("Consumer %d: %d, time: %lf\n",i+1,shm[j],(current_time-start_time));
					shm[j]=0;
					break;
					}
			}
		    }		
			exit(0);
		}
	}
	
	sleep(30);
	time(&rawtime);
	shmdt(shm);				// detaching shared memory
	printf("\nEND TIME: %s\nKilling child processes.\n",ctime(&rawtime));
	kill(0,SIGTERM);
	return 0;	
}
