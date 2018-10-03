#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <queue>
#include <stack>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>
#include <cstdint>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <atomic>

using namespace std;


class CFITCoin;

class CCVUTCoin;

class CCustomer;

typedef struct shared_ptr<CFITCoin> AFITCoin;
typedef struct shared_ptr<CCVUTCoin> ACVUTCoin;
typedef struct shared_ptr<CCustomer> ACustomer;

//=================================================================================================
class CFITCoin {
public:
    CFITCoin(const vector<uint32_t> &vectors, int distMax) :
            m_Vectors(vectors),
            m_DistMax(distMax),
            m_Count(0) {
    }

    virtual                  ~CFITCoin(void) = default;

    vector<uint32_t> m_Vectors;
    int m_DistMax;
    uint64_t m_Count; //je výsledkem výpočtu - počet 32 bitových hodnot x takových, že jejich vzdálenost od zadaných vektorů je nejvýše m_DistMax. Tuto hodnotu má vyplnit pracovní vlákno.
};

//=================================================================================================
class CCVUTCoin {
public:
    CCVUTCoin(const vector<uint8_t> &data, int distMin, int distMax) :
            m_Data(data),
            m_DistMin(distMin),
            m_DistMax(distMax),
            m_Count(0) {}

    virtual ~CCVUTCoin(void) = default;

    vector<uint8_t> m_Data;
    int m_DistMin;
    int m_DistMax;
    uint64_t m_Count; // je výsledek výpočtu, který má vyplnit pracovní vlákno. Hodnota má udávat počet dvojic (předpona, přípona) zadané bitové posloupnosti takových, že jejich editační vzdálenost patří do intervalu hodnot < m_DistMin ; m_DistMax >.
};

//=================================================================================================
class CCustomer {
public:
    virtual ~CCustomer(void) = default;

    virtual AFITCoin FITCoinGen(void) = 0;

    virtual ACVUTCoin CVUTCoinGen(void) = 0;

    virtual void FITCoinAccept(AFITCoin x) = 0;

    virtual void CVUTCoinAccept(ACVUTCoin x) = 0;

};
//=================================================================================================
#endif /* __PROGTEST__ */

#define CHECK_BIT(var, pos) ( (bool) ( ( (var) & (1<<(pos)) ) >> (pos) ) )

uint32_t Min2(uint32_t x, uint32_t y) { return x < y ? x : y; }

uint32_t Min(uint32_t x, uint32_t y, uint32_t z) { return Min2(Min2(x, y), z); }

unsigned g_numOfThreads;

bool g_zavolanStop;      // NEPRIJIMEJ NOVE ZAKAZNIKY
bool g_vseRecieved;      // WORK THREADS SE UZ NEUSPI
bool g_vypoctyDobehly;   // DELIVER THREADS SE UZ NEUSPI

std::condition_variable cond_problemsBuff;
std::condition_variable cond_solvedBuff;
std::condition_variable cond_userBuff;


std::mutex g_vseRecievedMutex;
std::condition_variable cond_allBigProblems;
uint64_t g_bigProblemsCnt;


/// ********************************************************************************************************************
/// **************************************   <PROBLEM>   ***************************************************************
/// ********************************************************************************************************************
class Problem {
public:
    ACustomer m_customer;

    explicit Problem(ACustomer customer) : m_customer(std::move(customer)) {}

    Problem() : m_customer(nullptr) {}

    virtual ~Problem() = default;

    virtual bool Solve() = 0;

    virtual bool Solve(uint32_t from, uint32_t to) = 0;

    virtual void Deliver() = 0;

    virtual bool Partial() { return false; }

    virtual vector<Problem *> GetPartialProblems() { return std::vector<Problem *>(); }
};

class PartialFITProblem : public Problem {
public:
    PartialFITProblem(uint32_t from, uint32_t to, Problem *source) : Problem(source->m_customer), m_from(from), m_to(to), m_fitProblemSource(source) {}

    bool Solve() override {
        return m_fitProblemSource->Solve(m_from, m_to);
    }

    bool Solve(uint32_t, uint32_t) override { return true;}

    bool Partial() override { return true; }

    void Deliver() override {
        m_fitProblemSource->Deliver();
        delete m_fitProblemSource;
    }

private:
    uint32_t m_from;
    uint32_t m_to;
    Problem *m_fitProblemSource;
};

class PartialCVUTProblem : public Problem {
public:
    PartialCVUTProblem(uint32_t from, uint32_t to, Problem *source) : Problem(source->m_customer), m_from(from), m_to(to), m_cvutProblemSource(source) {}

    bool Solve() override {
        return m_cvutProblemSource->Solve(m_from, m_to); }

    bool Solve(uint32_t, uint32_t) override {return true;}

    bool Partial() override { return true; }

    void Deliver() override {
        m_cvutProblemSource->Deliver();
        delete m_cvutProblemSource;
    }

private:
    uint32_t m_from;
    uint32_t m_to;
    Problem *m_cvutProblemSource;
};

int dist1;
int dist2;
int dist3;
int dist4;

/// ********************************************************************************************************************
/// ***********************************  FIT PROBLEM  ******************************************************************
/// ********************************************************************************************************************
class FITProblem : public Problem {
public:
    explicit FITProblem(ACustomer customer, shared_ptr<CFITCoin> x) : Problem(std::move(customer)), m_hotovo(false), m_FITCoin(std::move(x)) { Init(); }

    explicit FITProblem(shared_ptr<CFITCoin> x) : Problem(), m_hotovo(false), m_FITCoin(std::move(x)) { Init(); }

    ~FITProblem() override = default;

    bool Solve() override {
        if(m_hotovo)
            return true;

        return Solve(0, m_value);
    }

    bool Solve(uint32_t from, uint32_t to) override {
        uint64_t resultat = GetRealNumOfDifferences(from, to);

        {
            std::unique_lock<std::mutex> locker(m_numberMutex);
            m_FITCoin->m_Count += resultat;
            return (!(--m_cntPartialProblems));
        }
    }

    vector<Problem *> GetPartialProblems() override {
        vector<Problem *> podproblems;
        m_cntPartialProblems = 0;

        if (m_hotovo) {
            m_cntPartialProblems++;
            podproblems.push_back(new PartialFITProblem(1, 0, this));
            return podproblems;
        }

        uint32_t naJedenThread = m_value / g_numOfThreads;
        if (naJedenThread < 20000)
            naJedenThread = 20000;

        uint32_t from = 0;
        while (true) {
            if (from + 2 * naJedenThread > m_value) {
                podproblems.push_back(new PartialFITProblem(from, m_value, this));
                m_cntPartialProblems++;
                break;
            }

            m_cntPartialProblems++;
            podproblems.push_back(new PartialFITProblem(from, from + naJedenThread, this));
            from += naJedenThread + 1;
        }

        return podproblems;
    }

    inline void Deliver() override { m_customer->FITCoinAccept(m_FITCoin); }

private:

    void Init() {

        if (m_FITCoin->m_DistMax == 0) {
            m_FITCoin->m_Count = 1;
            m_hotovo = true;
        }

        if (m_FITCoin->m_Vectors.size() == 1) {
            m_FITCoin->m_Count = Combinatory(32, 0, m_FITCoin->m_DistMax);
            m_hotovo = true;
        }

        m_cntRedundant = RemoveRedundant(m_FITCoin->m_Vectors);

        m_value = (uint32_t) (((uint64_t) 1 << (32 - m_cntRedundant)) - 1);

        m_maxVal = 32 - m_cntRedundant;
    }

    uint64_t Combinatory(unsigned cntRedundant, unsigned numOfDifferences, int dist) {
        uint64_t result = 0;
        for (unsigned i = 0; i <= dist - numOfDifferences; ++i)
            result += nChoosek(cntRedundant, i);

        return result;
    }

    uint32_t RemoveRedundant(vector<uint32_t> &vectors) {
        if (vectors.size() == 1) return 0;
        uint32_t samecnt = 0;

        // traverse over all bits
        for (int i = 0; i < 32; ++i) {
            if (!AllBitsSame(vectors, i))continue;

            ++samecnt;
            DeleteBit(vectors, i);
        }

        for (unsigned int &vector : vectors)
            vector >>= samecnt;

        return samecnt;
    }

    uint64_t nChoosek(unsigned int n, unsigned int k) {
        if (k > n) return 0;
        if (k * 2 > n) k = n - k;
        if (k == 0) return 1;

        uint64_t result = n;
        for (unsigned int i = 2; i <= k; ++i) {
            result *= (n - i + 1);
            result /= i;
        }
        return result;
    }

    bool AllBitsSame(vector<uint32_t> &vectors, int nThBit) {
        for (size_t i = 0; i < vectors.size(); ++i)
            if (CHECK_BIT(vectors[i], nThBit) != CHECK_BIT(vectors[0], nThBit))
                return false;

        return true;
    }

    void DeleteBit(vector<uint32_t> &vectors, int nThBit) {
        for (unsigned int &vector : vectors) {
            // m1 = (1 << nThBit) -1
            int tmpShifted = (vector & ((1 << nThBit) - 1)) << 1;
            // m2 ~ = (1 << nThBit+1 ) -1
            vector = vector & (~((uint32_t) (((uint64_t) 1 << (nThBit + 1)) - 1)));
            vector |= tmpShifted;
        }
    }

    uint64_t GetRealNumOfDifferences(uint32_t from, uint32_t to) {
        uint64_t resultus = 0;

        while (true) {
            uint32_t diff = 0;
            uint32_t number;

            { /// zvetsujeme m_number - musime locknout
                std::unique_lock<std::mutex> locker(m_numberMutex);
                if (from > to)
                    break;
                else
                    number = from++;
            }

            // go throught all vectors
            for (auto vector : m_FITCoin->m_Vectors) {
                uint32_t currDiff = 0;
                // and compare bits
                for (unsigned i = 0; i < m_maxVal; ++i) // TODO XOR
                    if (CHECK_BIT(number, i) != CHECK_BIT(vector, i))
                        currDiff++;

                diff = currDiff > diff ? currDiff : diff;
            }

            if(diff == 1)
                dist1++;
            if(diff == 2)
                dist2++;
            if(diff == 3)
                dist3++;
            if(diff == 4)
                dist4++;

            uint64_t realDiff = 0;

            if (diff > (unsigned) m_FITCoin->m_DistMax)
                realDiff = 0;

            else if (diff == (unsigned) m_FITCoin->m_DistMax)
                realDiff = 1;

            else if (diff < (unsigned) m_FITCoin->m_DistMax && m_cntRedundant == 0)
                realDiff = 1;

            else
                realDiff = Combinatory(m_cntRedundant, diff, m_FITCoin->m_DistMax);

            resultus += realDiff;
        }

        return resultus;
    }

    bool m_hotovo;
    shared_ptr<CFITCoin> m_FITCoin;
    unsigned m_cntRedundant;
    std::mutex m_numberMutex;
    uint32_t m_value;
    uint32_t m_maxVal;
    uint32_t m_cntPartialProblems;
};


/// ********************************************************************************************************************
/// ***********************************  CVUT PROBLEM  *****************************************************************
/// ********************************************************************************************************************
class CVUTProblem : public Problem {
public:
    explicit CVUTProblem(ACustomer customer, shared_ptr<CCVUTCoin> x) : Problem(std::move(customer)), m_CVUTCoin(std::move(x)) { Init(); }

    explicit CVUTProblem(shared_ptr<CCVUTCoin> x) : Problem(), m_CVUTCoin(std::move(x)) { Init(); }

    ~CVUTProblem() override = default;

    void Init() {
        dataBegin = new bool[8 * m_CVUTCoin->m_Data.size()];
        dataSize = 8 * m_CVUTCoin->m_Data.size();
        auto *ptr = dataBegin;

        /// vyrob string
        for (auto byte : m_CVUTCoin->m_Data)
            for (unsigned i = 0; i < 8; ++i)
                *(ptr++) = CHECK_BIT(byte, i);

        str2 = (dataBegin + dataSize);
        str1 = (unsigned) dataSize;
    }

    vector<Problem *> GetPartialProblems() override {
        vector<Problem *> podproblems;
        m_cntPartialProblems = 0;


        /// vzdy to je jakoby dataSize * (from + (from+1) ... + to)
        /// takze bez ohledu na dataSize musim udelat nejak tak rovnomerne rozlozeni (from + (from+1) ... + to)


        uint32_t naJedenThread = (uint32_t) dataSize / g_numOfThreads;
        if (naJedenThread < 2000)
            naJedenThread = 2000;

        uint64_t ciselCelkem =  ( dataSize / 2)  * (1+dataSize);
        uint64_t S = ciselCelkem / g_numOfThreads;
        uint64_t S2 = S*2;

        uint32_t from = 1;

       // cout <<"dataSize" << dataSize << " cisel celkem "<< ciselCelkem << " S " << S << endl;
        while(++m_cntPartialProblems)
        {
            uint64_t a = S2 - from + (from*from);
            uint64_t res = (uint64_t)(sqrt(4*a + 1.)-1) / 2;
            if(res <= from)
                res = from +1;


            if(res > dataSize)
            {
        //        cout << "from " << from << " to " << dataSize << endl;
                podproblems.push_back(new PartialCVUTProblem(from, (uint32_t) dataSize, this));
                break;
            }

          //  cout << "from " << from << " to " << res << endl;
            podproblems.push_back(new PartialCVUTProblem(from, (uint32_t)res, this));
            from = (uint32_t)res + 1;
        }
/*
        while (++m_cntPartialProblems) {
            if (from + 2 * naJedenThread > dataSize)
            {
                podproblems.push_back(new PartialCVUTProblem(from, (uint32_t) dataSize, this));
                break;
            }

            podproblems.push_back(new PartialCVUTProblem(from, from + naJedenThread, this));
            from += naJedenThread + 1;
        }

*/
        return podproblems;
    }

    bool Solve() override { return Solve(1, (uint32_t)dataSize); }

    bool Solve(uint32_t from, uint32_t to) override {

        uint64_t result = 0;

        for (; from <= to; from++)
            result += threadCalcDst(dataBegin, str2, str1, from, m_CVUTCoin->m_DistMax, m_CVUTCoin->m_DistMin);

        {
            std::unique_lock<std::mutex> locker(m_increment);
            m_CVUTCoin->m_Count += result;
            if (!(--m_cntPartialProblems)) {
                delete[]dataBegin;
                return true;
            }
            return false;
        }
    }

    void Deliver() override { m_customer->CVUTCoinAccept(m_CVUTCoin); }

    unsigned str1;
    bool *str2;
    size_t dataSize;
    bool *dataBegin;
    uint32_t m_cntPartialProblems;

private:
    uint64_t
    threadCalcDst(const bool *str1, const bool *str2, unsigned str1Len, unsigned str2Len, int dMax, int dMin) const {

        str2 -= str2Len;

        auto **table = new uint32_t *[str1Len + 1];
        for (unsigned k = 0; k < str1Len + 1; ++k) {
            table[k] = new uint32_t[str2Len + 1];
        }

        for (uint32_t i = 0; i <= str1Len; ++i)
            table[i][0] = i;

        for (uint32_t j = 0; j <= str2Len; ++j)
            table[0][j] = j;

        for (uint32_t i = 1; i <= str1Len; i++)
            for (unsigned j = 1; j <= str2Len; j++)

                if (str1[i - 1] == str2[j - 1])
                    table[i][j] = table[i - 1][j - 1];
                else {
                    table[i][j] = Min(table[i][j - 1], table[i - 1][j], table[i - 1][j - 1]);
                    table[i][j]++;
                }

        uint64_t result = 0;

        for (unsigned i = 1; i <= str1Len; ++i)
            if (table[i][str2Len] <= (unsigned) dMax && table[i][str2Len] >= (unsigned) dMin)
                result++;

        for (unsigned k = 0; k < str1Len + 1; ++k)
            delete[] table[k];

        delete[] table;

        return result;
    }

    shared_ptr<CCVUTCoin> m_CVUTCoin;
    std::mutex m_increment;
};

///*********************************************************************************************************************
/// *************************************** BUFFERS ********************************************************************
///*********************************************************************************************************************

class Queue {
public:
    void insert(Problem *p) {
        while (true) {
            {
                std::unique_lock<std::mutex> locker(mu);
                cond_problemsBuff.wait(locker, [this]() { return m_buffer.size() < m_size; }); //spi dokud je plno
                m_buffer.push_front(p);
            }

            cond_problemsBuff.notify_all();
            return;
        }
    }

    void insertAll(const vector<Problem *> &problems) {
        std::unique_lock<std::mutex> locker(mu);
        for (auto problem : problems)
            m_buffer.push_front(problem);
        cond_problemsBuff.notify_all();
    }

    Problem *remove() { /// VRACI NULL PTR PRAVE TEHDY KDYZ UZ NEJSOU ZADNA DATA A NIKDY NEBUDOU
        while (true) {
            Problem *p;
            {
                std::unique_lock<std::mutex> locker(mu);
                cond_problemsBuff.wait(locker, [this]() { return !m_buffer.empty() || g_vseRecieved; }); //spi dokud je prazdno
                p = removeIt();
            }
            cond_problemsBuff.notify_all();
            return p;
        }
    }

    Queue() = default;

private:
    std::mutex mu;

    uint32_t m_size = 1000;

    Problem *removeIt() {
        Problem *p = nullptr;
        if ( ! m_buffer.empty()) {
            p = m_buffer.back();
            m_buffer.pop_back();
        }
        return p;
    }

    deque<Problem *> m_buffer;
};

class QueueSOLVED {
public:
    void insert(Problem *p) {
        while (true) {
            {
                std::unique_lock<std::mutex> locker(mu);
                cond_solvedBuff.wait(locker, [this]() { return m_buffer.size() < m_size; }); //spi dokud je plno
                m_buffer.push_front(p);
            }
            cond_solvedBuff.notify_all();
            return;
        }
    }

    Problem *tryGetProblem(const ACustomer &customer) {
        while (true) {
            Problem *p;
            {
                std::unique_lock<std::mutex> locker(mu);
                cond_solvedBuff.wait(locker, [this]() { return !m_buffer.empty() || g_vypoctyDobehly; }); //spi dokud je prazdno
                p = tryGet(customer);

            }
            cond_solvedBuff.notify_all();
            return p;
        }
    }

    QueueSOLVED() = default;

private:
    std::mutex mu;

    Problem *tryGet(const ACustomer &customer) {
        for (size_t i = 0; i < m_buffer.size(); ++i) {
            if (m_buffer[i]->m_customer == customer) {
                Problem *tmp = m_buffer[i];
                m_buffer[i] = m_buffer.back();
                m_buffer.pop_back();
                return tmp;
            }
        }
        return nullptr;
    }

    deque<Problem *> m_buffer;
    const unsigned int m_size = 100;
};

class User {
public:
    int m_count;
    bool m_FITFinished;
    bool m_CVUTFinished;
    ACustomer m_customer;

    explicit User(ACustomer customer) : m_count(0), m_FITFinished(false), m_CVUTFinished(false),
                                        m_customer(std::move(customer)) {}

};

class QueueUsers {
public:
    void insert(User *p) {
        while (true) {
            {
                std::unique_lock<std::mutex> locker(mu);
                cond_userBuff.wait(locker, [this]() { return m_buffer.size() < m_size; }); //spi dokud je plno
                m_buffer.push_front(p);
            }
            cond_userBuff.notify_all();
            return;
        }
    }

    User *remove(const ACustomer &customer) { /// VRACI NULL PTR PRAVE TEHDY KDYZ UZ NEJSOU ZADNA DATA A NIKDY NEBUDOU
        while (true) {
            User *u;
            {
                std::unique_lock<std::mutex> locker(mu);

                if (!g_zavolanStop) { // jeste neni stop  a proto muzes jit spat
                    cond_userBuff.wait(locker, [this]() {
                        return !m_buffer.empty() || g_zavolanStop;
                    }); //spi dokud je prazdno nebo do te doby kdy je zavolano stoo
                    u = removeCustomer(customer);
                } else // stop a dalsi customers uz nebudou
                    u = removeCustomer(customer);
            }
            cond_userBuff.notify_all();
            return u;
        }
    }

    User *getUser(const ACustomer &customer) {
        for (auto &i : m_buffer)
            if (i->m_customer == customer)
                return i;

        return nullptr;
    }

    void setFITFinished(const ACustomer &customer) {
        getUser(customer)->m_FITFinished = true;
    }

    void setCVUTFinished(const ACustomer &customer) {
        getUser(customer)->m_CVUTFinished = true;
    }

    void incrementProblems(const ACustomer &customer) {
        std::unique_lock<std::mutex> locker(mu_increment);
        getUser(customer)->m_count++;
    }

    void decrementProblems(const ACustomer &customer) {
        std::unique_lock<std::mutex> locker(mu_increment);
        getUser(customer)->m_count--;
    }

    QueueUsers() = default;

private:
    std::mutex mu_increment;
    std::mutex mu;
    deque<User *> m_buffer;
    const unsigned int m_size = 10;

    User *removeCustomer(const ACustomer &customer) // TODO
    {
        for (size_t i = 0; i < m_buffer.size(); ++i) {
            if (m_buffer[i]->m_customer == customer) {
                User *tmp = m_buffer[i];
                m_buffer[i] = m_buffer.back();
                m_buffer.pop_back();
                return tmp;
            }
        }

        return nullptr;
    }
};


///*********************************************************************************************************************
///*********************************************************************************************************************
///*********************************************************************************************************************
///*********************************************************************************************************************
///*********************************************************************************************************************
///*********************************************************************************************************************


///*********************************************************************************************************************
/// ********************************************** CRIG ****************************************************************
///*********************************************************************************************************************
class CRig {
public:
    static void Solve(shared_ptr<CFITCoin> x) {
        FITProblem problem(std::move(x));
        problem.Solve();
    }

    static void Solve(ACVUTCoin x) {
        CVUTProblem problem(std::move(x));
        problem.Solve();
    }

    CRig() : m_newProblems(), m_solvedProblems() {
        g_zavolanStop = false;
        g_zavolanStop = false;
        g_vseRecieved = false;
        g_vypoctyDobehly = false;
    }

    ~CRig() = default;

    void Start(int thrCnt) {
        g_zavolanStop = false;
        g_zavolanStop = false;
        g_vseRecieved = false;
        g_vypoctyDobehly = false;

        g_numOfThreads = (unsigned) thrCnt;
        for (int i = 0; i < thrCnt; ++i) m_workThreads.emplace_back(&CRig::workThreadFunc, this);
    }

    void Stop();

    // Vzhledem k použitému HW je rozumné najednou obsluhovat přibližně 5-10 zákazníků.
    // Jedná se o test bonusový.
    void AddCustomer(ACustomer c) {
        if (g_zavolanStop) return; // kdyz je stop noveho zakaznika uz neprijmu

        User *newUser = new User(c);

        m_users.insert(newUser);

        m_recievingThreads.emplace_back(&CRig::receiveFITProblem, this, c);
        m_recievingThreads.emplace_back(&CRig::receiveCVUTProblem, this, c);
        m_deliveringThreads.emplace_back(&CRig::deliverProblem, this, c);
    }

private:
    Queue m_newProblems;
    QueueSOLVED m_solvedProblems;
    QueueUsers m_users;

    vector<thread> m_workThreads;
    vector<thread> m_recievingThreads;
    vector<thread> m_deliveringThreads;

    void workThreadFunc();

    void receiveFITProblem(const shared_ptr<CCustomer> &customer) {
        AFITCoin fitcoin;
        while ((fitcoin = customer->FITCoinGen()) != nullptr) {
            {
                std::unique_lock<std::mutex> locker(g_vseRecievedMutex);
                ++g_bigProblemsCnt;
            }
            m_newProblems.insert(new FITProblem(customer, fitcoin));
            m_users.incrementProblems(customer);
        }

        m_users.setFITFinished(customer);
    }

    void receiveCVUTProblem(const shared_ptr<CCustomer> &customer) {
        ACVUTCoin cvutcoin;
        while ((cvutcoin = customer->CVUTCoinGen()) != nullptr) {

            {
                std::unique_lock<std::mutex> locker(g_vseRecievedMutex);
                ++g_bigProblemsCnt;
            }


            m_newProblems.insert(new CVUTProblem(customer, cvutcoin));
            m_users.incrementProblems(customer);
        }
        m_users.setCVUTFinished(customer);
    }

    void deliverProblem(const shared_ptr<CCustomer> &customer);
};

///*********************************************************************************************************************
///*********************************************************************************************************************
///*********************************************************************************************************************

// take remaining requests from existing customers and wait till computation is finished and results returned
// uvolni nacitaci, predavaci i pracovni vlakna
// potom se vrati
void CRig::Stop() {

    g_zavolanStop = true; // prestane prijimat nove zakazniky - pro nikoho jineho
    cond_userBuff.notify_all();

    for (auto &t : m_recievingThreads) //pocka na doruceni problemu od stavajicich zakazniku
        t.join();

    // spi dokud neni zpracovan posleni velky problem

    while (true)
    {
        std::unique_lock<std::mutex> locker(g_vseRecievedMutex);
        cond_allBigProblems.wait(locker, [this]() { return g_bigProblemsCnt == 0; });
        break;
    }

    g_vseRecieved = true;

    //tady // cond_allBigProblems.notify_all();

    cond_problemsBuff.notify_all();

    for (auto &t : m_workThreads)  // pocka na zpracovani vsech soucasnych problemu
        t.join();

    g_vypoctyDobehly = true;
    cond_solvedBuff.notify_all();

    for (auto &t : m_deliveringThreads) // pocka na doruceni vsech soucasnych problemu
        t.join();
}

///*********************************************************************************************************************
void CRig::workThreadFunc() {

    while (true) {

        Problem *p = m_newProblems.remove();
        if (p == nullptr) // vrati null ptr jen kdyz uz zadne problemy nebudou
            break;

        if (!p->Partial()) {
            vector<Problem *> subproblems = p->GetPartialProblems();
            {
                std::unique_lock<std::mutex> locker(g_vseRecievedMutex);
                --g_bigProblemsCnt;
                cond_allBigProblems.notify_all();
            }
            m_newProblems.insertAll(subproblems);
        } else {
            if (p->Solve()) {
                m_solvedProblems.insert(p);
            } else
                delete p;
        }
    }
  //  cout << "problemy dosly" << endl;
}

///*********************************************************************************************************************
void CRig::deliverProblem(const shared_ptr<CCustomer> &customer) {

    User *u = m_users.getUser(customer);

    while (!u->m_CVUTFinished || !u->m_FITFinished || !u->m_count == 0) {

        Problem *p = m_solvedProblems.tryGetProblem(customer);
        if (p == nullptr)
            continue;
        else {
            p->Deliver();
            m_users.decrementProblems(customer);
        }

        delete p;
    }

    delete m_users.remove(customer); // udelame volno pro noveho customera

}


/// ********************************************************************************************************************
/// ********************************************************************************************************************
/// ********************************************************************************************************************
/// ********************************************************************************************************************
/// ********************************************************************************************************************
/// ********************************************************************************************************************


#ifndef __PROGTEST__

//=================================================================================================
class CCustomerTest : public CCustomer {
public:
    //---------------------------------------------------------------------------------------------
    CCustomerTest()
            : m_FITIdx(0),
              m_CVUTIdx(0) {
        prepareTests();
    }

    //---------------------------------------------------------------------------------------------
    AFITCoin FITCoinGen() override {
        if (m_FITIdx < m_TestsFIT.size())
            return AFITCoin(m_TestsFIT[m_FITIdx++].first);
        else
            return AFITCoin();
    }

    //---------------------------------------------------------------------------------------------
    ACVUTCoin CVUTCoinGen() override {
        if (m_CVUTIdx < m_TestsCVUT.size())
            return m_TestsCVUT[m_CVUTIdx++].first;
        else
            return ACVUTCoin();
    }

    //---------------------------------------------------------------------------------------------
    void FITCoinAccept(AFITCoin x) override {
        auto it = find_if(m_TestsFIT.begin(), m_TestsFIT.end(),
                          [x](const pair<AFITCoin, size_t> &v) {
                              return v.first == x;
                          });
        if (it == m_TestsFIT.end())
            printf("FITCoin: an unknown problem returned\n");
        else if (it->second != x->m_Count)
            printf("FITCoin: count mismatch, %zu != %zu\n", x->m_Count, it->second);
    }

    //---------------------------------------------------------------------------------------------
    void CVUTCoinAccept(ACVUTCoin x) override {
        auto it = find_if(m_TestsCVUT.begin(), m_TestsCVUT.end(),
                          [x](const pair<ACVUTCoin, size_t> &v) {
                              return v.first == x;
                          });
        if (it == m_TestsCVUT.end())
            printf("CVUTCoin: an unknown problem returned\n");
        else if (it->second != x->m_Count)
            printf("CVUTCoin: count mismatch, %zu != %zu\n", x->m_Count, it->second);
    }
    //---------------------------------------------------------------------------------------------
private:
    vector<pair<AFITCoin, uint64_t> > m_TestsFIT;
    vector<pair<ACVUTCoin, uint64_t> > m_TestsCVUT;
    size_t m_FITIdx;
    size_t m_CVUTIdx;

    //---------------------------------------------------------------------------------------------
    void prepareTests() {
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x6b8b4567}, 0), 1);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x327b23c6}, 1), 33);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x643c9869}, 15), 1846943453);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x66334873}, 16), 2448023843);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x74b0dc51}, 17), 3013746563);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x19495cff}, 31), 4294967295);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x2ae8944a}, 32), 4294967296);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(vector<uint32_t>{0x406518a4, 0x412f1ca1, 0x406d1ca5}, 2), 0);
        m_TestsFIT.emplace_back(
                make_shared<CFITCoin>(vector<uint32_t>{0x5cad9aeb, 0x7da99afb, 0x5d2992cb, 0x5d2992cb}, 4), 147);
       // m_TestsFIT.emplace_back(
       //         make_shared<CFITCoin>(vector<uint32_t>{0x396070f3, 0x3b6070f2, 0x796070a2, 0x796470b2, 0x3a6470f3}, 9),
       //         3294848);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(
       //         vector<uint32_t>{0x408543d7, 0x428543d6, 0x4a81c3d7, 0x48a1c3d6, 0x4aa143d7, 0x4a8dc3d7}, 21),
       //                         3985193193);
       // m_TestsFIT.emplace_back(make_shared<CFITCoin>(
       //         vector<uint32_t>{0xe31d5fdc, 0xe1891fdc, 0xa19b7fde, 0xe3091ddc, 0xa39f6ddc, 0xe19b4fdc, 0xa11d5ddc},
       //         6), 337);
       //m_TestsFIT.emplace_back(make_shared<CFITCoin>(
       //        vector<uint32_t>{0x5ba85e68, 0x3aa85e78, 0x5aaa5ef8, 0x1baa5efa, 0x7ba856ea, 0x5baa5e68, 0x5bb85678,
       //                         0x3aaa5668, 0x5aaa56fa, 0x1bba5e6a}, 1), 0);
       //m_TestsFIT.emplace_back(make_shared<CFITCoin>(
       //        vector<uint32_t>{0x72d8aa96, 0x64c8a296, 0x76d8aaf6, 0x64c0aafe, 0x76d8aa9a, 0x76d0aaba, 0x74c0aab2,
       //                         0x70c8aaba, 0x64d0aa96, 0x76c0aad2, 0x62c0a2be, 0x74d0aa96, 0x76c8a2f2, 0x74c8aafe,
       //                         0x76c8aada, 0x66c0aaf6, 0x70d0aab2, 0x66c0aab6, 0x60d0a29a, 0x76c8aad2, 0x74c8aab2,
       //                         0x66c0a2f2, 0x62d8aa96, 0x60d8a2d2, 0x76d8aada, 0x62c0aab6, 0x72d8aaf6, 0x74d0a2de,
       //                         0x64c8aab2, 0x60c0a2f2, 0x72d8a292, 0x60c8a2ba, 0x64c8aaf6, 0x72d0a296, 0x66c8a296,
       //                         0x64c0a292, 0x62c8aabe, 0x62c8a2b6, 0x76d0aabe, 0x76d8a2d6, 0x62d0aafa, 0x60d8a2fa,
       //                         0x74d0aada, 0x60c0aafa, 0x76c0a2f6, 0x74c0aab2, 0x70c0a2d2, 0x70d0aa9a, 0x62c0aada,
       //                         0x72d8aafa}, 12), 8084682);
       //m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x45, 0x23, 0x98, 0x48, 0xdc}, 0, 1), 10);
       // m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x5c, 0x94, 0x58, 0x1f, 0x7c}, 3, 8), 277);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x58, 0xd7, 0x41, 0x1e, 0xa9}, 1, 12),
       //                           704); //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0xe1, 0x00, 0x62, 0x08, 0x27}, 0, 25),
       //                           1390); //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x23, 0xe9, 0xcd, 0x43, 0x0f}, 0, 70), 1600);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x25, 0xf9, 0x72, 0xc2, 0xd7}, 7, 7), 76);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0xc4, 0x07, 0xfb, 0x5d, 0x50, 0xd7}, 9, 14),
       //                           597);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0xba, 0xe4, 0x30, 0xd9, 0x61, 0x89}, 9, 29),
       //                           1549);  //chyba
       //  m_TestsCVUT.emplace_back(
       //          make_shared<CCVUTCoin>(vector<uint8_t>{0xb1, 0xa3, 0xa8, 0x5a, 0x84, 0xa8, 0xbd}, 15, 20), 729);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(vector<uint8_t>{0x8c, 0xd0, 0xe0, 0x76, 0x9e, 0x24, 0x86, 0xc4}, 19, 37), 1771);  //chyba
       //  m_TestsCVUT.emplace_back(
       //          make_shared<CCVUTCoin>(vector<uint8_t>{0x1d, 0xf8, 0x86, 0xf5, 0xbd, 0x8d, 0xf0, 0x1a, 0xdd}, 6, 16),
       //          1209);
       //  m_TestsCVUT.emplace_back(
       //          make_shared<CCVUTCoin>(vector<uint8_t>{0xc8, 0xd4, 0xc2, 0xf8, 0xad, 0x23, 0x82, 0x5f, 0xc6, 0x2a}, 5,
       //                                 43), 4970);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xb9, 0x4a, 0xd3, 0x77, 0xd7, 0xa4, 0x58, 0x4e, 0x42, 0x7c, 0xd4}, 27, 70), 3806);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x06, 0x9a, 0xcc, 0x8d, 0x8f, 0x89, 0x1b, 0x7f, 0xa4, 0xf9, 0x48, 0x78}, 50, 100),
       //                           2167);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xbb, 0x40, 0x26, 0xde, 0xc3, 0x85, 0xa5, 0xed, 0x3f, 0xf0, 0xc1, 0xb7, 0xc7}, 42, 99),
       //                           4074);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x65, 0x0f, 0x15, 0xa8, 0x8c, 0xe9, 0xaf, 0x26, 0xb6, 0x3c, 0xb6, 0x40, 0x57, 0x35}, 6,
       //          42), 7397);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xe4, 0x50, 0x7e, 0x5d, 0x0b, 0xbf, 0x84, 0xea, 0x82, 0x0a, 0x8f, 0x70, 0x4a, 0x7f,
       //                          0x31}, 29, 120), 10254);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x02, 0x47, 0x96, 0x12, 0x5d, 0x3f, 0x9e, 0x47, 0xee, 0xc5, 0xfd, 0x2b, 0x7b, 0x3e,
       //                          0x82, 0xb1}, 47, 95), 5992);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x23, 0xd3, 0x2f, 0x81, 0xdf, 0xee, 0x06, 0xca, 0x70, 0x11, 0x59, 0xe0, 0x5b, 0xd9,
       //                          0x11, 0x5e, 0x21}, 87, 101), 1260);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xa8, 0x70, 0x7e, 0xe7, 0x0e, 0xc5, 0xd6, 0xd4, 0xc3, 0x01, 0x4f, 0x01, 0x84, 0x01,
       //                          0x24, 0x57, 0x30, 0xa5}, 55, 130), 8277);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x37, 0x1e, 0xac, 0x01, 0x8f, 0xbd, 0x5a, 0x70, 0x18, 0x34, 0x82, 0x77, 0x55, 0x2a,
       //                          0xe7, 0xd3, 0x12, 0xf6, 0x99}, 41, 140), 14161);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xe8, 0xca, 0x5c, 0xea, 0x1a, 0x5d, 0x6e, 0x1b, 0x82, 0xc5, 0x4b, 0x28, 0xfd, 0x6a,
       //                          0xd4, 0xfe, 0xfa, 0x91, 0x59, 0x6a}, 67, 111), 6494);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xaa, 0x8d, 0xec, 0x21, 0xe3, 0x17, 0x09, 0xb7, 0x29, 0xff, 0x50, 0x12, 0xc9, 0xac,
       //                          0xfc, 0xe3, 0x0a, 0x6b, 0xff, 0x8d, 0x31, 0x4a, 0xb5, 0x2e, 0xb5}, 29, 127),
       //                           30444);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x8a, 0x2c, 0xaf, 0x1b, 0x85, 0x1a, 0xc6, 0x13, 0x06, 0xe8, 0xf6, 0x1d, 0xf1, 0xae,
       //                          0x47, 0xf0, 0xfe, 0x5a, 0xb9, 0xab, 0x57, 0x9d, 0xb6, 0xc2, 0x9d, 0x43, 0xf3, 0xe8,
       //                          0xf8, 0x22}, 33, 200), 50188);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x9d, 0x82, 0x4e, 0x4d, 0x9e, 0xd4, 0x67, 0x64, 0xe7, 0x6e, 0x4c, 0xde, 0x8c, 0x3d,
       //                          0x8c, 0xd3, 0x2e, 0x8a, 0x2e, 0xe8, 0x36, 0x85, 0x85, 0xec, 0x48, 0x23, 0x2f, 0x3b,
       //                          0x0b, 0x28, 0x5e, 0xa8, 0xab, 0xac, 0xf6}, 100, 210), 28425);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0x4a, 0x80, 0x5d, 0xae, 0x68, 0xcc, 0xfb, 0x46, 0x58, 0x39, 0xd2, 0x2c, 0x67, 0x5d,
       //                          0x5a, 0x4f, 0x94, 0xdf, 0xd5, 0x81, 0x27, 0xf8, 0xb1, 0x63, 0x04, 0xd9, 0xc1, 0xac,
       //                          0x85, 0x6e, 0xa2, 0xcf, 0xef, 0x00, 0x7e, 0x57, 0xcd, 0x79, 0x9e, 0x25}, 45, 300),
       //                           91049);
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xb3, 0x71, 0x51, 0x1b, 0xcf, 0xab, 0x6b, 0x63, 0x8b, 0x41, 0xe4, 0xb2, 0x3a, 0x95,
       //                          0x16, 0x3e, 0x6f, 0xd8, 0xea, 0xf4, 0x46, 0x8d, 0xc3, 0x36, 0x8e, 0x42, 0x8d, 0x5b,
       //                          0xbb, 0x2c, 0x81, 0x6f, 0x9d, 0xd2, 0x8a, 0x6c, 0x7e, 0xf5, 0xd0, 0x09, 0x37, 0xb4,
       //                          0xbc, 0x71, 0x4a}, 176, 218), 14019);  //chyba
       //  m_TestsCVUT.emplace_back(make_shared<CCVUTCoin>(
       //          vector<uint8_t>{0xd3, 0xaf, 0xb9, 0xab, 0x9a, 0xae, 0xf2, 0x28, 0x72, 0x28, 0xb7, 0xb4, 0xb6, 0x13,
       //                          0x70, 0xe2, 0x94, 0xdf, 0x80, 0x67, 0x6a, 0xed, 0xe5, 0x60, 0xbd, 0xef, 0x97, 0x72,
       //                          0xac, 0x08, 0xbd, 0x7f, 0xb8, 0x76, 0x2b, 0x53, 0x25, 0x1d, 0x7b, 0x97, 0x46, 0x32,
       //                         0x4c, 0xfc, 0x45, 0xbc, 0xdf, 0xda, 0x9c, 0x60}, 245, 365), 22990); //chyba
    }
};

//=================================================================================================
void TestParallel() {
    CRig s;
    s.Start(20);
    s.AddCustomer(make_shared<CCustomerTest>());
    s.Stop();
}

//=================================================================================================
void TestSequential() {
    CCustomerTest c;

    for (AFITCoin x = c.FITCoinGen(); x; x = c.FITCoinGen()) {
        CRig::Solve(x);
        c.FITCoinAccept(x);
    }

    for (ACVUTCoin x = c.CVUTCoinGen(); x; x = c.CVUTCoinGen()) {
        CRig::Solve(x);
        c.CVUTCoinAccept(x);
    }
}

//=================================================================================================
int main() {
    // TestSequential();
     TestParallel();
    std::cout << dist1 << std::endl<< dist2 << std::endl<< dist3 << std::endl<< dist4 << std::endl;
    return 0;
}


#endif /* __PROGTEST__ */