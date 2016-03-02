#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <array>
#include <numeric>
#include <chrono>
#include <ctime>
#include <ratio>

using namespace std;

/*  Benchmark the execution time to sum a large array 
 *  for a single thread and for multiple threads
 *  The array is broken into multiple blocks for processing by each thread
 *  with their results summed for a result.
 *  Execution times are averaged over a number of runs
 *
 *  Summary: The improvement in execution time using single V multple threads 
 *   (8 in this case) was ~70% 
 *   
 *  g++ acc.cpp -o run -Wall -std=c++11 -lpthread -g
**/

typedef unsigned long const ulong_const;

void do_join(std::thread& t)
{
    t.join();
}

template<typename Iterator, typename T>
struct accumulate_block
{
	void operator()(Iterator first, Iterator last, T& result)
	{
		result=std::accumulate(first, last, result);
	}
};

template<typename Iterator, typename T>
T multi_accumulate(Iterator first, Iterator last, T init)
{
	ulong_const length = std::distance(first, last);        
		
        // checks local CPU for max no of threads possible 
	ulong_const hardware_threads = thread::hardware_concurrency();

        // if hardware_threads==0 use 2 
	ulong_const num_threads = hardware_threads != 0 ? hardware_threads : 2;
        //cout << "num_threads=" << num_threads << endl;
        
	ulong_const block_size = length / num_threads;
        //cout << "block_size=" << block_size << endl;
        
        // store results of each block
        vector<T> results(num_threads);

        // create num_threads-1 threads as we already have the main thread 
	vector<thread> threads(num_threads-1);

	Iterator block_start = first;
        // Launching the threads  
	for(unsigned long i=0; i < num_threads - 1; ++i)
	{
		Iterator block_end = block_start;

                // advance the block_end iterator block_size positions
		std::advance(block_end, block_size);

                // launch a new thread to accumulate the results for the block
		threads[i] = thread(accumulate_block<Iterator, T>(),
				block_start, block_end, std::ref(results[i]));
                //cout << "starting thread" << i << " with range(" << *block_start << ":" << block_end << ")" << endl;
 
                // reset for next pass
		block_start = block_end;
	}

        // process last block in MT
	accumulate_block<Iterator, T>()(
	        block_start, last, results[num_threads-1]);

        // wait for all threads to finish
        std::for_each(threads.begin(), threads.end(),do_join);

        // add up the results with a final call to std::accumulate
	return std::accumulate(results.begin(), results.end(), init);
}


int main() {
    int result = 0;          
    vector<int> largeDataSet(100000000,9999);
    double singletime = 0;     
    double multitime = 0;

    for(int i=0;i<10;i++){
      // time the calculation with a single thread 
      result = 0;
      std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
      result=accumulate(largeDataSet.begin(), largeDataSet.end(), result); 

      std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
      cout << "single thread:  Result=" << result << "(" << time_span.count() << ")" << endl;
      singletime+=time_span.count();
    
      // time the calculation with the 8 avaliable threads
      // well 8 on my machine 
      result = 0;
      std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
      result = multi_accumulate(largeDataSet.begin(), largeDataSet.end(), result);
      std::chrono::high_resolution_clock::time_point t4 = std::chrono::high_resolution_clock::now();
      time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3);
      cout << "multi threaded: Result=" << result << "(" << time_span.count() << ")" << endl;
      multitime+=time_span.count();
    }
    cout << "single threaded calculated averaged duration:" << singletime/8 << endl;
    cout << "multi threaded calculated averaged duration:" << multitime/8 << "(" << 1-((multitime/8)/(singletime/8)) <<"\% quicker)" << endl;
}
 



