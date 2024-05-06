#include <stdio.h>
#include "queue.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/*
 * Custom testing framework
 */

void dumpQueue(struct Queue *q);

/*
 * Main()
 */
int main(int argc, char** argv)
{
     // start with empty Queue
     struct Queue *Q = malloc(sizeof(struct Queue));
     Q->head = NULL;
     Q->tail = NULL;
     Q->service_time = 2;
     Q->arrival_count = 0;
     Q->departure_count = 0;
     
     // Add first node to queue
     Q->head = CreateNode (100, 1);
     Q->tail = Q->head;   // Queue has single node {100, 1}
     Q->arrival_count ++;
     Insert (Q, 101, 7);  // Queue: {100, 1} {101, 7}
     Insert (Q, 102, 11); // Queue: {100, 1} {101, 7} {102, 11}
     Insert (Q, 103, 12); // Queue: {100, 1} {101, 7} {102, 11} {103, 12}
     Insert (Q, 104, 13); // Queue: {100, 1} {101, 7} {102, 11} {103, 12} {104, 13}
     Insert (Q, 105, 27); // Queue: {100, 1} {101, 7} {102, 11} {103, 12} {104, 13} {105, 27}
     dumpQueue(Q);
     printf("Item Count at time 14 = %5lu\n", CountItems(Q, 14));   // should return 2
     printf("Item Count at time 15 = %5lu\n", CountItems(Q, 15));   // should return 2 since item 103 departs at t=15 is counted
     printf("Item Count With Time > 10 = %5lu\n", CountItemsAfterTime(Q, 10));  // should return 4
     Delete(Q); // Queue should be {101, 7} {102, 11} {103, 12} {104, 13} {105, 27}
     dumpQueue(Q);
     printf("Average Response Time for customers after time 10 = %f\n",
             AverageResponseTime(Q, 10));  // should return 2.75 - response times are 2 (item 102), 3 (item 103), 4 (item 104), 2 (item 105))
     printf("Utilization after time 10 = %5.3f\n", Utilization(Q, 10));  // should return 8/19 = 0.421
     printf("Throughput after time 10 = %5.3f\n", Throughput(Q, 10));  // should return 4/19 = 0.211 since departures=4 , total_time = 19
     

     // free queue
     struct QueueNode *ptr = Q->head;
     while (ptr) {
            Q->head = ptr->next;
            free (ptr);
            ptr = Q->head;
     }
     Q->tail = NULL;
     free(Q);
     return 0;
}

/*
 * Helper Functions
 */
void dumpQueue(struct Queue *q)
{
    struct QueueNode *current = q->head;
    while (current != NULL) {
        printf("{%d,%lu} ", current->item, current->arrival_time);
        current = current->next;
    }
    printf("\n");
  printf("Total Arrivals = %lu\n", q->arrival_count);
  printf("Total Departures = %lu\n", q->departure_count);
}


