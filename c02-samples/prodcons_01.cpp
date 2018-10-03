/********************************************************************************
  - Producer/consumer problem synchronized by condition variables.
  - There is some problem with program termination. Try to correct it.

    - If the number of producers and consumers is the same, 
      then the proram runs correctly, otherwise it will end by deadlock.
*********************************************************************************/

#include <iostream>
#include <vector>
#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

#define BUFFER_SIZE 5

using namespace std;

struct item_t
{
  int  id;
  int  count;
  int  value;
};

/*************************************/
class BufferClass
{
private:
  deque<item_t>        buff;
  mutex                mtx; 
  condition_variable   cv_full, cv_empty;

public: 
  BufferClass() { }
 
  /****************************/
  void insert(item_t item) 
  {
    unique_lock<mutex> ul (mtx);
    cv_full.wait(ul, [ this ] () { return ( buff.size() < BUFFER_SIZE ); } );
    buff.push_back(item);

    printf("Buffer: ");
    for (deque<item_t>::iterator it = buff.begin(); it!=buff.end(); ++it)
      printf("[%d, %d, %d] ", it->id, it->count, it->value);
    printf("\n");
    
    cv_empty.notify_one();
  }

  /****************************/
  item_t remove() 
  {
    item_t item;

    unique_lock<mutex> ul (mtx);
    cv_empty.wait(ul, [ this ] () { return ( ! buff.empty() ); } );
    item = buff.front();
    buff.pop_front();
   
    printf("buffer: ");
    for (deque<item_t>::iterator it = buff.begin(); it!=buff.end(); ++it)
      printf("[%d, %d, %d] ", it->id, it->count, it->value);
    printf("\n");

    cv_full.notify_one();
    return item;
  }
  
  /****************************/
  bool empty()
  {
    return buff.empty();
  }
};

/*************************************/
void producer(int tid, BufferClass &buff)
{
  int count = 0;

  for ( int i = 0; i < 10; i++ )
  {
    item_t *item  = new item_t { tid, count, rand() % 100 };
    count++;
    
    buff.insert(*item);

    printf("Producer %d:  item [%d,%d,%d] was inserted\n", tid, item->id, item->count, item->value);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  printf("Producer %d: end\n", tid);
}

/*************************************/
void consumer(int tid, BufferClass &buff )
{
  for ( int i = 0; i < 10; i++ ) 
  {
    item_t item = buff.remove();

    printf("Consumer %d:  item [%d,%d,%d] was removed\n", tid, item.id, item.count, item.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
  printf("Consumer %d: end\n", tid);
}

/*************************************/
int main ( int argc, char * argv [] )
{
  int            prod, cons;
  BufferClass    buffer;
  vector<thread> threads;

  // Check arguments
  if ( argc != 3 || sscanf ( argv[1], "%d", &prod ) != 1
                 || sscanf ( argv[2], "%d", &cons ) != 1
                 || prod < 1 || cons < 1 )
    {
      printf ( "Usage: %s <prod> <cons>\n", argv[0] );
      return ( 1 );
    }
  
  for ( int i = 0; i < prod; i ++ )
    threads.push_back ( thread ( producer, i, ref(buffer) ) );

  for ( int i = 0; i < cons; i ++ )
    threads.push_back ( thread ( consumer, i, ref(buffer) ) );

  for ( auto & t : threads )
    t.join (); 

  return 0;
} 
