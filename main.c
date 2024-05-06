/* In the following code, you need to fill in body
 * of each function
*/
#include "queue.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
/*
 * Allocate memory for a node of type struct QueueNode and
 * initialize it with the item value and its arrival_time;
 * Return a pointer to the new node.
 */
struct QueueNode* CreateNode(int item, uint64 arrival_time) {
    struct QueueNode* newNode = (struct QueueNode*)malloc(sizeof(struct QueueNode));
    if (newNode) {
            newNode->item = item;
            newNode->arrival_time = arrival_time;
            newNode->next = NULL;
        }
    return newNode;
}

/*
 * Insert node with specified item and arrival time at the tail of the queue.
 * If the queue is empty, then both the head and the tail should point to the new node
 * Should increment the arrival_count of the queue
 */
void Insert (struct Queue *q, int item, uint64 arrival_time) {
    struct QueueNode* newNode = CreateNode(item, arrival_time);
    if (q->head == NULL) {
            q->head = newNode;
            q->tail = newNode;
    } 
    else {
            q->tail->next = newNode;
            q->tail = newNode;
    }
    q->arrival_count++;
}
/*
 * Delete node from head of the queue and free its allocated memory
 * head pointer should move to the next node in the queue.
 * If head == NULL, do nothing
 * If the queue has one node, then both the head and the tail should point to NULL after the node is deleted
 * Should increment the departure_count of the queue
 */
void Delete (struct Queue *q) {
    if (q->head == NULL)
            return;
        
    struct QueueNode* temp = q->head;
    q->head = q->head->next;
    free(temp);
    if (q->head == NULL)
        q->tail = NULL;
    q->departure_count++;
}

/*
 * Count the number of items in the queue at time "current_time".
 * If a node arrives or departs at exactly "current_time" it should be counted.
 * Do NOT count items that have departed before "current_time" or arrived after "current_time"
 * You need to traverse the queue from head until you reach or exceed current_time.
 * For each arrival, increment node_count
 * For each departure, decrement node_count
 * For each node, compute when it will depart (service time + wait time for previous items to complete service)
 */
uint64 CountItems (struct Queue *q, uint64 current_time) {
    uint64 node_count = 0;

        // Traverse the queue from head until we reach or exceed current_time
        struct QueueNode* current = q->head;
    uint64 departure_time = current->arrival_time + q->service_time;
        while (current != NULL && current->arrival_time <= current_time) {
            
            if (current->arrival_time > departure_time)
                        departure_time = current->arrival_time;
                    departure_time += q->service_time;
            // If the departure time is after current_time, count this node
            if (departure_time >= current_time)
                node_count++;

            current = current->next;
        }

        return node_count;
}

/*
 * Return the first node holding the value item
 * return NULL if none found
 */
struct QueueNode* FindNode(struct Queue *q, int item) {
    struct QueueNode* current = q->head;
        while (current != NULL) {
            if (current->item == item)
                return current;
            current = current->next;
        }
    return NULL;
}

/*
 * Return a pointer to first node that has an arrival time > t
 * Returns NULL if zero nodes arrived after time t
 * For example, in the queue head:{1,4}, {2, 80}, {3, 500}, {4, 510}, tail:{5, 640}
 * FindNodeAfterTime(q, 505) returns a pointer to QueueNode {4, 510}
 * FindNodeAfterTime(q, 80) returns a pointer to QueueNode {3, 500}
 * FindNodeAfterTime(q, 700) returns NULL
 */
struct QueueNode* FindNodeAfterTime(struct Queue *q, uint64 t) {
    struct QueueNode* current = q->head;
    while (current != NULL) {
        if (current->arrival_time > t)
            return current;
        current = current->next;
    }
    return NULL;
}

/*
 * Return the count of all items that have an arrival time > t
 * For example, in the queue head:{1,4}, {2, 80}, {3, 500}, {4, 510}, tail:{5, 640}
 * CountItemsAfterTime(q, 505) returns 2
 * CountItemsAfterTime(q, 80) returns 3
 */
uint64 CountItemsAfterTime(struct Queue *q, uint64 t) {
    uint64 count = 0;
    struct QueueNode* current = q->head;
    while (current != NULL) {
        if (current->arrival_time > t)
                count++;
        current = current->next;
    }
    return count;
}

/*
 * Compute the average response time for all nodes in the queue that have arrival_time > current_time
 * If no nodes arrive after current_time, return 0
 * You should traverse the queue and compute the response time for each node.
 * Note that the response time of nodes that arrive after current_time could be affected by nodes that arrived at or before current_time
 * The response time for a node should be the same as service_time if there are no other nodes in the queue
 * The response time should be service_time+queueing delay if the current node overlaps with previous node(s)
 * You need to add up all the response times of all nodes arriving after current_time and divide by their count to get an average response time
 * Example: In the queue head:{1, 4}, {2, 5}, tail:{3, 6} with service_time=2
 * 1 arrives at time 4, leaves at 6; 2 arrives at 5, leaves at 8; 3 arrives at 6, leaves at 10
 * AverageResponseTime(q,5) = 4 (only includes node 3)
 * AverageResponseTime(q,4) = 3.5 (includes nodes 2, 3)
 * AverageResponseTime(q,6) = 0 (no nodes arrive after time 6)
 */
float AverageResponseTime(struct Queue *q, uint64 current_time) {
    uint64 total_response_time = 0;
    uint64 node_count = 0;

       // If the queue is empty, return 0
       if (q->head == NULL)
           return 0;

       // Traverse the queue and compute the response time for each node arriving after current_time
       struct QueueNode* current = q->head;
       while (current != NULL) {
           if (current->arrival_time > current_time) {
               // Compute the response time for the current node
               uint64 response_time = q->service_time;
               if (current->arrival_time < current_time + q->service_time) {
                   response_time += current->arrival_time - current_time;
               }
               total_response_time += response_time;
               node_count++;
           }
           else if (current_time < current->arrival_time + q->service_time) {
                    total_response_time += q->service_time + (current->arrival_time - current_time);
                        node_count++;
           }
           current = current->next;
       }
    printf("Total response time: %lu\n", total_response_time);
    printf("Number of nodes: %lu\n", node_count);
       // Calculate the average response time
       if (node_count > 0)
           return (float)total_response_time / node_count;
       else
           return 0; // No nodes arrive after current_time
}

/*
 * Compute the queue utilization starting from current_time until the last node departs
 * Should return busy_time/total_time
 * total_time = completion time of last node - current time
 * busy_time is the total time after current_time where a node is in service
 * Note: Include service even for nodes that arrives before current_time that were in the queue at current_time
 * Example: In the queue head:{1, 4}, {2, 7}, {3, 8}; tail:{4, 10} with service_time=2
 * Serving node 1 from 4-6, 2 from 7-9, 3 from 9-11, 4 from 11-13.
 * Utilization(q,5) = 0.875  (busy_time = 7, total_time = 13-5 = 8)
 * Utilization(q,3) = 0.8 (busy_time = 8; total_time = 10)
 * Utilization(q,9) = 1.0 (busy_time = 4; total_time = 4)
 * Utilization(q,13) = 0.0 (no nodes in service after time 13)
 */
float Utilization(struct Queue *q, uint64 current_time) {
    if (!q->head)
           return 0.0; // Queue is empty

       uint64 busyTime = 0;
       struct QueueNode* current = q->head;
       while (current) {
           if (current->arrival_time <= current_time) {
               busyTime += (current_time - current->arrival_time) + q->service_time;
           }
           current = current->next;
       }
       uint64 totalTime = (q->tail->arrival_time + q->service_time) - current_time;
       return (float)busyTime / totalTime;
}

/*
 * Compute the queue throughput starting from current_time until the last node departs
 * Do NOT include nodes that depart exactly at current_time
 * Returns (total departures after current_time)/(total_time)
 * total_time = completion time of last node - current_time
 * You can traverse the queue to compute total departures after current_time and total_time
 * Example: In the queue head:{1, 4}, {2, 7}, {3, 8}; tail:{4, 10} with service_time=2
 * Departure times: node 1 departs at time 6, 2 departs at 9, 3 departs at 11, 4 departs at 13.
 * Throughput(q,5) = 0.5 (departures = 4, total_time = 13-5 = 8)
 * Throughput(q,3) = 0.4 (departures = 4; total_time = 10)
 * Throughput(q,9) = 0.5 (departures = 2; total_time = 4)
 * Throughput(q,13) = 0.0 (no departures after time 13)
 */
float Throughput(struct Queue *q, uint64 current_time) {
    if (!q->head)
            return 0.0; // Queue is empty

        uint64 departures = 0;
        struct QueueNode* current = q->head;
        while (current) {
            if (current->arrival_time > current_time)
                departures++;
            current = current->next;
        }
        uint64 totalTime = (q->tail->arrival_time + q->service_time) - current_time;
        return (float)departures / totalTime;
}
