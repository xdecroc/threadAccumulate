# Benchmark the execution time to sum a large array 
for a single thread and for multiple threads
The array is broken into multiple blocks for processing by each thread
with their results summed for a result.
Execution times are averaged over a number of runs

Summary: The improvement in execution time using single V multple threads 
(8 in this case) was ~70% 
   
Usage:  g++ acc.cpp -o run -Wall -std=c++11 -lpthread -g

