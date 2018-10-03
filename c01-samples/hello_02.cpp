#include <iostream>
#include <vector> 
#include <chrono>
#include <thread>                                                          

using namespace std;

//-------------------------------------------
class ThreadClass                                                          // <--- #1
{
public: 
  void Solve(int tid)
  {
    printf("Thread %d: Start\n", tid);
  
    // Simulation of some work
    this_thread::sleep_for(chrono::seconds(5));

    printf("Thread %d: Stop\n", tid);
  }
};
 
//-------------------------------------------
int main ( int argc, char * argv [] )
{
  ThreadClass     threadObj;    
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
    threads.push_back( thread(&ThreadClass::Solve, &threadObj, i) );       // <--- #2
  }

  // Wait for threads
  for ( int i = 0; i < threadNum; i++ )
    threads[i].join();                                                  

  printf("Main:     Stop\n");
  return 0;
}

