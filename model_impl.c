#ifndef QUEUE_H
#define QUEUE_H
#endif

#include <stdio.h>	// for printf()
#include <stdlib.h>	// for random()
#include "queue.h"


#define bufferSize 40

extern int NR_VIRT_PAGES;
extern int NR_PHYS_PAGES;
extern int NR_REFERENCES;


void printStats(int frames,int trace_count, counters counters_struct){
      printf("total frames: %d\n",frames);
      printf("total pageFaults: %d\n", counters_struct.pageFaults);
      printf("total reads: %d\n",counters_struct.reads_counter);
      printf("total writes: %d\n", counters_struct.writes_counter);
      printf("total trace examined from files: %d\n",trace_count);
}

int check_forPageNum(int ***inv_pageTable,int frames,int pageNum,char* type_rw, int proc_id){

  //find trace
  int trace_flag = 0; //init to 0
  for (int i = 0; i < frames; i++){

     // value not null
     if (inv_pageTable[i][5] != 0)
     {
          if ((inv_pageTable[i][0] == proc_id) && (inv_pageTable[i][1] == pageNum)) {

                  trace_flag = 1;

                  //set type_bit
                  if (strcmp(type_rw, "W") == 0){
                      inv_pageTable[i][4] = 1; //w
                  }
                  else{
                      inv_pageTable[i][4] = 0; //r
                  }
                  inv_pageTable[i][2] = 1; //set valid_bit

              }
              //otherwise trace_flag remains 0
           }

    }
    return trace_flag;

}

int search_inv_pageTable(int frames,int pid,int pageNum, int modified, int ***inv_pageTable, counters* pageFaults,int add){

    for (int i = 0; i < frames; i++){

        if (inv_pageTable[i][5] != 0 && inv_pageTable[i][0] == pid
                 && inv_pageTable[i][1] == pageNum){

                      if (add == 1){
                          inv_pageTable[i][2] = 1; // valid
                          inv_pageTable[i][3] = modified; //set modified,it was init to 0
                      }
                      else{ // it is going to be deleted from memory queue
                          inv_pageTable[i][2] = 0; //invalid
                          if (inv_pageTable[i][3] == 1){ // if modified (traced with w) increment writes
                              inv_pageTable[i][3] = 0;
                              pageFaults->writes_counter++;
                          }
                      }
        }
        if (inv_pageTable[i][5] == 0){ //linear table,first empty entry

                      if (add == 1){
                           inv_pageTable[i][2] = 1; // valid
                           inv_pageTable[i][1] = pageNum; // valid

                      }

        }

    }

}

void LRU_replacement(Queue *queue, Hash* hash, int pageNum, int pid,
                      int frames, int modified, counters* pageFaults, int*** inv_pageTable){

      ReferencePage(queue,hash,pageNum,pid,modified,frames,pageFaults,inv_pageTable);

}


void WS_replacement(Queue *queue, Hash* hash, int pageNum, int pid,
                      int frames, int modified, counters* pageFaults, int*** inv_pageTable,int ***ws_array,int k){

        ReferencePage2(queue,hash,pageNum,pid,modified,frames,pageFaults,inv_pageTable,ws_array,k);

}

int virtualMemory(int frames, int k, int q, int max, char* repl_arg){

    int pageNum, pid, type_rw, max_flag;
    int pageFaults = 0, in_memory = 0,
        reads_counter = 0, writes_counter = 0, trace_count = 0;

    int ws_array[k][2]; // hold pageNum and time
    for (int i = 0; i < k; i++){
        ws_array[i][0] = -1;
        ws_array[i][1] = -1;
    }

    int **inv_pageTable;

    char buff[bufferSize];
    char *file_val; //store arg1 or arg2 from file
    char pageNum_str[5];


    //create page: pid|page_name|valid|modified|r_or_w(0 or 1)
    if ((inv_pageTable = malloc(frames * sizeof(int*))) == NULL){
        printf("malloc failed1\n");
        return -1;
    }

    //initialize inverted page table
    for(int i =0; i<frames; i++){
      //5 ints
        if ((inv_pageTable[i] = malloc(6*sizeof(int))) == NULL){
            printf("malloc failed2\n");
            return -1;
        }

        inv_pageTable[i][5] = 0; //5th-bit indicates null value(not in memory)
        inv_pageTable[i][2] = 0;  //use valid_bit to know if page is in memory,LRU
        inv_pageTable[i][3] = 0; //use modified_bit to know if you have to write it in backing store,init to 0
    }


    //init struct
    counters pageFaults_struct;
    pageFaults_struct.reads_counter = 0;
    pageFaults_struct.writes_counter = 0;
    pageFaults_struct.proc1_pageFaults = 0;
    pageFaults_struct.proc2_pageFaults = 0;
    pageFaults_struct.pageFaults = 0;

    FILE* file_1 = fopen("bzip.trace", "r");
    if (file_1 == NULL){
        printf("failed to open bzip.trace\n");
        return -1;
      }

      FILE* file_2 = fopen("gcc.trace", "r");
      if (file_2== NULL){
        printf("failed to open gcc.trace\n");
        return -1;
      }

    Queue *queue = createQueue( frames );
    //in order to use hash array for LRU's implementation ,the array must hold all addresses uo to 0xFFFF
    //or i could make an adjacency list and hash page nums to avoid overflow because of large array
    Hash* hash;
    if (strcmp(repl_arg, "LRU") == 0) { hash = createHash( 65535 ); }

    //reading q from each process,interchanging
    for (int i = 0; i < q; i++){
      FILE *file; file = file_1; //init values for 1st process

      int j;
      for (j = 0; j < 2; j++){
        //for 1st process
        pid = j + 1;

        fgets(buff, bufferSize, file);

        //read address
        file_val = strtok(buff, " ");
        strncpy(pageNum_str, file_val, 5);
        pageNum_str[4] = '\0';
        pageNum = (int)strtol(pageNum_str, NULL, 16);


        //read type
        file_val = strtok(NULL, " ");
        if (file_val == 'R'){  type_rw = 0;  }
        else{  type_rw = 1;  }

        //search ws_array to update time or insert page with current_time
        int time = 0; int min;
        for (int t=0; t<k; t++){
            if (t == 0){  min = i; }
            else if(ws_array[i][0] < min){ min = i; }

            if (ws_array[i][0] == pageNum){
                ws_array[i][1] = time++;
                break;
            }
            else if(ws_array[i][0] == -1){ // not in array,add it there
                ws_array[i][0] = pageNum;
                ws_array[i][1] = time++;
                break;
            }

            if (t == k-1){//change min time of k
                  ws_array[i][k-1] = time++;
            }
        }

        if (check_forPageNum(inv_pageTable,
                    frames, pageNum, type_rw, pid) == 0){

                        //setting counters
                        pageFaults_struct.reads_counter++; //we read from file
                        //if type_bit is w,we will modify to 1 in
                        pageFaults_struct.proc1_pageFaults ++;
                        pageFaults_struct.pageFaults ++;


                        //LRU:swap in,out means reads++ writes++
                        if (strcmp(repl_arg, "LRU") == 0) { LRU_replacement(queue,hash,pageNum,pid,frames,type_rw,&pageFaults_struct,inv_pageTable); }
                        //if (strcmp(repl_arg, "WS") == 0) { WS_replacement(queue,hash,pageNum,pid,frames,type_rw,&pageFaults_struct,inv_pageTable,ws_array,k); }

        }

        //in this file we checked a trace
        trace_count ++;
        max_flag = 0;
        if (trace_count == max){
           max_flag = 1;
           //break;
        }

        //values for 2st process
        file = file_2;
      }

      if (max_flag == 1){
          //break;
      }


    }

    printStats(frames,trace_count,pageFaults_struct);


    //close files
    fclose(file_1);
    fclose(file_2);

    //free invert table
    for (int i = 0; i < frames; i++){
        //free(inv_pageTable[i]);
    }
    free(inv_pageTable);



}
