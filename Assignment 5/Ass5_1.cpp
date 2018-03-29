#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERRUPT 10

// data structures
int page_table[64];
int *page_frame;

int time_used[64];
int counter;

int getFrame(int m){
    int i;
    for(i=0;i<m;i++){
        if(page_frame[i]==0)
            page_frame[i] = 1;
            return i;
    }
    return -1;
}

int random(){
	do{
		i = rand()%64;
	}while(page_table[i] & 100 == 0);
	return i;
}

int lru(){
	int m=counter,i,page_no=-1;
	for(i=0;i<64;i++){
		
		if((page_table[i] & 100==1) && (time_used[i]<=m)){
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
	if(v0.size()>0)
		return v0[rand()%(v0.size())];
	if(v1.size()>0)
		return v1[rand()%(v1.size())];
	if(v2.size()>0)
		return v2[rand()%(v2.size())];
	if(v3.size()>0)
		return v3[rand()%(v3.size())];
}

void printPageTable(){
    int i;
    printf("____________________________________________\n");
    printf("| Page No | Frame No | Valid | Dirty | Ref |\n");
    printf("--------------------------------------------\n");
    for(i=0;i<64;i++){
        printf("|\t%d\t%d\t%d\t%d\t%d |\n",i,page_table[i]>>3,page_table[i]&4,page_table[i]&2,page_table[i]&1);
        printf("--------------------------------------------\n");
    }

}

void readFile(char *filename){
    FILE *fp;
    char line[50];
    int page_no,page_frame,victim_page,line_no=0;

    long long cost=0;
    int page_faults=0,page_transfers=0;

    fp = fopen(filename,"r");

    while(fgets(line, sizeof(line), fp) != NULL){
        line_no++;
        if(line_no%INTERRUPT==0){
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
            case '0':   if(page_table[page_no] & 4 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 1;
                            printPageTable();
                        }else{
                            // invalid page
                            page_frame = getFrame();
                            page_faults += 1;
                            if(page_frame==-1){
                                // memory full
                                // victim_page = fifo();
                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 5;

                            	printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                                // check dirty bit
                                if(page_table[victim_page] & 2 == 1){
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
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 5;
                                printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                            }
                        }
                        break;

            case '1':   if(page_table[page_no] & 4 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 3;
                            printPageTable();
                        }else{
                            // invalid page
                            page_faults += 1;
                            page_frame = getFrame();
                            if(page_frame==-1){
                                // memory full
                                // victim_page = fifo();
                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 7;

                            	printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                                // check dirty bit
                                if(page_table[victim_page] & 2 == 1){
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
                                
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 5;
                                printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                            }
                        }
        }
    }

}

int main()
{
	srand(time(0));

	counter=0;
	for(i=0;i<64;i++)
		time_used[i]=-1;

    readFile("mytest.txt");
    return 0;
}
