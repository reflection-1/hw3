#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include<pthread.h>

#define BATCH_SIZE 1000

// Definition of a Queue Node including arrival and service time
struct QueueNode {
    double arrival_time;  // customer arrival time, measured from time t=0, inter-arrival times exponential
    double service_time;  // customer service time (exponential)
    struct QueueNode *next;  // next element in line; NULL if this is the last element
};

// Suggested queue definition
// Feel free to use some of the functions you implemented in HW2 to manipulate the queue
// You can change the queue definition and add/remove member variables to suit your implementation
struct Queue {

    struct QueueNode* head;    // Point to the first node of the element queue
    struct QueueNode* tail;    // Point to the tail node of the element queue

    struct QueueNode* first;  // Point to the first arrived customer that is waiting for service
    struct QueueNode* last;   // Point to the last arrrived customer that is waiting for service
    int waiting_count;     // Current number of customers waiting for service

    double cumulative_response;  // Accumulated response time for all effective departures
    double cumulative_waiting;  // Accumulated waiting time for all effective departures
    double cumulative_idle_times;  // Accumulated times when the system is idle, i.e., no customers in the system
    double cumulative_area;   // Accumulated number of customers in the system multiplied by their residence time, for E[n] computation
};
struct Event {
    double time;
    int type; // 0 for arrival, 1 for departure
    struct QueueNode* node; // Pointer to the corresponding queue node
    struct Event* next; // Pointer to the next event in the list
};


// ------------Global variables----------------------------------------------------------------------
// Feel free to add or remove.
// You could make these local variables for cleaner code, but you need to change function definitions
static double computed_stats[4];  // Store computed statistics: [E(n), E(r), E(w), p0]
static double simulated_stats[4]; // Store simulated statistics [n, r, w, sim_p0]
int departure_count = 0;         // current number of departures from queue
double current_time = 0;          // current time during simulation
double last_event_time = 0;       // time of the last event during simulation
struct Event* eventList = NULL;
//-----------------Queue Functions--------------------------------------------------------------------
// Feel free to add more functions or redefine the following functions

// The following function initializes all "D" (i.e., total_departure) elements in the queue
// 1. It uses the seed value to initialize random number generator
// 2. Generates "D" exponentially distributed inter-arrival times based on lambda
//    And inserts "D" elements in queue with the correct arrival times
//    Arrival time for element i is computed based on the arrival time of element i+1 added to element i's generated inter-arrival time
//    Arrival time for first element is just that element's generated inter-arrival time
// 3. Generates "D" exponentially distributed service times based on mu
//    And updates each queue element's service time in order based on generated service times
// 4. Returns a pointer to the generated queue

void scheduleEvent(struct Event** eventList, double time, int type, struct QueueNode* node) ;
struct Event* getNextEvent(struct Event** eventList);

double exponentialRandom(double lambda) {
    double u = rand() / (RAND_MAX + 1.0); // Generate a uniform random number between 0 and 1
    return -log(1 - u) / lambda; // Compute the inverse transform
}

struct Queue* InitializeQueue(int seed, double lambda, double mu, int total_departures){
    // Initialize random number generator
    srand(seed);
        struct Queue* elementQ = (struct Queue*)malloc(sizeof(struct Queue));
        elementQ->head = NULL;
        elementQ->tail = NULL;
        elementQ->first = NULL;
        elementQ->last = NULL;
        elementQ->waiting_count = 0;
        elementQ->cumulative_response = 0;
        elementQ->cumulative_waiting = 0;
        elementQ->cumulative_idle_times = 0;
        elementQ->cumulative_area = 0;

        // Generate arrivals and service times
        double interarrival_time = exponentialRandom(lambda);
        double service_time = exponentialRandom(mu);

        // Initialize first node
        struct QueueNode* newNode = (struct QueueNode*)malloc(sizeof(struct QueueNode));
        newNode->arrival_time = interarrival_time;
        newNode->service_time = service_time;
        newNode->next = NULL;
        elementQ->head = newNode;
        elementQ->tail = newNode;
        elementQ->first = newNode;

        // Generate remaining arrivals and service times
    for (int i = 1; i < total_departures; i++) {
        interarrival_time = exponentialRandom(lambda);
        service_time = exponentialRandom(mu);
        newNode = (struct QueueNode*)malloc(sizeof(struct QueueNode));
        newNode->arrival_time = elementQ->tail->arrival_time + interarrival_time;
        newNode->service_time = service_time;
        newNode->next = NULL;
        scheduleEvent(&eventList, newNode->arrival_time, 0, newNode);
        elementQ->tail->next = newNode;
        elementQ->tail = newNode;
    }
    return elementQ;
}

// Use the M/M/1 formulas from class to compute E(n), E(r), E(w), p0
void GenerateComputedStatistics(double lambda, double mu){
    // Compute computed statistics using M/M/1 formulas
    computed_stats[0] = lambda / (mu - lambda); // E[n]
    computed_stats[1] = 1 / (mu - lambda); // E[r]
    computed_stats[2] = lambda / (mu * (mu - lambda)); // E[w]
    computed_stats[3] = 1 - (lambda / mu); // p0
}

// This function should be called to print periodic and/or end-of-simulation statistics
// Do not modify output format
void PrintStatistics(struct Queue* elementQ, int total_departures, int print_period, double lambda){
    // Compute simulated statistics
        simulated_stats[0] = elementQ->cumulative_area / current_time;
        simulated_stats[1] = elementQ->cumulative_response / total_departures;
        simulated_stats[2] = elementQ->cumulative_waiting / total_departures;
        simulated_stats[3] = (double)elementQ->cumulative_idle_times / current_time;

  printf("\n");
  if(departure_count == total_departures) printf("End of Simulation - after %d departures\n", departure_count);
  else printf("After %d departures\n", departure_count);

  printf("Mean n = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[0], computed_stats[0]);
  printf("Mean r = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[1], computed_stats[1]);
  printf("Mean w = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[2], computed_stats[2]);
  printf("p0 = %.4f (Simulated) and %.4f (Computed)\n", simulated_stats[3], computed_stats[3]);
}

// This function is called from simulator if the next event is an arrival
// Should update simulated statistics based on new arrival
// Should update current queue nodes and various queue member variables
// *arrival points to queue node that arrived
// Returns pointer to node that will arrive next
struct QueueNode* ProcessArrival(struct Queue* elementQ, struct QueueNode* arrival, double current_time){
    // Update simulated statistics
    printf("Processing arrival\n");
    if (arrival == NULL) {
        printf("Error: Arrival node is NULL\n");
        return NULL;
    }
    printf("Arrival time: %f\n", arrival->arrival_time);

    // Update cumulative area
    elementQ->cumulative_area += (current_time - last_event_time) * elementQ->waiting_count;

    // Update queue nodes and variables
    if (elementQ->waiting_count == 0)
        elementQ->first = arrival;
    else
        elementQ->last->next = arrival;

    elementQ->last = arrival;
    elementQ->waiting_count++;

    // Move current time to arrival time
    last_event_time = current_time;

    // Update mean number of customers (n)
    simulated_stats[0] = elementQ->cumulative_area / current_time;

    return arrival;
}

// This function is called from simulator if next event is "start_service"
//  Should update queue statistics
void StartService(struct Queue* elementQ){
    // Update simulated statistics
    printf("Starting service\n");
    if (elementQ->first != NULL) {
        double wait_time = current_time - elementQ->first->arrival_time;
        elementQ->cumulative_waiting += wait_time;
        elementQ->cumulative_response += wait_time; // Also update cumulative response
        elementQ->waiting_count--;
        
        // Schedule departure event for the customer being served
        double service_time = elementQ->first->service_time;
        scheduleEvent(&eventList, current_time + service_time, 1, elementQ->first);
        
        if (elementQ->first->next != NULL) {
            current_time = elementQ->first->next->arrival_time;
            elementQ->first = elementQ->first->next;
        } 
        else {
            elementQ->first = NULL;
            elementQ->last = NULL;
        }
    } 
    else {
        printf("Error: Attempted to start service with an empty queue.\n");
    }
    last_event_time = current_time;
}

// This function is called from simulator if the next event is a departure
// Should update simulated queue statistics
// Should update current queue nodes and various queue member variables
void ProcessDeparture(struct Queue* elementQ, struct QueueNode* departing_node, double current_time){
    // Update simulated statistics
    printf("Processing departure\n");
    double wait_time = current_time - departing_node->arrival_time;
    elementQ->cumulative_response += (departing_node->service_time + wait_time);
    departure_count++;
    
   /* current_time += departing_node->service_time;
    if (departing_node->next != NULL) {
        elementQ->first = departing_node->next;
    } else {
        // If there are no more customers in the queue, set first and last to NULL
        elementQ->first = NULL;
        elementQ->last = NULL;
    }
    last_event_time = current_time; */

    // Update mean response time (r)
    simulated_stats[1] = elementQ->cumulative_response / departure_count;

    // Update mean waiting time (w)
    simulated_stats[2] = elementQ->cumulative_waiting / departure_count;

    // Update p0
    double cumulative_busy_times = elementQ->cumulative_response + elementQ->cumulative_waiting;
    elementQ->cumulative_idle_times = current_time - cumulative_busy_times;
    simulated_stats[3] = (double)elementQ->cumulative_idle_times / current_time;
}

// This is the main simulator function
// Should run until departure_count == total_departures
// Determines what the next event is based on current_time
// Calls appropriate function based on next event: ProcessArrival(), StartService(), ProcessDeparture()
// Advances current_time to next event
// Updates queue statistics if needed
// Print statistics if departure_count is a multiple of print_period
// Print statistics at end of simulation (departure_count == total_departures)
void Simulation(struct Queue* elementQ, double lambda, double mu, int print_period, int total_departures){
    struct Event *currentEvent;
    double simulation_clock = 0.0; // Initialize simulation clock starting at time 0

    // Schedule the first arrival event
    double next_arrival_time = exponentialRandom(lambda);
    struct QueueNode *arrival_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
    arrival_node->arrival_time = next_arrival_time;
    arrival_node->service_time = exponentialRandom(mu); // Generate service time for the arrival
    arrival_node->next = NULL;

    scheduleEvent(&eventList, next_arrival_time, 0, arrival_node);

    while (departure_count < total_departures) {
        printf("Inside simulation loop\n");

        // Get the next event
        currentEvent = getNextEvent(&eventList);
        if (currentEvent == NULL) {
            break;
        }

        printf("Current event: %p\n", (void*)currentEvent);
        printf("Processing event at time %f\n", currentEvent->time);

        // Advance simulation clock to the time of the current event
        simulation_clock = currentEvent->time;

        // Process current event
        if (currentEvent->type == 0) { // Arrival event
            printf("Processing arrival event\n");
            ProcessArrival(elementQ, currentEvent->node, simulation_clock);
            // Schedule next arrival event
            double interarrival_time = exponentialRandom(lambda);
            next_arrival_time += interarrival_time;
            struct QueueNode *next_arrival_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
            next_arrival_node->arrival_time = next_arrival_time;
            next_arrival_node->service_time = exponentialRandom(mu);
            next_arrival_node->next = NULL;
            scheduleEvent(&eventList, next_arrival_time, 0, next_arrival_node);
        } 
        else if (currentEvent->type == 1) { // Departure event
            printf("Processing departure event\n");
            ProcessDeparture(elementQ, currentEvent->node, simulation_clock);
        }

        printf("Event processing complete\n");

        if ((departure_count % print_period) == 0) {
            PrintStatistics(elementQ, total_departures, print_period, lambda);
        }
    }

    // Print Statistics at end of simulation
    PrintStatistics(elementQ, total_departures, print_period, lambda);
}

// Free memory allocated for queue at the end of simulation
void FreeQueue(struct Queue* elementQ) {
    // Free memory allocated for the queue
    struct QueueNode* current = elementQ->head;
    struct QueueNode* temp;

    while (current != NULL) {
          temp = current;
          current = current->next;
          free(temp);
    }
    free(elementQ);
}

// Program's main function
int main(int argc, char* argv[]){

    // input arguments lambda, mu, P, D, S
    //if(argc >= 6){

      /*  double lambda = atof(argv[1]);
        double mu = atof(argv[2]);
        int print_period = atoi(argv[3]);
        int total_departures = atoi(argv[4]);
        int random_seed = atoi(argv[5]); */
    
    double lambda = 4;
    double mu = 5;
    int print_period = 1000;
    int total_departures = 2500;
    int random_seed = 534;
   
   // Add error checks for input variables here, exit if incorrect input

   // If no input errors, generate M/M/1 computed statistics based on formulas from class
       GenerateComputedStatistics(lambda, mu);

   // Start Simulation
        printf("Simulating M/M/1 queue with lambda = %f, mu = %f, P = %d, D = %d, S = %d\n", lambda, mu, print_period, total_departures, random_seed);
        struct Queue* elementQ = InitializeQueue(random_seed, lambda, mu, total_departures);
        Simulation(elementQ, lambda, mu, print_period, total_departures);
        FreeQueue(elementQ);
    //}
    /*else {
        printf("Insufficient number of arguments provided!\n");
        return 1;
     }
    */
   
    return 0;
}
    // Function to schedule a new event
void scheduleEvent(struct Event** eventList, double time, int type, struct QueueNode* node) {
    struct Event* newEvent = (struct Event*)malloc(sizeof(struct Event));
     newEvent->time = time;
     newEvent->type = type;
     newEvent->node = node;
     newEvent->next = NULL;

     // Insert the new event into the list in chronological order
     if (*eventList == NULL || time < (*eventList)->time) {
         newEvent->next = *eventList;
         *eventList = newEvent;
     } else {
         struct Event* current = *eventList;
         while (current->next != NULL && current->next->time < time) {
             current = current->next;
         }
         newEvent->next = current->next;
         current->next = newEvent;
     }
}
struct Event* getNextEvent(struct Event** eventList) {
        if (*eventList == NULL) {
            printf("getNextEvent: Event list is empty\n");
            return NULL;
        }
        struct Event* nextEvent = *eventList;
        *eventList = (*eventList)->next;
        return nextEvent;
}
