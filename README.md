# hw3
In this programming assignment, you will implement some of the basic elements of an event-driven simulation of a system with a single queue. You will implement queues, servers, event routines, and periodically print statistics. You will also design and implement data structures and the routines to maintain them, and a main routine to coordinate the parts of the simulation. The details of the system are described below. You can reuse some of your code from HW2, but you need to write significant extra code for this homework.

To get started, please download these files to your local directory:

"Makefile" Download Makefile" is a simple makefile to compile your code and the test code
"hw3_solution.c Download hw3_solution.c" is a skeleton file that you need to fill in all the empty functions. You should submit this file when you are done. You can add any variables, data structures, or functions that you need for your assignment as long as it still compiles with the provided Makefile. 
 

System to Simulate:
The system consists of a single queue and one server. The system capacity (and therefore the maximum size of the queue) is infinite. Jobs arrive at the system and enter the queue. The job at the head of the queue receives service when the server becomes available and then leaves the system. Simulation will end when a certain number of departures D has occurred.

Queues:
You will implement an M/M/1 queue. The population is infinite and the queueing discipline is FCFS (First-come, First-served).  Interarrival time and service time distributions are exponentially distributed. You will need to write a function to generate values for arrival times and service times per server based on the exponential distribution (as discussed in class on Jan 23).  

Simulation:
Setup: You need to generate D elements and insert them into an "element" queue. Each element has two properties:

Arrival time (based on exponential inter-arrival time distribution, added to the previous element’s arrival time)
Service time (based on exponential service time distribution)
Arrivals are generated based on the mean interarrival time 1/. Service time is based on a service time distribution with mean service time 1/µ. You need to use a seed for your random number generator so you don’t get the same result from the simulation every time. You should initialize your random number generator to the seed using srand(), then generate all arrival times, then go back through the queue to generate all service times.

Once you’ve established all arrival and service times, you need to run a simulation experiment where you process each element in the queue and determine the next event:

Arrival, wait in queue (if server is busy)
Arrival, start service (if server is free)
End service for current customer being served (departure), start service for next in line (if queue has at least one customer)
End service for current customer being served (departure), and wait for next arrival (if queue has zero customers) 
During the simulation, you should keep track of current time, and advance it to the time of the next event (arrival or departure). You should keep track of elements in the queue at current time, so you can have two pointers that point to "first element in queue at current time" and "last element in queue at current time". The next event would be either the departure of the first element or an arrival for the element beyond the current last element. A corner case will be if a departure and an arrival happen at the same time, in which case you should process the departure first then the arrival before advancing current time. 

Event Routines:
You need to implement a separate routine for each event (arrival, start service, end service/departure). So you need to write a function to model queue changes when each of the events above occurs. Your simulator should determine the next event then call the correct event routine.  Do not start on this part of the homework yet. We will discuss event routines in class on Jan 30. 

Statistics:
You need to generate a report with important performance metrics:  E[n] (average number in system), E[r] (average response time), E[w] (average waiting time), and p0 (probability of having zero customers). Statistics should be printed periodically every P departures,  and also at the end of the simulation. You need to compute these statistics during runtime, then compare them to the M/M/1 formulas discussed in class (Jan 30) to get these parameters.

Your simulation should correctly handle any choice of input parameters , µ, P and D. All these parameters should be arguments to call the program.

Submission instructions:
Submit a single file “hw3_solution.c” containing the source code of your simulation program (including all the data structure definitions and appropriate comments). Your program should compile using the attached Makefile Download Makefile. Your executable should run using the following command line parameter format

./hw3_solution  µ P D S

The arrival and service rates are positive floating point numbers (use "double" data type). The remaining parameters P, D and S are all positive integers.   For example:

./hw3_solution 2 5 1000 3500 534
Should simulate an M/M/1 queue with arrival rate=2, service rate=5, should print statistics periodically every 1000 departures, and should finish after 3,500 departures. You should use the random seed 534 to generate inter-arrival and service times (all arrival times first, then all service times).

Note about input parameters. Arrival rate represents "mean arrivals per second" so the mean interarrival time in seconds is 1/lambda. The same applies to service rate and mean service time. 

Output Format: 
Your program output should have this format:

Simulating M/M/1 queue with lambda = 2.000000, mu = 5.000000, P = 1000, D = 3500, S = 534

After  1000 departures
Mean n = xxxx (Simulated) and yyyy (Computed)
Mean r = xxxx (Simulated) and yyyy (Computed)
Mean w = xxxx (Simulated) and yyyy (Computed)
p0 = xxxx (Simulated) and yyyy (Computed)

After  2000 departures
Mean n = xxxx (Simulated) and yyyy (Computed)
Mean r = xxxx (Simulated) and yyyy (Computed)
Mean w = xxxx (Simulated) and yyyy (Computed)
p0 = xxxx (Simulated) and yyyy (Computed)

After  3000 departures
Mean n = xxxx (Simulated) and yyyy (Computed)
Mean r = xxxx (Simulated) and yyyy (Computed)
Mean w = xxxx (Simulated) and yyyy (Computed)
p0 = xxxx (Simulated) and yyyy (Computed)

End of Simulation - after  3500 departures
Mean n = xxxx (Simulated) and yyyy (Computed)
Mean r = xxxx (Simulated) and yyyy (Computed)
Mean w = xxxx (Simulated) and yyyy (Computed)
p0 = xxxx (Simulated) and yyyy (Computed)
Sample Output
 ./hw3_solution 4 5 1000 2500 534
Simulating M/M/1 queue with lambda = 4.000000, mu = 5.000000, P = 1000, D = 2500, S = 534

After 1000 departures
Mean n = 3.4761 (Simulated) and 4.0000 (Computed)
Mean r = 0.8797 (Simulated) and 1.0000 (Computed)
Mean w = 0.6786 (Simulated) and 0.8000 (Computed)
p0 = 0.2058 (Simulated) and 0.2000 (Computed)

After 2000 departures
Mean n = 3.5639 (Simulated) and 4.0000 (Computed)
Mean r = 0.8961 (Simulated) and 1.0000 (Computed)
Mean w = 0.6921 (Simulated) and 0.8000 (Computed)
p0 = 0.1901 (Simulated) and 0.2000 (Computed)

End of Simulation - after 2500 departures
Mean n = 3.4499 (Simulated) and 4.0000 (Computed)
Mean r = 0.8699 (Simulated) and 1.0000 (Computed)
Mean w = 0.6687 (Simulated) and 0.8000 (Computed)
p0 = 0.2022 (Simulated) and 0.2000 (Computed)

Important Notes:
Your program should be able to handle a large number of arrivals, at least up to 100 million elements. Programs should run efficiently even when the number of elements is high. We will test your programs with millions of elements. 
Your simulated and computed results can diverge for a small number of elements. But the results should be very close with a large number of elements (e.g., more than a million elements)
Your simulated results are highly dependent on the random seed for a small number of elements. But as the number of elements grows, you should be getting results close to the computed results regardless of the random seed. 
