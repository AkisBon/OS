typedef struct pageFaultCounters{
    int reads_counter;
    int writes_counter;
    int proc1_pageFaults;
    int proc2_pageFaults;
    int pageFaults;
} counters;



int search_inv_pageTable(int ,int ,int ,int ,int ***, counters *,int );
