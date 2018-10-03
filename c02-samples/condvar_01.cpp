/********************************************************************************
  - Main thread creates 4 working threads (T0, T1, T2, T3).
    - Fist shift:   T0 and T1 should be working,  T2 and T3 should be sleeping.
    - Second shift: T0 and T1 should be sleeping, T2 and T3 should be working.
  - There is some problem with synchonization. Try to correct it.
*********************************************************************************/

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class CounterClass
{
private:
  int                  g_Cnt;
  mutex                g_Mtx1, g_Mtx2; 
  condition_variable   g_Cond;

public: 
  CounterClass()
  {
    g_Cnt = 0;
  }

  /*************************************/
  void firstShift()
  {
    int val;

    while ( 1 )
    {
      lock_guard<mutex> lg (g_Mtx1);
      if ( g_Cnt < 10000 ) g_Cnt++;
      val = g_Cnt;
      if ( val >= 10000 ) break;
    }

    unique_lock<mutex> ul (g_Mtx2);
    g_Cond.notify_all();
    g_Cond.wait(ul);
    cout << "Thread:  counter = " << g_Cnt << endl;
  }

  /*************************************/
  void secondShift()
  {
    int val;

    //this_thread::sleep_for(chrono::seconds(1));

    unique_lock<mutex> ul (g_Mtx2);
    g_Cond.wait(ul);
    ul.unlock();

    while ( 1 )
    {
      lock_guard<mutex> lg (g_Mtx1);;
      if ( g_Cnt < 50000 ) g_Cnt++;
      val = g_Cnt;
      if ( val >= 50000 ) break;
    }

    ul.lock();
    g_Cond.notify_all();
    cout << "Thread:  counter = " << g_Cnt << endl;
  }
};

/*************************************/
int main ( int argc, char * argv [] )
{
  CounterClass   counterObj;
  vector<thread> workers;

  for ( int i = 0; i < 4; i ++ )
    workers.push_back ( ( i < 2) ? 
      thread(&CounterClass::firstShift, &counterObj) : 
      thread(&CounterClass::secondShift, &counterObj) );
    
  for ( auto & w : workers )
    w.join (); 

  return 0;
} 
