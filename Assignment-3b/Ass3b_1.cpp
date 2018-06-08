#include <bits/stdc++.h>
#include <cstdlib>
#include <unistd.h>
using namespace std;

int *status;
int n,totp,cthr;

void signal_sleep(int signum){
	signal(SIGUSR1,signal_sleep);		// Signal to put a thread to sleep
	pause();
}

void signal_wake(int signum){
	signal(SIGUSR2,signal_wake);		// Signal to wake up a thread
}


/*
	worker thread
*/
void *worker(void *param){
	long curr_thread_ind;
	curr_thread_ind = (long)param;
	status[curr_thread_ind] = 0;
	signal(SIGUSR1,signal_sleep);
	signal(SIGUSR2,signal_wake);
	pthread_kill(pthread_self(),SIGUSR1);
	
	int arr[1000];
	long long temp=0,max;
	max = (rand()%400000)+1;
	max *= 10000;
	
	for(int i=0;i<1000;i++){			//
		srand(time(NULL)+i);			//	Populating 1000 random integers
		arr[i] = rand()%10000 + 1;		//
	}									//
	
	sort(arr,arr+1000);					// Sorting 1000 random integers

	while(temp<max){					//
		temp++;							// Sleep of random time between 0 and 10 seconds
	}									//
	
	totp--;
	status[curr_thread_ind] = -1;		// change status of thread to exit
	pthread_exit(NULL);
}

/*
	Scheduler thread
*/
void *scheduler(void *param){	
	int flag = 0;		
	time_t start_time,rawtime;
	
	time(&rawtime);
	printf("Scheduler thread started at time %s\n",ctime(&rawtime));
	
	while(totp){													// While total waiting threads < n
		usleep(10);
		if(status[cthr]!=-1){			
			//printf("(Scheduler)Thread %d :- Start = %d",cthr,(int)time(NULL));
			pthread_kill(*((pthread_t *)param+cthr),SIGUSR2);		// wake up the current thread
			status[cthr] = 1;										// change status of current thread
						
			start_time = time(NULL);
			while(time(NULL)-start_time<1){							//
				if(status[cthr]==-1){								//
					flag = 1;										//	Allow current thread to execute for 1 second
					break;											//
				}													//
			}														//
			
			if(flag==0){
				pthread_kill(*((pthread_t *)param+cthr),SIGUSR1);	// if thread has not terminated then put the thread to sleep
				status[cthr] = 0;
			}
			else 
				flag = 0;
			
			//printf("\t(Scheduler)End = %d\n",(int)time(NULL));
		}
		
		cthr = (cthr+1)%n;											// switch to next thread
	}

	time(&rawtime);
	printf("\nScheduler thread exiting at time %s\n",ctime(&rawtime));
	pthread_exit(NULL);
}

void *reporter(void *param){
	time_t rawtime;
	int old_status[n],flag1,flag2,flag3,resumed_thread,suspended_thread,terminated_thread;
	for(int i=0;i<n;i++)
		old_status[i] = 0;
		

	flag1 = 0;
	flag2 = 0;
	flag3 = 0;
	while(totp){															// While total waiting threads < n
		for(int i=0;i<n;i++){
			if(status[i]!=old_status[i]){									// If status of any thread has changed
				if(status[i]==-1 && old_status[i]==1){
					terminated_thread = i;
					cout<<"Thread "<<terminated_thread<<" has finished";	// keep record of terminated thread
					flag1 = 1;
				}
				else{
					if(status[i]==0 && old_status[i]==1){
						suspended_thread = i;								// keep record of suspended thread
						flag2 = 1;
					}
					else{
						if(status[i]==1 && old_status[i]==0){
							if((flag2==1 || flag1==1)){
								resumed_thread = i;							// keep record of resumed thread
								flag3 = 1;
							}
						}
					}
				}
				
				old_status[i] = status[i];									// update old status
			}
		}
		

		/*
			Printing the context switch information
		*/
		if(flag2==1 && flag3==1){
			time(&rawtime);
			cout<<"Thread "<<suspended_thread<<" suspended and Thread "<<resumed_thread<<" resumed at time "<<ctime(&rawtime)<<endl;
			flag2 = 0;
			flag3 = 0;	
		}
		
		/*
			Printing thread termination information
		*/
		if(flag1==1 && flag3==1){
			time(&rawtime);
			cout<<" and Thread "<<resumed_thread<<" resumed at time "<<ctime(&rawtime)<<endl;	
			flag1 = 0;
			flag3 = 0;
		}
		
	}

	for(int i=0;i<n;i++){
		if(status[i]==-1 && (old_status[i]==1 || old_status==0)){
			cout<<"Thread "<<i<<" has finished";
			break;
		}
	}

	time(&rawtime);
	cout<<"\nReporter thread exiting at time "<<ctime(&rawtime)<<endl;
	pthread_exit(NULL);
	
}

int main(){
	cout << "Enter some integer n\n";
	cin>>n;																		// take n as input from user
	
	totp = n;
	cthr = 0;
	status = new int[n];
	
	pthread_t worker_threads[n], scheduler_thread, reporter_thread;				// Declare all the threads
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	for(int i=0;i<n;i++){
		pthread_create(&worker_threads[i],&attr,worker,(void *)i);				// create n worker threads
	}
		
	pthread_create(&scheduler_thread,&attr,scheduler,(void *)worker_threads);	// create scheduler thread
	pthread_create(&reporter_thread,&attr,reporter,NULL);						// create reporter thread
	
	pthread_exit(NULL);
}