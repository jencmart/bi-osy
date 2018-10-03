#include <iostream>
#include <vector>
#include <chrono>
#include <thread>                                                

using namespace std;

//-------------------------------------------
void threadFunc(int tid)                                         
{
  printf("Thread %d: Start\n", tid);                                           
  
  // Simulation of some work
  this_thread::sleep_for(chrono::seconds(5));

  printf("Thread %d: Stop\n", tid);                                            
}
 
//-------------------------------------------
int main ( int argc, char * argv [] )
{
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
    thread(threadFunc, i).detach();
  }

  // Simulation of some work                                 
  this_thread::sleep_for(chrono::seconds(10));                        // <--- #1

  printf("Main:     Stop\n");
  return 0;
}

