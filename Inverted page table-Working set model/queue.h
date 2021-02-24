#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model_impl.h"

typedef struct QNode
{
    struct QNode *prev, *next;
    int pageNumber;
    int pid;
} QNode;

typedef struct Queue
{
    int count;
    int numberOfFrames;
    QNode *front, *rear;
} Queue;

typedef struct Hash
{
    int capacity;
    QNode* *array;
} Hash;

QNode* newQNode( int, int  );
Queue* createQueue( int  );
Hash* createHash( int  );
int AreAllFramesFull( Queue*  );
int isQueueEmpty( Queue*  );
void deQueue( Queue* , int, int,int***, counters* pageFaults );
void Enqueue( Queue*, Hash*, int ,int, int, int, counters*, int*** );
void ReferencePage( Queue*, Hash*, int, int, int, int, counters*, int*** );







//##################### could just alter a little the above functions instead of these

void ReferencePage2( Queue*, Hash*, int, int, int, int, counters*, int*** ,int*** ,int);
void Enqueue2( Queue*, Hash*, int,int ,int, int, counters*, int*** ,int*** ,int);
void deleteNodeAtGivenPos(Queue* , int , int , int , int *** ,counters *);
void deleteNode(Queue* ,QNode* );
int node_out_of_wsarray(Queue* ,int*** , int );
