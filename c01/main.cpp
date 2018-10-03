#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <zconf.h>

using namespace std;

//-------------------------------------------
double calculate(int from, int to)
{
    double result = 0;

    if(from == 0)
    {
        result = 1;
        from = 1;
    }

    for (int i = from; i < to ; ++i)
        result += (sqrt(i+1) + i) / (sqrt(i) * sqrt(i + 1 + 1/i));

    return result;
}

//-------------------------------------------
void threadFunc(int i, int from, int to, vector<double> &values)
{
    values[i] = calculate(from, to);
}

//-------------------------------------------
int main ( int argc, char * argv [] ) {
    int m = 0, threadNum = 0;

    printf("insert m {1,100.000.000}, t{1,1024}: ");
    scanf("%d%d", &m, &threadNum);
    if (m < threadNum) threadNum = m;

    while (m < 1 || m > 100000000 || threadNum < 1 || threadNum > 1024) {
        printf("\ninsert m {1,100.000.000}, t{1,1024}: ");
        scanf("%d%d", &m, &threadNum);
        if (m < threadNum) threadNum = m;
    }
    printf("using %d threads\n\n", threadNum);


    ////------------------------------------------- compute in one thread
    auto start = std::chrono::high_resolution_clock::now();
    double result = calculate(0, m);
    auto finish = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    std::cout << "t(m,1) = " << result << " ("<< time1 << " ms" <<")" << std::endl;

    ////------------------------------------------- Create threads and wait for completion
    start = std::chrono::high_resolution_clock::now();

    vector<double> values(threadNum, 0);
    vector<thread> threads;

    int cnt = m / threadNum;

    for (int i = 0; i < threadNum - 1; ++i)
        threads.emplace_back(thread(threadFunc, i, i * cnt, i * cnt + cnt, ref(values)));

    if (m % threadNum == 0)
        threads.emplace_back(thread(threadFunc, threadNum-1, (threadNum-1 )* cnt, (threadNum-1 ) * cnt + cnt, ref(values)));
    else
        threads.emplace_back(thread(threadFunc, threadNum-1, (threadNum-1 )* cnt, (threadNum-1 ) * cnt + cnt + (m % threadNum), ref(values)));

    for (int j = 0; j < threadNum; ++j)
        threads[j].join();

    result = 0;

    for (int k = 0; k < threadNum; ++k)
        result += values[k];

    finish = std::chrono::high_resolution_clock::now();
    auto time2 =  std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count();
    std::cout << "t(m,"<< threadNum <<") = "<< result << " ("<< time2 << " ms" <<")" << std::endl << std::endl;
    ////-------------------------------------------

    std::cout << "S(m, "<< threadNum <<"): " << ((double)time1)/time2;
    return 0;


    /**
     * n =   1 (1.02045)
     * n =   2 (2.07341)
     * n =   4 (3.93871)
     * n =   8 (3.77565)
     * n =  16 (3.79369)
     * n =  32 (3.81981)
     * n =  64 (3.76712)
     * n = 128 (3.75417)
     */
}
