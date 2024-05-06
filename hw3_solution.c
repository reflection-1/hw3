#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>

#define ARRIVAL_EVENT 1
#define START_SERVICE_EVENT 2
#define DEPARTURE_EVENT 3

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

// Definition of an Event
typedef struct Event {
    double event_time;  // Time at which the event occurs
    int event_type;     // Type of the event (arrival, departure, end_simulation, etc.)
    double arrival_time;  // Customer arrival time for arrival events
    double service_time;  // Customer service time for arrival events
    struct Event* next; // Pointer to the next event in the list
} Event;

// Definition of the EventList
typedef struct {
    Event* head;        // Pointer to the first event in the list
    int num_events;     // Number of events in the list
} EventList;

// Function prototypes
void initialize_event_list(EventList* event_list);
void schedule_event(EventList* event_list, Event* event);
Event* get_next_event(EventList* event_list);
void free_event_list(EventList* event_list);
Event* create_event(double event_time, int event_type);
void GenerateComputedStatistics(double lambda, double mu);
void PrintStatistics(struct Queue* elementQ, int total_departures, int print_period, double lambda);
void Simulation(struct Queue* elementQ, EventList* event_list, double lambda, double mu, int print_period, int total_departures);
struct Queue* InitializeQueue(int seed, double lambda, double mu, int total_departures);
struct QueueNode* ProcessArrival(struct Queue* elementQ, struct QueueNode* arrival, EventList* event_list);
void StartService(struct Queue* elementQ);
void ProcessDeparture(struct Queue* elementQ, double departure_time, struct QueueNode* arrival);
void FreeQueue(struct Queue* elementQ);


// Function to initialize the event list
void initialize_event_list(EventList* event_list) {
    event_list->head = NULL;
    event_list->num_events = 0;
}

// Function to add an event to the event list
void schedule_event(EventList* event_list, Event* event) {
    event->next = NULL;
    if (event_list->head == NULL) {
        // If the list is empty, set the new event as the head
        event_list->head = event;
    } else {
        // Otherwise, find the last event and append the new event to it
        Event* current = event_list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = event;
    }
    event_list->num_events++;
}

// Function to remove and return the next event from the event list
Event* get_next_event(EventList* event_list) {
    if (event_list->head == NULL) {
        // If the list is empty, return NULL
        return NULL;
    }
    // Remove the head event and update the head pointer
    Event* next_event = event_list->head;
    event_list->head = event_list->head->next;
    event_list->num_events--;
    return next_event;
}

// Function to free memory allocated for the event list
void free_event_list(EventList* event_list) {
    Event* current = event_list->head;
    while (current != NULL) {
        Event* temp = current;
        current = current->next;
        free(temp);
    }
    event_list->num_events = 0;
}
// Define create_event function to allocate memory for a new Event
Event* create_event(double event_time, int event_type) {
    Event* new_event = (Event*)malloc(sizeof(Event));
    if (new_event != NULL) {
        new_event->event_time = event_time;
        new_event->event_type = event_type;
        new_event->next = NULL;
    }
    return new_event;
}

// ------------Global variables----------------------------------------------------------------------
// Feel free to add or remove.
// You could make these local variables for cleaner code, but you need to change function definitions
static double computed_stats[4];  // Store computed statistics: [E(n), E(r), E(w), p0]
static double simulated_stats[4]; // Store simulated statistics [n, r, w, sim_p0]
int departure_count = 0;         // current number of departures from queue
double current_time = 0;          // current time during simulation
double last_event_time = 0;       // time of the last event during simulation
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

struct Queue* InitializeQueue(int seed, double lambda, double mu, int total_departures) {
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

        // Generate service time for the node
        node->service_time = -log((double)rand() / RAND_MAX) / mu;

        // Insert the node into the queue
        node->next = NULL;
        if (queue->head == NULL) {
            queue->head = node;
            queue->tail = node;
        } else {
            queue->tail->next = node;
            queue->tail = node;
        }
    }
    printf("Initialized Queue \n");
    return queue;
}


// Use the M/M/1 formulas from class to compute E(n), E(r), E(w), p0
void GenerateComputedStatistics(double lambda, double mu){
    double rho = lambda / mu;
    computed_stats[3] = 1.0 - rho; // p0

    computed_stats[0] = rho / (1.0 - rho); // E(n)
    computed_stats[1] = 1.0 / (mu - lambda); // E(r)
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
struct QueueNode* ProcessArrival(struct Queue* elementQ, struct QueueNode* arrival, EventList* event_list) {
    // Update simulated statistics based on new arrival
    simulated_stats[0]++; // Increase number of customers in the system
    printf("Debugging - Before checking if the queue is empty\n");
    // Update queue variables
    if (elementQ->head == NULL) {
        // If the queue is empty, the arrival starts service immediately
        printf("Checkpoint 1 \n");
        elementQ->head = arrival;
        elementQ->first = arrival;
        elementQ->tail = arrival;
        elementQ->waiting_count = 0;
    }
    else {
        // If the queue is not empty, add the arrival to the tail of the queue
        elementQ->tail->next = arrival;
        elementQ->tail = arrival;
        elementQ->waiting_count++; // Increment number of waiting customers
    }
    printf("Debugging - After checking if the queue is empty\n");
    // Update last event time
    last_event_time = current_time;
    // Schedule a "start_service" event for the arrived customer
    double service_time = arrival->service_time; // Assuming service time is available
    double start_service_time = current_time;
    Event* start_service_event = create_event(start_service_time, START_SERVICE_EVENT);
    assert(start_service_event != NULL);
    schedule_event(event_list, start_service_event);
    printf("Processed Arrival");
    // Return the next arrival node (arrival is already added to the queue)
    return arrival->next;
}


// This function is called from simulator if next event is "start_service"
//  Should update queue statistics
void StartService(struct Queue* elementQ) {
    printf("Inside StartService\n");
    if (elementQ->head == NULL) {
            printf("Queue is empty. Exiting StartService.\n");
            return;
    }
    // Update queue statistics
    struct QueueNode* current_customer = elementQ->head;
    double service_time = current_customer->service_time;

    // Update cumulative idle times
    elementQ->cumulative_idle_times += current_time - last_event_time;

    // Update simulated statistics
    simulated_stats[1] += service_time; // Increase simulated response time

    // Remove the customer from the head of the queue
    elementQ->head = current_customer->next;
    elementQ->waiting_count--;

    // Update last event time
    last_event_time = current_time;
    printf("Service fn \n");
}

// This function is called from simulator if the next event is a departure
// Should update simulated queue statistics
// Should update current queue nodes and various queue member variables
void ProcessDeparture(struct Queue* elementQ, double departure_time,struct QueueNode* arrival) {
    // Update simulated statistics
    simulated_stats[0] += (current_time - last_event_time) * elementQ->waiting_count; // Increase simulated mean number of customers
    if (elementQ->waiting_count > 0) {
        simulated_stats[2] += (departure_time - elementQ->head->arrival_time); // Increase simulated mean waiting time
    }
    departure_count++; // Increment departure count

    // Update current queue nodes
    if (elementQ->head == NULL) {
        // If queue is empty, set both first and last to NULL
        elementQ->first = NULL;
        elementQ->last = NULL;
    } else {
        struct QueueNode* temp = elementQ->head;
        elementQ->head = elementQ->head->next;
        free(temp);
        elementQ->waiting_count--;
    }
    printf("Processed Departure \n");
    // Update last event time
    last_event_time = current_time;
}


// This is the main simulator function
// Should run until departure_count == total_departures
// Determines what the next event is based on current_time
// Calls appropriate function based on next event: ProcessArrival(), StartService(), ProcessDeparture()
// Advances current_time to next event
// Updates queue statistics if needed
// Print statistics if departure_count is a multiple of print_period
// Print statistics at end of simulation (departure_count == total_departures)

void Simulation(struct Queue* elementQ, EventList* event_list, double lambda, double mu, int print_period, int total_departures) {
    struct QueueNode* arrival = NULL;

    // Generate all arrival events
    double arrival_time = 0;
    for (int i = 0; i < total_departures; i++) {
        double interarrival_time = -log(drand48()) / lambda;
        arrival_time += interarrival_time;
        Event* arrival_event = create_event(arrival_time, ARRIVAL_EVENT);
        assert(arrival_event != NULL);
        printf("Scheduled arrival event at time: %lf\n", arrival_time);
        schedule_event(event_list, arrival_event);
    }

    while (departure_count < total_departures) {
        // Determine next event
        Event* next_event = get_next_event(event_list);
        if (next_event == NULL) {
            // If no more events, break the loop
            break;
        }
        
        // Advance current time to next event
        current_time = next_event->event_time;
        printf("Next event type: %d\n", next_event->event_type);
        // Call appropriate function based on the event type
        if (next_event->event_type == ARRIVAL_EVENT) {
            // Process the arrival
            printf("Process Arrival \n");
            arrival = malloc(sizeof(struct QueueNode));
            assert(arrival != NULL);
            arrival->arrival_time = next_event->arrival_time;
            arrival->service_time = -1;  // Service time will be determined later
            arrival->next = NULL;
            ProcessArrival(elementQ, arrival, event_list);
        }
        else if (next_event->event_type == START_SERVICE_EVENT) {
            // Process start of service
            printf("Processing start of service event\n");
            StartService(elementQ);
            // Schedule departure event
            if (elementQ->head != NULL) {
                printf("Starting Service \n");
                double service_time = elementQ->head->service_time; // Assuming service time is available
                double departure_time = current_time + service_time;
                Event* departure_event = create_event(departure_time, DEPARTURE_EVENT);
                assert(departure_event != NULL);
                schedule_event(event_list, departure_event);
            }
        }
        else if (next_event->event_type == DEPARTURE_EVENT) {
            // Process departure
            ProcessDeparture(elementQ, current_time, arrival);
            printf("Process Departure \n");
        }
        
        // Print statistics if departure count is a multiple of print_period
        if (departure_count % print_period == 0) {
            PrintStatistics(elementQ, total_departures, print_period, lambda);
        }
        
        // Free memory allocated for the processed event
        free(next_event);
    }
    
    // Print statistics at the end of simulation
    PrintStatistics(elementQ, total_departures, print_period, lambda);
}

// Free memory allocated for queue at the end of simulation
void FreeQueue(struct Queue* elementQ) {
    struct QueueNode* current = elementQ->head;
    struct QueueNode* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(elementQ);
}

// Main function
int main(void) {
    double lambda = 4;
    double mu = 5;
    int print_period = 1000;
    int total_departures = 2500;
    int random_seed = 534;

    GenerateComputedStatistics(lambda, mu);
    
    // Initialize event list
    EventList event_list;
    initialize_event_list(&event_list);


    printf("Simulating M/M/1 queue with lambda = %f, mu = %f, P = %d, D = %d, S = %d\n", lambda, mu, print_period, total_departures, random_seed);
    struct Queue* elementQ = InitializeQueue(random_seed, lambda, mu, total_departures);
    
    Simulation(elementQ, &event_list, lambda, mu, print_period, total_departures);
    FreeQueue(elementQ);
    free_event_list(&event_list);
   
    return 0;
}

/*// Program's main function
int main(int argc, char* argv[]){

    // input arguments lambda, mu, P, D, S
    if(argc >= 6){

        double lambda = atof(argv[1]);
        double mu = atof(argv[2]);
        int print_period = atoi(argv[3]);
        int total_departures = atoi(argv[4]);
        int random_seed = atoi(argv[5]);
    
   
        if (lambda <= 0 || mu <= 0 || mu <= lambda) {
             fprintf(stderr, "Invalid input arguments. Lambda and mu must be positive, and mu must be greater than lambda.\n");
             return 1;
         }

        // If no input errors, generate M/M/1 computed statistics based on formulas from class
       GenerateComputedStatistics(lambda, mu);

        // Start Simulation
        printf("Simulating M/M/1 queue with lambda = %f, mu = %f, P = %d, D = %d, S = %d\n", lambda, mu, print_period, total_departures, random_seed);
        struct Queue* elementQ = InitializeQueue(random_seed, lambda, mu, total_departures);
        Simulation(elementQ, lambda, mu, print_period, total_departures);
        FreeQueue(elementQ);
    }
    else {
        printf("Insufficient number of arguments provided!\n");
        return 1;
     }
   
    return 0;
}
*/
