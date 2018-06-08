#include <bits/stdc++.h>
#include <iostream>
#define INTERRUPT 10
#define READ_PROB 0.7

using namespace std;

// data structures
int *page_table;				// Table that keeps record of page and frame mapping as well as valid, dirty and reference bits 
int *page_frame_table;			// Stores status of frames i.e. used(1) or free(0)

int *time_used;					// Keeps track of recently used pages (useful in LRU page replacement)
int counter;					//

queue <int> myfifo; 			// Queue used to store pages in case of FIFO page-replacement

/*
	Returns victim page in case of FIFO replacement
*/
int fifo(){
    int temp = myfifo.front(); 
    myfifo.pop();
    return temp;
}

/*
	Returns victim page in case of Random replacement
*/
int randum(int no_page){
	int i;
	do{
		i = rand()%no_page;
	}while((page_table[i] & 100)>>2 == 0);
	return i;
}

/*
	Returns victim page in case of LRU replacement
*/
int lru(int no_page){
	int m=counter,i,page_no=-1;
	for(i=0;i<no_page;i++){
		// find page with least value in time_used (least recently used page)
		if(((page_table[i] & 100)>>2 == 1) && (time_used[i]<=m)){
			m=time_used[i];
			page_no = i;
		}
	}
	return page_no;
}

/*
	Returns victim page in case of NRU replacement
*/
int nru(int no_page){
	/*	
		sets of pages arranged in increasing order of priority
			v0 :- not modified, not referenced set of pages
			v1 :- modified, not referenced set of pages
			v2 :- not modified, referenced set of pages
			v3 :- modified, referenced set of pages
	*/
	vector<int> v0,v1,v2,v3;
	int i;
	for(i=0;i<no_page;i++){
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

/*
	Returns victim page in case of Second Chance replacement
*/
int secondChance(){
    int page_probe;

    while(1){
        page_probe = myfifo.front(); 
        if((page_table[page_probe] & 1) == 0){
        	// return page as victim if ref bit = 0
            myfifo.pop();
            return page_probe;
        }
        else{
        	// give this page a second chance if ref bit = 1
            page_table[page_probe] ^= 1;
            myfifo.pop();
            myfifo.push(page_probe);
        }
    }
}

/*
	returns a free frame by scanning through page_frame_table 
*/
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

/*
	Prints page table
*/
void printPageTable(int no_page, FILE *fp1, char mode){
    int i;

    fprintf(fp1, "____________________________________________\n");
    fprintf(fp1, "| Page No | Frame No | Valid | Dirty | Ref |\n");
    fprintf(fp1, "--------------------------------------------\n");
    for(i=0;i<no_page;i++){
    	fprintf(fp1, "|\t%d\t%d\t%d\t%d\t%d |\n",i,page_table[i]>>3,(page_table[i]&4)>>2,(page_table[i]&2)>>1,page_table[i]&1);
        fprintf(fp1, "--------------------------------------------\n");
    }

    if(mode=='t'){
    	printf("____________________________________________\n");
	    printf("| Page No | Frame No | Valid | Dirty | Ref |\n");
	    printf("--------------------------------------------\n");
	    for(i=0;i<no_page;i++){
	    	printf("|\t%d\t%d\t%d\t%d\t%d |\n",i,page_table[i]>>3,(page_table[i]&4)>>2,(page_table[i]&2)>>1,page_table[i]&1);
	        printf("--------------------------------------------\n");
	    }
    }
}

void readFile(const char *filename, int no_page, int m, int mode, const char *outfile, char mode1){
    FILE *fp, *fp1;
    char line[50];
    int page_no,page_frame,victim_page,line_no=0,i;

    long long cost=0;
    int page_faults=0,page_transfers=0;

    fp = fopen(filename,"r");
    fp1 = fopen(outfile, "w");

    while(fgets(line, sizeof(line), fp) != NULL){
        line_no++;        
        if(page_faults%INTERRUPT==0 and mode==3){
        	// periodically set ref bits of all pages to zero
        	for(i=0;i<no_page;i++){
        		page_table[i]>>1;
        	    page_table[i]<<1;
        	}
        }
        if(line[0]=='#')continue;
        
        page_no = atoi(line+2);
        time_used[page_no] = counter;
        counter++;
        switch(line[0]){
            case '0':   // read operation
            			if((page_table[page_no] & 4)>>2 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 1;
                            if(mode1=='t')
                            	printPageTable(no_page, fp1, mode1);
                        }else{
                            // invalid page
                            page_frame = getFrame(m);		// get free frame
                            
                            page_faults += 1;
                            if(page_frame==-1){
                                // memory full, replace a page using page replacement
                                switch(mode){
                                    case 0: victim_page = fifo(); break;
                                    case 1: victim_page = randum(no_page); break;
                                    case 2: victim_page = lru(no_page); break;
                                    case 3: victim_page = nru(no_page); break;
                                    case 4: victim_page = secondChance(); break;
                                }

                                // update page_table and page_frame_table
                            	page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 5;

                                myfifo.push(page_no);

                            	fprintf(fp1, "Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);
                            	if(mode1=='t')
                            		printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);
                                // check dirty bit
                                if((page_table[victim_page] & 2)>>1 == 1){
                                    page_transfers += 2;
                                    cost += 6501;
                                    fprintf(fp1, "  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);
                                    if(mode1=='t')
                                    	printf("  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);
                                }
                                else{
                                	page_transfers += 1;
                                	cost += 3501;
                                }

                                fprintf(fp1, "  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                fprintf(fp1, "  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                                if(mode1=='t'){
                                	printf("  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                	printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                                }


                            }else{
                                // frame available
                                
                                myfifo.push(page_no);
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 5;
                                fprintf(fp1, "Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                fprintf(fp1, "  %d: MAP     %d  %d\n",line_no,page_no,page_frame);

                                if(mode1=='t'){
                                	printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                	printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                                }
                            }
                        }
                        break;

            case '1':   // Write operation
            			if((page_table[page_no] & 4)>>2 == 1){
                            // valid page
                            cost += 1;
                            page_table[page_no] |= 3;
                            if(mode1=='t')
                            	printPageTable(no_page, fp1, mode1);
                        }else{
                            // invalid page
                            page_faults += 1;
                            page_frame = getFrame(m);	// get free frame
                            if(page_frame==-1){
                                // memory full, replace a page using page replacement
                                switch(mode){
                                    case 0: victim_page = fifo(); break;
                                    case 1: victim_page = randum(no_page); break;
                                    case 2: victim_page = lru(no_page); break;
                                    case 3: victim_page = nru(no_page); break;
                                    case 4: victim_page = secondChance(); break;
                                }

                                // update page_table and page_frame_table
                                page_frame = page_table[victim_page]>>3;
                            	page_table[page_no] = page_frame<<3;
                            	page_table[victim_page] ^= 4;
                                page_table[page_no] |= 7;

                                myfifo.push(page_no);

                            	fprintf(fp1, "Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                            	if(mode1=='t')
									printf("Output generated:\n  %d: UNMAP     %d  %d\n",line_no,victim_page,page_frame);

                                // check dirty bit
                                if((page_table[victim_page] & 2)>>1 == 1){
                                    page_transfers += 2;
                                    cost += 6501;
                                    fprintf(fp1, "  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);

                                    if(mode1=='t')
                                    	printf("  %d: OUT     %d  %d\n",line_no,victim_page,page_frame);
                                }
                                else{
                                	page_transfers += 1;
                                	cost += 3501;
                                }

                                fprintf(fp1, "  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                fprintf(fp1, "  %d: MAP     %d  %d\n",line_no,page_no,page_frame);

                                if(mode1=='t'){
                                	printf("  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                	printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                                }

                            }else{
                                // frame available
                                myfifo.push(page_no);
                                page_transfers += 1;
                                cost += 3251;

                                page_table[page_no] = page_frame<<3 | 7;
                                fprintf(fp1, "Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                fprintf(fp1, "  %d: MAP     %d  %d\n",line_no,page_no,page_frame);

                                if(mode1=='t'){
                                	printf("Output generated:\n  %d: IN     %d  %d\n",line_no,page_no,page_frame);
                                	printf("  %d: MAP     %d  %d\n",line_no,page_no,page_frame);
                                }
                            }
                        }
                        
        }

        if(mode1=='t')
        	getchar();
       // while(getchar()!='\n');
    }

    // print stats
    printPageTable(no_page, fp1, mode1);
    fprintf(fp1, "Number of page faults = %d\n", page_faults);
    fprintf(fp1, "Number of page transfers = %d\n", page_transfers);
    fprintf(fp1, "Cost of memory operations = %lld\n", cost);

    if(mode1=='t'){
    	printf("Number of page faults = %d\n", page_faults);
	    printf("Number of page transfers = %d\n", page_transfers);
	    printf("Overall execution time in cycles = %lld\n", cost);
    }
    fclose(fp);
    fclose(fp1);
}

int getRW(float prob){
	float temp = ((float)(rand()%11))/10.0;
	if(temp > prob)
		return 0;
	else
		return 1;
}

void printSet(set <int> w_set, int a){
	set<int>:: iterator itr;
	itr = w_set.begin();
	int t = w_set.size();
	printf("%d inserted set = ", a);
	while(t--){
		printf("%d ", *itr);
		itr++;
	}
	printf("\n");
}

/*Function to generate page references*/
void generatePageTrace(const char *filename, int no_page, int no_ref, int set_size, float prob){
	FILE *fptr;
	fptr = fopen(filename,"w");
	int timer[no_page];
	set<int> w_set;					// working set
	set<int>:: iterator itr;
	set<int> diff;					// complement of all pages
	set<int> superset;				// set of all pages

	// constructing superset
	for(int j=0;j<no_page;j++){
		superset.insert(j);
	}
	
	int num, i;
	
	for(i=0;i<no_page;i++)
		timer[i] = -1;
	
	// Populating the working set
	for(i=0;i<set_size;i++){
		num = rand()%no_page;
		w_set.insert(num);
		fprintf(fptr, "%d %d\n", getRW(READ_PROB), num);
		timer[num] = i;
	}
	
	while(i<no_ref){
		if(getRW(prob)){
			// choose from working set
			itr = w_set.begin();
			num = rand()%w_set.size();
			while(num--)
				itr++;
			fprintf(fptr, "%d %d\n", getRW(READ_PROB), *itr);
			timer[*itr] = i;
		}
		else{
			// choose from complement of working set
			set_difference(superset.begin(), superset.end(), w_set.begin(), w_set.end(), inserter(diff, diff.end()));
			itr = diff.begin();
			num = rand()%diff.size();
			while(num--)
				itr++;
			fprintf(fptr, "%d %d\n", getRW(READ_PROB), *itr);
			timer[*itr] = i;
		}
		
		w_set.insert(*itr);
		
		for(int j=0;j<no_page;j++){
			if(timer[j]!=-1 && timer[j]<=i-set_size){
				w_set.erase(j);
				timer[j] = -1;
			}
		}
		
		i++;
	}
	
	fclose(fptr);
}

int main()
{
	int i, j, no_page, no_ref, set_size, m, algo;
	char mode; 
	char str[50];
	float prob;
	srand(time(0));

    cout<<"Enter number of pages\n";
    cin>>no_page;

    cout<<"Enter number of page references (number of instructions)\n";
    cin>>no_ref;

    cout<<"Enter size of working set\n";
    cin>>set_size;

    cout<<"Enter probability that the next memory reference is from current working directory\n";
    cin>>prob;

    generatePageTrace("input.txt", no_page, no_ref, set_size, prob);

    cout<<"Enter size of memory (number of page frames)\n";
    cin>>m;

    cout<<"Specify mode of execution (type 'n' for normal or 't' for trace)\n";
    cin>>mode;

    if(mode=='t'){
    	// trace mode
    	cout<<"Enter the page replacement algo to be used:-\n";
    	cout<<"0 for FIFO\n";
    	cout<<"1 for Random\n";
    	cout<<"2 for LRU\n";
    	cout<<"3 for NRU\n";
    	cout<<"4 for Second Chance\n";
    	cin>>algo;
    	getchar();

    	page_table = (int *)malloc(no_page*sizeof(int));
	    time_used = (int *)malloc(no_page*sizeof(int));

		counter=0;
		for(i=0;i<no_page;i++){
				time_used[i]=-1;
				page_table[i]=0;
		}
	    
	    page_frame_table = (int*)malloc(m*sizeof(int));
	    for (i = 0; i < m; ++i)
	    	page_frame_table[i]=0;
		
		switch(algo){
			case 0: strcpy(str, "fifo.txt"); break;
			case 1: strcpy(str, "random.txt"); break;
			case 2: strcpy(str, "lru.txt"); break;
			case 3: strcpy(str, "nru.txt"); break;
			case 4: strcpy(str, "second_chance.txt"); break;
		}
		
	    readFile("input.txt",no_page, m, algo, str, mode);
    }
    else{
    	// normal mode
    	cout<<"Input file is stored as 'input.txt'\n";
	    cout<<"Output of FIFO algorithm is stored in 'fifo.txt'\n";
	    cout<<"Output of RANDOM algorithm is stored in 'random.txt'\n";
	    cout<<"Output of LRU algorithm is stored in 'lru.txt'\n";
	    cout<<"Output of NRU algorithm is stored in 'nru.txt'\n";
	    cout<<"Output of SECOND CHANCE algorithm is stored in 'second_chance.txt'\n";


	    for(j=0;j<5;j++){
		    page_table = (int *)malloc(no_page*sizeof(int));
		    time_used = (int *)malloc(no_page*sizeof(int));

			counter=0;
			for(i=0;i<no_page;i++){
					time_used[i]=-1;
					page_table[i]=0;
			}
		    
		    page_frame_table = (int*)malloc(m*sizeof(int));
		    for (i = 0; i < m; ++i)
		    	page_frame_table[i]=0;
			
			switch(j){
				case 0: strcpy(str, "fifo.txt"); break;
				case 1: strcpy(str, "random.txt"); break;
				case 2: strcpy(str, "lru.txt"); break;
				case 3: strcpy(str, "nru.txt"); break;
				case 4: strcpy(str, "second_chance.txt"); break;
			}
			
		    readFile("input.txt",no_page, m, j, str, mode);
	    }
    }
    return 0;
}
