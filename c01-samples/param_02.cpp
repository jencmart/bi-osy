#include <iostream>
#include <vector>
#include <chrono>
#include <thread>                                                

using namespace std;

//-------------------------------------------
void threadFunc(int &value, vector<int> &values)                                         
{
  printf("Thread:   Start\n");                                            

  printf("Thread:   value = %d     values[1] = %d     &value = %p   values.data = %p\n", value, values[1], (void *) & value, (void *) values.data());

  // Modify value
  printf("Thread:   modifying values\n");
  value      = value + 100;
  values[1]  = values[1] + 100;
  printf("Thread:   value = %d   values[1] = %d    &value = %p   values.data = %p\n", value, values[1], (void *) & value, (void *) values.data());
  for ( int i = 0; i < 10; i ++ )
    values . push_back ( i );
  printf("Thread:   value = %d   values[1] = %d    &value = %p   values.data = %p\n", value, values[1], (void *) & value, (void *) values.data());

  
  printf("Thread:   Stop\n");                                         
}

//-------------------------------------------
int main ( int argc, char * argv [] )
{
  int            value       = 1;
  vector<int>    values      = {10, 20, 30};

  printf("Main:     Start\n");
  printf("Main:     value = %d     values[1] = %d     &value = %p   values.data = %p\n", value, values[1], (void *) & value, (void *) values.data());
  
  // Create thread and wait for completion
  printf("Main:     Creating thread\n");
  thread(threadFunc, ref(value), ref(values)).join();
  
  printf("Main:     value = %d   values[1] = %d    &value = %p   values.data = %p\n", value, values[1], (void *) & value, (void *) values.data());

  printf("Main:     Stop\n");
  return 0;
}

