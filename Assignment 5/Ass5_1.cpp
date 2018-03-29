#include <bits/stdc++.h>
#include <iostream>

using namespace std;

// data structures
int page_table[64];
int *page_frame;
queue <int> myfifo; 

int FIFO(){
	int temp = myfifo.front(); 
	myfifo.pop();
	return temp;
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
        if(page_frame[i]==0)
            page_frame[i] = 1;
            return i;
    }
    return -1;
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

void readFile(const char *filename, int m, int mode){
    FILE *fp;
    char line[50];
    int page_no, page_frame, victim_page, line_no=0, page_probe;

    long long cost=0;
    int page_faults=0,page_transfers=0;

    fp = fopen(filename,"r");

    while(fgets(line, sizeof(line), fp) != NULL){
        line_no++;
        if(line[0]=='#')continue;
        page_no = atoi(line+2);
        switch(line[0]){
            case '0':   if(page_table[page_no] & 4 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 1;
                            printPageTable();
                        }else{
                            // invalid page
                            page_frame = getFrame(m);
                            page_faults += 1;
                            if(page_frame==-1){
                                // memory full

                                switch(mode){
                                	case 0:	victim_page = FIFO(); break;
                                	/*case 1:	victim_page = (); break;
                                	case 2:	victim_page = (); break;*/
                                	case 3:	break;
                                	case 4:	victim_page = secondChance(); break;
                                }
                                
                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 5;
                                
                                myfifo.push(page_no);

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
                            	myfifo.push(page_no);

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
                            page_frame = getFrame(m);
                            if(page_frame==-1){
                                // memory full

                                switch(mode){
                                	case 0:	victim_page = FIFO(); break;
                                	/*case 1:	victim_page = (); break;
                                	case 2:	victim_page = (); break;
                                	case 3:	victim_page = (); break;*/
                                	case 4:	victim_page = secondChance(); break;
                                }

                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 7;
                                
                                myfifo.push(page_no);

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
                                myfifo.push(page_no);

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
	int m;
	cout<<"Enter size of memory\n";
	cin>>m;
    readFile("mytest.txt", m, 0);
    return 0;
}
