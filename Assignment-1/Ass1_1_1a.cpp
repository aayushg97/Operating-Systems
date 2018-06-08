#include <bits/stdc++.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int mypipe[2];							// Pipe between Process A and Process D
int mypipe1[2];							// Pipe between Process B and Process D
int mypipe2[2];							// Pipe between Process C and Process D


int main(){
	pipe(mypipe);
	pipe(mypipe1);
	pipe(mypipe2);
	
	int x;
	x = fork();
	
	if(x==0){
		x = fork();
		
		if(x==0){
			
			// Process D
			
			sleep(2);
			int arr[300],p,q,r,cp,cq,cr;
			
			read(mypipe[0],&p,sizeof(int));			// Reading first number from process A 
			read(mypipe1[0],&q,sizeof(int));		// Reading second number from process B
			read(mypipe2[0],&r,sizeof(int));		// Reading third number from process C
			
			cp=0; cq=0; cr=0;
			for(int i=0;i<300;i++){																// Merging starts
				if(cp<100 && cq<100 && cr<100){																	
					if(p<=q && p<=r){
						arr[i] = p;
						cp++;
						if(cp<100)read(mypipe[0],&p,sizeof(int));
					}
					else{
						if(q<=p && q<=r){
							arr[i] = q;
							cq++;
							if(cq<100)read(mypipe1[0],&q,sizeof(int));
						}
						else{
							if(r<=q && r<=p){
								arr[i] = r;
								cr++;
								if(cr<100)read(mypipe2[0],&r,sizeof(int));
							}
						}	
					}
				}
				else{
					if(cp<100 && cq<100){
						if(p<=q){
							arr[i] = p;
							cp++;
							if(cp<100)read(mypipe[0],&p,sizeof(int));
						}
						else{
							arr[i] = q;
							cq++;
							if(cq<100)read(mypipe1[0],&q,sizeof(int));			
						}
					}
					else{
						if(cp<100 && cr<100){
							if(p<=r){
								arr[i] = p;
								cp++;
								if(cp<100)read(mypipe[0],&p,sizeof(int));
							}
							else{
								arr[i] = r;
								cr++;
								if(cr<100)read(mypipe2[0],&r,sizeof(int));			
							}
						}
						else{
							if(cq<100 && cr<100){
								if(q<=r){
									arr[i] = q;
									cq++;
									if(cq<100)read(mypipe1[0],&q,sizeof(int));
								}
								else{
									arr[i] = r;
									cr++;
									if(cr<100)read(mypipe2[0],&r,sizeof(int));
								}
							}
							else{
								if(cp<100){
									while(cp<100){
										arr[i] = p;
										cp++;
										if(cp<100)read(mypipe[0],&p,sizeof(int));
										i++;
									}
									break;
								}
								else{
									if(cq<100){
										while(cq<100){
											arr[i] = q;
											cq++;
											if(cq<100)read(mypipe1[0],&q,sizeof(int));
											i++;
										}
										break;
									}
									else{
										if(cr<100){
											while(cr<100){
												arr[i] = r;
												cr++;
												if(cr<100)read(mypipe2[0],&r,sizeof(int));
												i++;
											}
											break;
										}
									}
								}
							}
						}
					}
				}
			}																	// Merging ends
			
			cout<<"Process D \n";
			for(int i=0;i<300;i++){
				cout<<arr[i]<<" "; 
			}
			cout<<endl;
			
			cout<<"Minimum number is "<<arr[0]<<endl;
		}
		else{
			int arr[100];							// Process C 
			
			srand(time(NULL)+1);					// setting random seed value
				
			for(int i=0;i<100;i++){
				arr[i] = rand()%100 + 1;			// generation of 100 random numbers in process C 
			}
			sort(arr,arr+100);						
			
			cout<<"Process C"<<endl;
		
			for(int i=0;i<100;i++){
				cout<<arr[i]<<" "; 
			}
			cout<<endl;
			
			write(mypipe2[1],arr,sizeof(int)*100);
		}
	}
	else{
		x = fork();
		
		if(x==0){
			int arr[100];								// Process A
		
			srand(time(NULL));							// setting random seed value
		
			for(int i=0;i<100;i++){
				arr[i] = rand()%100 + 1;				// generation of 100 random numbers in process A
			}
			sort(arr,arr+100);
		
			cout<<"Process A"<<endl;
			for(int i=0;i<100;i++){
				cout<<arr[i]<<" "; 
			}
			cout<<endl;
		
			write(mypipe[1],arr,sizeof(int)*100);
		}
		else{
			int arr[100];								// Process B
		
			srand(time(NULL)+2);						// setting random seed value
		
			for(int i=0;i<100;i++){
				arr[i] = rand()%100 + 1;				// generation of 100 random numbers in process B
			}
			sort(arr,arr+100);
		
			cout<<"Process B"<<endl;
			for(int i=0;i<100;i++){
				cout<<arr[i]<<" "; 
			}
			cout<<endl;
		
			write(mypipe1[1],arr,sizeof(int)*100);
		}
	}
	
	return 0;
}
