#include <iostream>
#include <vector>
#include <chrono>
#include <thread>                                                // <--- #1

using namespace std;

//-------------------------------------------
void threadFunc(int tid)                                         // <--- #2
{
  printf("Thread %d: Start\n", tid);                             // <--- #3
  //cout << "Thread " << tid << ": Start" << endl;               // <--- #4
  
  // Simulation of some work
  this_thread::sleep_for(chrono::seconds(5));

  printf("Thread %d: Stop\n", tid);                              // <--- #5
  //cout << "Thread " << tid << ": Stop" << endl;                // <--- #6
}
 
//-------------------------------------------
int main ( int argc, char * argv [] )
{
  vector<thread>  threads;
  int             threadNum;

  // Check arguments
  if ( argc != 2 || sscanf(argv[1], "%d", &threadNum) != 1 )
  {
    printf("Usage: %s number_of_threads\n", argv[0]);
    return 1;
  }

  printf("Main:     Start\n");
  
  // Create threads
  for ( int i = 0; i < threadNum; i++ )
  {
    printf("Main:     Creating thread %d\n", i);
    threads.push_back( thread (threadFunc, i) );                 // <--- #7
  }

  // Wait for threads
  for ( int i = 0; i < threadNum; i++ )
    threads[i].join();                                           // <--- #8

  printf("Main:     Stop\n");
  return 0;
}

