#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
using namespace std;

// structure to store information about a process
struct process{
	int pid;
	int arrival_time;
	int cpu_burst;
	int termination_time;
}; 

// function to compute average turnaround time
double ATN(vector<process> &finished_jobs){
  int total_TA = 0;
  
  for(int i=0;i<finished_jobs.size();i++)
    total_TA += (finished_jobs[i].termination_time - finished_jobs[i].arrival_time);

  return ((double)total_TA)/finished_jobs.size();  
}

// first come first serve scheduling
double fcfs(vector <process> &arr){
	int n = arr.size();
	cout << "\n ------FCFS----------------\n\n";
	
	arr[0].termination_time = arr[0].arrival_time + arr[0].cpu_burst;	
	
	for(int i=1;i<n;i++){
		arr[i].termination_time = (arr[i].arrival_time>=arr[i-1].termination_time) ? arr[i].arrival_time + arr[i].cpu_burst : arr[i-1].termination_time + arr[i].cpu_burst;		// process runs till completion, then next process starts in fifo manner 
	}
	
	cout << "Arrival\tBurst\tTermination\n";
	for(int i=0;i<n;i++){
		cout<< arr[i].arrival_time << "\t" << arr[i].cpu_burst << "\t" << arr[i].termination_time<<endl; 
	}
		
	cout<<"\nAverage turnaround time = "<<ATN(arr)<<endl;
	return ATN(arr);
}

// Pre-emptive SJF
class myComparator
{
  public:
      int operator() (const process& p1, const process& p2){
          return p1.cpu_burst > p2.cpu_burst;
      }
};

double sjf(const vector<process> &job){
  // assume job is sorted by arrival_time
  priority_queue <process, vector<process>, myComparator > pq;  // Creates a Min heap of jobs (order by cpu_burst)
  vector<process> finished_jobs;								// Stores finished jobs
  int next,i,current_time;										// next is the index of the next job
  process current_job;

  cout<<"\n\n+++++++ PRE-EMPTIVE SHORTEST-JOB-FIRST SCHEDULING +++++++"<<endl;	  	
  cout<<"---------------------------------------------------------"<<endl;
  pq.push(job[0]);									//
  current_time = job[0].arrival_time;				// Putting initial value in the queue
  next = 1;											//
  
  
  /*
  	Push all the jobs in the queue that have already arrived
  */
  while(next<job.size() && job[next].arrival_time==current_time){
  	pq.push(job[next]);						
  	next++;
  }

  while(!pq.empty()){							// while queue is not empty
    current_job = pq.top();						// extract the job with minimum remaining burst
    pq.pop();

    cout<<"Process: "<<current_job.pid<<"	|	Start Time: "<<current_time;	

    if(next<job.size()){						// if more jobs are waiting for the CPU and can be added to the queue then
      if((current_time+current_job.cpu_burst)<job[next].arrival_time){			// if current process can finish before arrival of next one
        current_job.termination_time = current_time + current_job.cpu_burst;	// set termination time of current process
        finished_jobs.push_back(current_job);									// add current process to the list of finished jobs
        current_time = current_job.termination_time;							// update current time
        cout<<"	|	End Time: "<<current_time<<endl;
        if(pq.empty()){															// if queue is empty add next job that requires CPU	
          current_time = job[next].arrival_time;								
          while(next<job.size() && job[next].arrival_time==current_time){		// add all those jobs whose arrival time = current_time
  		pq.push(job[next]);
  		next++;
  	  }
        }
      }
      else{																	// if current process can't finish before arrival of next one
        current_job.cpu_burst -= (job[next].arrival_time-current_time);		// update burst of current process
        current_time = job[next].arrival_time;								// update current time
        if(current_job.cpu_burst==0){										// if current job has finished, set its termination time 
          current_job.termination_time = current_time;
          finished_jobs.push_back(current_job);
        }
        else																// else push it back in the queue
          pq.push(current_job);
        while(next<job.size() && job[next].arrival_time==current_time){		// if next job has arrived then push it in the queue
  		pq.push(job[next]);
  		next++;
  	}
      	cout<<"	|	End Time: "<<current_time<<endl;
      }
    }
    else{
      current_job.termination_time = current_time + current_job.cpu_burst;		// if no more jobs can be added to the queue
      finished_jobs.push_back(current_job);										// then set termination time of remaining processes
      current_time = current_job.termination_time;
      cout<<"	|	End Time: "<<current_time<<endl;
    }
  }

  // Compute average turnaround time
  cout<<"\nAverage turnaround time = "<<ATN(finished_jobs)<<endl;
  cout<<"---------------------------------------------------------"<<endl;
  
  return ATN(finished_jobs);
}

// round robin scheduling
double RR(vector <process> &arr,int delta){		// delta is time quantum
	int n = arr.size(),i,j,totp,loq;
	int temp[n],time;
	
	for(int i=0;i<n;i++){				// storing cpu bursts in a temporary array
		temp[i] = arr[i].cpu_burst;
	}
	
	cout<<"\n-----Round Robin with delta = "<<delta<<"\n"<<endl;
	i=0,j=1;
	totp=n;				// total no of processes
	loq=1;				// length of queue (no of processes currently involved in round robin)	
	time = 0;
	while(totp){					// run while there are processes left
		if(temp[i]>0){				// if current process has computation left (remaining cpu burst>0)
			if(temp[i]<=delta){		// current process terminates before time slice expires
				cout << "Process: "<<arr[i].pid<<"\tstart: "<<time;
				time += temp[i];	// update time	
				totp--;			
				loq--;
				temp[i] = 0;
				arr[i].termination_time = time;		
				cout << "\tend: "<<time<<endl;
			}
			else{				// time slice expires
				cout << "Process: "<<arr[i].pid<<"\tstart: "<<time;
				temp[i] -= delta;
				time += delta;		// update time
				cout << "\tend: "<<time<<endl;
			}
		}
		else{					
			if(loq==0)			// if no process in queue, wait till next process arrives
				time = arr[j].arrival_time;
		}
		
		if(i==j-1 && j<n){			// insert new jobs in order
			if(time>=arr[j].arrival_time){	// if next job arrived, insert it to queue
				j++;
				loq++;
			}
		}
		i = (i+1)%j;				// give cpu control to next job in round 
	}
	
	cout<<"Average turnaround time is "<<ATN(arr)<<endl;
	return ATN(arr);
}

int main(){
	int n,l;
	double avg_fcfs=0.0,avg_sjf=0.0,avg_rr1=0.0,avg_rr2=0.0,avg_rr5=0.0;
	
	// input number of processes and mean L for interprocess time interval
	cin >> n >> l;
	vector <process> arr(n);
	ofstream fp;
	
	for(int j=0;j<10;j++){
		// first process
		arr[0].pid = 1;
		arr[0].arrival_time = 0;
		arr[0].cpu_burst = (rand()%20)+1;
	
		// store process details in file
		fp.open("Table.txt");
		fp << "Process No\tArrival Time\tCPU Burst\n";
		fp << 1 << "\t" << arr[0].arrival_time << "\t" << arr[0].cpu_burst << endl;
	
		// generate arrival time and cpu burst 
		for(int i=1;i<n;i++){
			srand(time(NULL)+i+100*j);
			arr[i].pid = i+1;
			arr[i].arrival_time = (int)((((-1.0)/(double)l)*log((rand()/1000000000.0)/RAND_MAX))*1000)%11;		// expoential distribution
			arr[i].arrival_time = arr[i].arrival_time + arr[i-1].arrival_time;
			arr[i].cpu_burst = (rand()%20)+1;									// uniform distribution
		
			fp << i+1 << "\t" << arr[i].arrival_time << "\t" << arr[i].cpu_burst << endl;
		}
	
		fp.close();
	
		// compute average TA time for 10 runs
	
		avg_fcfs += fcfs(arr);
		avg_sjf += sjf(arr);
		avg_rr1 += RR(arr,1);
		avg_rr2 += RR(arr,2);
		avg_rr5 += RR(arr,5);
	}
	
	// write to file
	fp.open("Table3.txt");
	fp << "FCFS\tSJF\tRR1\tRR2\tRR5\n";
	fp << avg_fcfs/10 << "\t" << avg_sjf/10 << "\t" << avg_rr1/10 << "\t" << avg_rr2/10 << "\t" << avg_rr5/10 << endl;
	
	fp.close();
	return 0;
}
