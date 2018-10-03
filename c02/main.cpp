#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <zconf.h>
#include <mutex>
#include <condition_variable>

using namespace std;



// lock guard
// unique lock
// oboje vrappery nad mutextem, resi automaticke odemknuti pri zaniku



//podminena promenna je rychlejsi nez semafory, ale take toho umi min
// podminena promenna vzdy s cyklem while a ne pouze if

//posix pthread_cond _wait _signal _broadcast

//C++
// wait() notify_one() notify_all()


// broadcast nemusi byt obaleny lockem
// wait musi

// 1. SEMESTRALKA - PROBLEM PRODUCENT KONZUMENT
// problem producent konzument
// producent (vice prodicentu) ma buffer ; ma limitovanou velikost. kdyz naplni buffer tak musime je zastavit
// konzumenti nacitaji z bufferu; pokud fronta prazdna, musime zastavit konzumenty
// fronta je sdileny prostredek
// funkce ptr na data ; kdyz ptr na null tak data dosly. nevime kolik dat dostaneme....
// dokud existuji producenti a dokud neni fronta prazdna, tak je cas na konec....
// DOMACI UKOL



condition_variable g_cont;

mutex m;

void threadFunc(int n, int  &shared)
{


    for (int i = 1; i <= n; ++i)
    {
        lock_guard<std::mutex> lock(m); // pokud uz je zamceno, tak zablokuje vlakno ktere chce lockovat
        shared += i;
    }
}

void threadFunc2(int n, int  &shared)
{

    for (int i = 1; i <= n; ++i)
    {
        lock_guard<std::mutex> lock(m); // pokud uz je zamceno, tak zablokuje vlakno ktere chce lockovat
        shared += i;
    }
}



int main ( int argc, char * argv [] ) {

    int n1, n2;
    int shared = 0;

    scanf("%d%d", &n1, &n2);

    vector<thread> threads;

    threads.emplace_back(thread(threadFunc, n1, ref(shared)));
    threads.emplace_back(thread(threadFunc, n2, ref(shared)));

    threads.emplace_back(thread(threadFunc, n2, ref(shared)));
    threads.emplace_back(thread(threadFunc, n2, ref(shared)));

    for (auto &thread : threads)
        thread.join();

    printf("Jarda secetly: %d\n", shared);

    int one = 0;
    for (int i = 1; i <= n1; ++i)
        one += i;
    for (int i = 1; i <= n2; ++i)
        one += i;

    printf("Bez vlaken:    %d\n", one);

    if(shared == one)
        printf("Soucet stejny\n");
    else
        printf("Soucet JINY!\n");
    return 0;
}