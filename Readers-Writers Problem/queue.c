 #ifndef QUEUE_H
#define QUEUE_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"


QNode* newQNode( int pageNumber, int pid)
{
    QNode* temp = (QNode *)malloc( sizeof( QNode ) );
    temp->pageNumber = pageNumber;
    temp->pid = pid;
    temp->prev = temp->next = NULL;
    return temp;
}

Queue* createQueue( int numberOfFrames )
{
    Queue* queue = (Queue *)malloc( sizeof( Queue ) );
    queue->count = 0;
    queue->front = queue->rear = NULL;
    queue->numberOfFrames = numberOfFrames;

    return queue;
}


Hash* createHash( int capacity )
{
    Hash *hash = (Hash *) malloc( sizeof( Hash ) );
    if ( hash == NULL ){
        printf("%d\n",sizeof(Hash));
        printf("malloc for hash part1 failed\n");
    }
    hash->capacity = capacity;
    if ( ( hash->array = (QNode **) malloc( hash->capacity * sizeof( QNode* ))) == NULL ){
        printf("malloc for hash part2 failed\n");
    }
    int i;
    for( i = 0; i < hash->capacity; ++i )
        hash->array[i] = NULL;
    return hash;
}

int AreAllFramesFull( Queue* queue )
{
    return queue->count == queue->numberOfFrames;
}


int isQueueEmpty( Queue* queue )
{
    return queue->rear == NULL;
}

int node_out_of_wsarray(Queue* queue,int*** ws_array, int k){

  if( isQueueEmpty( queue ) )
      return;

  int n = 0; // number of node to delete


  QNode* queue_node = queue->front;
  while ( queue_node != NULL ){

        for (int i = 0; i<k; i++){
            if (ws_array[i][0] != queue_node->pageNumber){
                return n+1;
            }
        }
        queue_node = queue_node->next;
        n++;

   }
   return -1;

}

void deleteNode(Queue* queue,QNode* del)
{
    /* base case */
    if (queue->front == NULL || del == NULL)
        return;

    /* If node to be deleted is head node */
    if (queue->front == del)
        queue->front = del->next;

    /* Change next only if node to be deleted is NOT
       the last node */
    if (del->next != NULL)
        del->next->prev = del->prev;

    if (del->next == NULL){
        queue->rear = del->prev;
    }
    /* Change prev only if node to be deleted is NOT
       the first node */
    if (del->prev != NULL)
        del->prev->next = del->next;

    /* Finally, free the memory occupied by del*/
    free(del);
}

void deleteNodeAtGivenPos(Queue* queue, int n, int frames, int modified, int *** inv_pageTable,counters *pageFaults)
{
    /* if list in NULL or invalid position is given */
    if (queue->front == NULL || n <= 0)
        return;

    QNode *temp = queue->rear;
    search_inv_pageTable(frames, temp->pid, temp->pageNumber, modified, inv_pageTable, pageFaults, 0);

    QNode* current = queue->front;
    int i;

    /* traverse up to the node at position 'n' from
       the beginning */
    for (int i = 1; current != NULL && i < n; i++)
        current = current->next;

    /* if 'n' is greater than the number of nodes
       in the doubly linked list */
    if (current == NULL)
        return;

    /* delete the node pointed to by 'current' */
    deleteNode(queue->front, current);
}


void deQueue( Queue* queue, int frames, int modified, int *** inv_pageTable,counters *pageFaults)
{
    if( isQueueEmpty( queue ) )
        return;

    if (queue->front == queue->rear)
        queue->front = NULL;

    QNode* temp = queue->rear;
    queue->rear = queue->rear->prev;

    if (queue->rear)
        queue->rear->next = NULL;

    //'delete' (set invalid) in invert table
    search_inv_pageTable(frames, temp->pid, temp->pageNumber, modified, inv_pageTable, pageFaults, 0);
    free( temp );

    queue->count--;
}

void Enqueue( Queue* queue, Hash* hash, int pageNumber, int pid,
                int frames, int modified, counters *pageFaults,int*** inv_pageTable)
{
    if ( AreAllFramesFull ( queue ) )
    {
        //we swap out and in,so increment counters
        pageFaults->reads_counter++;
        pageFaults->writes_counter++;


        //set invert page table
        //we want to add and in deQueue 'delete'(set invalid) pid,pageNum
        int add = 1;
        search_inv_pageTable(frames, pid, pageNumber, modified, inv_pageTable, pageFaults, add);


        //we can see
        pageFaults->pageFaults++;
        if (pid == 1){ pageFaults->proc1_pageFaults++; }
        else{ pageFaults->proc2_pageFaults++; }

        hash->array[ queue->rear->pageNumber ] = NULL;
        deQueue( queue , frames, modified, inv_pageTable ,pageFaults);
    }

    QNode* temp = newQNode( pageNumber, pid );
    temp->next = queue->front;

    if ( isQueueEmpty( queue ) )
        queue->rear = queue->front = temp;
    else
    {
        queue->front->prev = temp;
        queue->front = temp;
    }

    hash->array[ pageNumber ] = temp;


    queue->count++;
}

//reads writers counter may ++,inv table
void ReferencePage( Queue* queue, Hash* hash, int pageNumber, int pid,
                    int frames, int modified, counters *pageFaults, int*** inv_pageTable )
{
    QNode* reqPage = hash->array[ pageNumber ];
    if ( reqPage == NULL ){
        Enqueue( queue, hash, pageNumber, pid,
                frames, modified, pageFaults, inv_pageTable );
    }
    else if (reqPage != queue->front)
    {
        reqPage->prev->next = reqPage->next;
        if (reqPage->next)
           reqPage->next->prev = reqPage->prev;
         if (reqPage == queue->rear)
        {
           queue->rear = reqPage->prev;
           queue->rear->next = NULL;
        }
        reqPage->next = queue->front;
        reqPage->prev = NULL;
        reqPage->next->prev = reqPage;

        queue->front = reqPage;
    }
}




//############################## for MV,identically it would be implemented inside ReferencePage with some small changes


void Enqueue2( Queue* queue, Hash* hash, int pageNumber, int pid,
                int frames, int modified, counters *pageFaults,int*** inv_pageTable, int ***ws_array,int k)
{
    if ( AreAllFramesFull ( queue ) )
    {
        //we swap out and in,so increment counters
        pageFaults->reads_counter++;
        pageFaults->writes_counter++;


        //set invert page table
        //we want to add and in deQueue 'delete'(set invalid) pid,pageNum
        int add = 1;
        search_inv_pageTable(frames, pid, pageNumber, modified, inv_pageTable, pageFaults, add);


        //we can see
        pageFaults->pageFaults++;
        if (pid == 1){ pageFaults->proc1_pageFaults++; }
        else{ pageFaults->proc2_pageFaults++; }

        hash->array[ queue->rear->pageNumber ] = NULL;
        int n = node_out_of_wsarray(queue,ws_array,k);
        deleteNodeAtGivenPos( queue ,n, frames, modified, inv_pageTable ,pageFaults);
    }
}

ReferencePage2(Queue* queue, Hash* hash, int pageNumber, int pid,
                    int frames, int modified, counters *pageFaults, int*** inv_pageTable ,int ***ws_array,int k){
  {
      QNode* reqPage = hash->array[ pageNumber ];
      if ( reqPage == NULL ){
          Enqueue2( queue, hash, pageNumber, pid, frames, modified, pageFaults, inv_pageTable, ws_array, k);
      }
      else if (reqPage != queue->front)
      {
          reqPage->prev->next = reqPage->next;
          if (reqPage->next)
             reqPage->next->prev = reqPage->prev;
           if (reqPage == queue->rear)
          {
             queue->rear = reqPage->prev;
             queue->rear->next = NULL;
          }
          reqPage->next = queue->front;
          reqPage->prev = NULL;
          reqPage->next->prev = reqPage;

          queue->front = reqPage;
      }
  }
}
