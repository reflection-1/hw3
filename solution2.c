#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include <stdbool.h>

#define ARRIVAL_EVENT 1
#define START_SERVICE_EVENT 2
#define DEPARTURE_EVENT 3

#define SERVER_IDLE 1
#define SERVER_BUSY 2

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

void enqueue(struct Queue* queue, struct QueueNode* node) {
    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

struct QueueNode* dequeue(struct Queue* queue) {
    if (queue->head == NULL) {
        return NULL;
    }

    struct QueueNode* removedNode = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    return removedNode;
}

void newArrival(struct Queue* queue) {
  if (queue->last != NULL && queue->last->next != NULL) {
    queue->last = queue->last->next;
    queue->waiting_count++;
  } else {
    // Handle the case where queue->last->next is NULL
    printf("Cannot move to next arrival: end of queue reached\n");
  }
}

struct QueueNode* serveFirstInLine(struct Queue* queue) {
 // Check if the queue is empty
    if (queue->first == NULL) {
        printf("Queue is empty. No one to serve.\n");
        return NULL;
    }

    // Retrieve the first node in line
    struct QueueNode* servedNode = queue->first;

    // Update the first pointer to point to the next node in line
    queue->first = queue->first->next;

    // If the queue becomes empty after serving the first person, update the last pointer as well
    if (queue->first == NULL) {
        queue->last = NULL;
    }

    // Decrement the waiting count
    queue->waiting_count--;

    return servedNode;
}

bool isEmpty(struct Queue* queue) {
  return queue->head == NULL;
}


// ------------Global variables----------------------------------------------------------------------
// Feel free to add or remove. 
// You could make these local variables for cleaner code, but you need to change function definitions 
static double computed_stats[4];  // Store computed statistics: [E(n), E(r), E(w), p0]
static double simulated_stats[4]; // Store simulated statistics [n, r, w, sim_p0]
int departure_count = 0;         // current number of departures from queue
double current_time = 0;          // current time during simulation
double last_event_time = 0;       // time of the last event during simulation
int server_status = SERVER_IDLE;

void updateSimulatedMeanNrOfCustomers(double customers) {
  simulated_stats[0] += customers;
}
void updateSimulatedResponseTime(double time) {
  simulated_stats[1] += time;
}
void updateSimulatedMeanWaitingTime(double time) {
  simulated_stats[2] += time;
}

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
struct Queue* InitializeQueue(int seed, double lambda, double mu, int total_departures){
  // Seed the random number generator
  srand(seed);

  // Create a new queue
  struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
  if (queue == NULL) {
      fprintf(stderr, "Memory allocation failed for queue.\n");
      exit(EXIT_FAILURE);
  }

  // Initialize queue variables
  queue->head = NULL;
  queue->tail = NULL;
  queue->first = NULL;
  queue->last = NULL;
  queue->waiting_count = 0;
  queue->cumulative_response = 0;
  queue->cumulative_waiting = 0;
  queue->cumulative_idle_times = 0;
  queue->cumulative_area = 0;

  // Generate D exponentially distributed inter-arrival times based on lambda
  // and insert D elements in the queue with the correct arrival times
  double inter_arrival_time;
  double arrival_time = 0;
  for (int i = 0; i < total_departures; i++) {
      inter_arrival_time = -log((double)rand() / RAND_MAX) / lambda;
      arrival_time += inter_arrival_time;

      // Create a new queue node
      struct QueueNode* node = (struct QueueNode*)malloc(sizeof(struct QueueNode));
      if (node == NULL) {
          fprintf(stderr, "Memory allocation failed for queue node.\n");
          exit(EXIT_FAILURE);
      }

      node->arrival_time = arrival_time;
      node->service_time = -log((double)rand() / RAND_MAX) / mu;

      enqueue(queue, node);
  }
  
  printf("Initialized Queue \n");
  return queue;
}

// Use the M/M/1 formulas from class to compute E(n), E(r), E(w), p0
void GenerateComputedStatistics(double lambda, double mu){
  double rho = lambda / mu;
  computed_stats[3] = 1.0 - rho; // p0

  computed_stats[0] = rho / (1.0 - rho); // E(n)
  computed_stats[1] = 1.0 / mu; // E(r)
  computed_stats[2] = rho / (mu * (1.0 - rho)); // E(w)
}

// This function should be called to print periodic and/or end-of-simulation statistics
// Do not modify output format
void PrintStatistics(struct Queue* elementQ, int total_departures, int print_period, double lambda){

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
struct QueueNode* ProcessArrival(struct Queue* elementQ, struct QueueNode* arrival){
  // Create next arrival

  // TODO: Update Queue & Statistics
  updateSimulatedMeanNrOfCustomers(1);

  // Start service if server is idle
  if (server_status == SERVER_IDLE) {
  }

  return arrival->next;
}

// This function is called from simulator if next event is "start_service"
//  Should update queue statistics
void StartService(struct Queue* elementQ){
  server_status = SERVER_BUSY;

  if (isEmpty(elementQ)) {
    printf("Queue is empty. Exiting StartService.\n");
            return;
  }
  // TODO: Update statistics
  // Remove customer from head of queue
  struct QueueNode* customer = serveFirstInLine(elementQ);

  // Schedule departure event
}

// This function is called from simulator if the next event is a departure
// Should update simulated queue statistics 
// Should update current queue nodes and various queue member variables
void ProcessDeparture(struct Queue* elementQ, struct QueueNode* arrival){
  // TODO: Update Statistics
 
  if (!isEmpty(elementQ)) {
    printf("Departure, starting new service\n");
  }
  
  server_status = SERVER_IDLE;
  
  departure_count++;
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
  server_status = SERVER_IDLE;
  
  while (departure_count < total_departures && departure_count < 5) {
    // start service

    if (elementQ->last->next->arrival_time) {

    }
  }
  // Print Statistics at end of simulation
  PrintStatistics(elementQ, total_departures, print_period, lambda);
}


// Free memory allocated for queue at the end of simulation
void FreeQueue(struct Queue* elementQ) {

}

// Program's main function
int main(int argc, char* argv[]){

	// input arguments lambda, mu, P, D, S
	if(argc >= 6){

		double lambda = atof(argv[1]);
		double mu = atof(argv[2]);
		int print_period = atoi(argv[3]);
		int total_departures = atoi(argv[4]);
		int random_seed = atoi(argv[5]);
   
   // Add error checks for input variables here, exit if incorrect input

   // If no input errors, generate M/M/1 computed statistics based on formulas from class
   	GenerateComputedStatistics(lambda, mu);

   // Start Simulation
		printf("Simulating M/M/1 queue with lambda = %f, mu = %f, P = %d, D = %d, S = %d\n", lambda, mu, print_period, total_departures, random_seed); 
		struct Queue* elementQ = InitializeQueue(random_seed, lambda, mu, total_departures);
		Simulation(elementQ, lambda, mu, print_period, total_departures);
    FreeQueue(elementQ);
	}
	else printf("Insufficient number of arguments provided!\n");
   
	return 0;
}
