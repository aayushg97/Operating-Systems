#include <bits/stdc++.h>
#include <iostream>
#define INTERRUPT 10

using namespace std;

// data structures
int page_table[64];
int *page_frame_table;

int time_used[64];
int counter;

queue <int> myfifo; 

int fifo(){
    int temp = myfifo.front(); 
    myfifo.pop();
    return temp;
}

int randum(){
	int i;
	do{
		i = rand()%64;
	}while((page_table[i] & 100)>>2 == 0);
	return i;
}

int lru(){
	int m=counter,i,page_no=-1;
	for(i=0;i<64;i++){
		
		if(((page_table[i] & 100)>>2 == 1) && (time_used[i]<=m)){
			m=time_used[i];
			page_no = i;
		}
	}
	return page_no;
}

int nru(){
	vector<int> v0,v1,v2,v3;
	int i;
	for(i=0;i<64;i++){
		if((page_table[i]&4)>>2 == 1){
			switch((page_table[i] & 3)){
				case 0:	v0.push_back(i);
						break;
				case 1: v2.push_back(i);
						break;
				case 2: v1.push_back(i);
						break;
				case 3: v3.push_back(i);
						break;						
			}
		}
	}
	if(v0.size()>0)
		return v0[rand()%(v0.size())];
	if(v1.size()>0)
		return v1[rand()%(v1.size())];
	if(v2.size()>0)
		return v2[rand()%(v2.size())];
	if(v3.size()>0)
		return v3[rand()%(v3.size())];
}

int secondChance(){
    int page_probe;

    while(1){
        page_probe = myfifo.front(); 
        if((page_table[page_probe] & 1) == 0){
            myfifo.pop();
            return page_probe;
        }
        else{
            page_table[page_probe] ^= 1;
            myfifo.pop();
            myfifo.push(page_probe);
        }
    }
}

int getFrame(int m){
    int i;
    for(i=0;i<m;i++){
        if(page_frame_table[i]==0){
            page_frame_table[i] = 1;
            return i;
        }
    }
    return -1;
}

void printPageTable(){
    int i;
    printf("____________________________________________\n");
    printf("| Page No | Frame No | Valid | Dirty | Ref |\n");
    printf("--------------------------------------------\n");
    for(i=0;i<64;i++){
    	printf("|\t%d\t%d\t%d\t%d\t%d |\n",i,page_table[i]>>3,(page_table[i]&4)>>2,(page_table[i]&2)>>1,page_table[i]&1);
        printf("--------------------------------------------\n");
    }

}

void readFile(const char *filename, int m, int mode){
    FILE *fp;
    char line[50];
    int page_no,page_frame,victim_page,line_no=0,i;

    long long cost=0;
    int page_faults=0,page_transfers=0;

    fp = fopen(filename,"r");

    while(fgets(line, sizeof(line), fp) != NULL){
        line_no++;        
        if(line_no%INTERRUPT==0 and mode==3){
        	for(i=0;i<64;i++){
        		page_table[i]>>1;
        	    page_table[i]<<1;
        	}
        }
        if(line[0]=='#')continue;
        
        page_no = atoi(line+2);
        time_used[page_no] = counter;
        counter++;
        switch(line[0]){
            case '0':   if((page_table[page_no] & 4)>>2 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 1;
                            printPageTable();
                        }else{
                            // invalid page
                            page_frame = getFrame(m);
                            //cout<<"frame returned: "<<page_frame;
                            page_faults += 1;
                            if(page_frame==-1){
                                // memory full
                                switch(mode){
                                    case 0: victim_page = fifo(); break;
                                    case 1: victim_page = randum(); break;
                                    case 2: victim_page = lru(); break;
                                    case 3: victim_page = nru(); break;
                                    case 4: victim_page = secondChance(); break;
                                }
                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 5;

                                myfifo.push(page_no);

                            	printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                                // check dirty bit
                                if((page_table[victim_page] & 2)>>1 == 1){
                                    page_transfers += 2;
                                    cost += 6501;
                                    printf("  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);
                                }
                                else{
                                	page_transfers += 1;
                                	cost += 3501;
                                }

                                printf("  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);


                            }else{
                                // frame available
                                //cout<<"Frame available\n";
                                myfifo.push(page_no);
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 5;
                                printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                            }
                        }
                        break;

            case '1':   //cout<<""<<page_table[page_no];
            			if((page_table[page_no] & 4)>>2 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 3;
                            printPageTable();
                        }else{
                            // invalid page
                            page_faults += 1;
                            page_frame = getFrame(m);
                            if(page_frame==-1){
                                // memory full
                                switch(mode){
                                    case 0: victim_page = fifo(); break;
                                    case 1: victim_page = randum(); break;
                                    case 2: victim_page = lru(); break;
                                    case 3: victim_page = nru(); break;
                                    case 4: victim_page = secondChance(); break;
                                }
                                page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 7;

                                myfifo.push(page_no);

                            	printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                                // check dirty bit
                                if((page_table[victim_page] & 2)>>1 == 1){
                                    page_transfers += 2;
                                    cost += 6501;
                                    printf("  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);
                                }
                                else{
                                	page_transfers += 1;
                                	cost += 3501;
                                }

                                printf("  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);


                            }else{
                                // frame available
                                myfifo.push(page_no);
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 7;
                                printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                            }
                        }
        }
    }

}

int main()
{
	int i;
	srand(time(0));

	counter=0;
	for(i=0;i<64;i++){
			time_used[i]=-1;
			page_table[i]=0;
	}
    int m;
    cout<<"Enter size of memory\n";
    cin>>m;
    page_frame_table = (int*)malloc(m*sizeof(int));
    for (i = 0; i < m; ++i)
    	page_frame_table[i]=0;

    readFile("test.txt",m,3);
    return 0;
}
