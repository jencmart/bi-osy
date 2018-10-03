/* reseni s C++11 vlakny, mutexy a cond. var
 */
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <algorithm>
#include <unistd.h>

using namespace std;

const int PLATFORM_MAX = 3;

class CTrain {
public:
    CTrain(int id) : m_ID(id), m_Platform(PLATFORM_MAX) {}

    condition_variable m_Cond;
    int m_ID;
    int m_Platform;
};

// Do fronty cekajicich nechceme ukladat kopie CTrain, ale odkazy (reference) na
// existujici objekty. Obecne, neni dobry napad kopirovat objekty typu thread/mutex/cond_var.
// Kompilator se tomu brani, prislusne copy konstruktory jsou deleted.
// Bud muzeme ukladat ukazatele, nebo reference v podobe reference-wrapperu


mutex g_Switch;                 /* vyhybka - zamek pro kritickou sekci vyberu koleje */
deque<reference_wrapper<CTrain> > g_queueWaiting;
int g_StationPlatform[PLATFORM_MAX]; /* priznaky obsazeni koleji */

//-------------------------------------------------------------------------------------------------
/* Vypis cekajicich vlaku + obsazeni koleji.
 */
void displayStation(void) {
    // display in reverse order
    for (auto it = g_queueWaiting.rbegin(); it != g_queueWaiting.rend(); ++it)
        printf(" %2d ", it->get().m_ID);

    printf("-> [");
    for (int i = 0; i < PLATFORM_MAX; i++)
        if (g_StationPlatform[i])
            printf(" %2d ", g_StationPlatform[i]);
        else
            printf(" -- ");
    printf("]\n");
}
/*---------------------------------------------------------------------------*/
// Najde volnou kolej, vrati jeji cislo. Predpoklada, ze volajici vlakno vlastni zamek g_Switch.
inline int findPlatform() {
    return static_cast<int>(find(g_StationPlatform, g_StationPlatform + PLATFORM_MAX, 0) - g_StationPlatform);
}
/*---------------------------------------------------------------------------*/
// Prijezd do nadrazi
int waitStation(int id) {
    unique_lock<mutex> locker(g_Switch);
    printf("Train %d arriving\n", id);

    int pl;

    // Pokud uz nekdo ceka || je plno
    if (!g_queueWaiting.empty() || (pl = findPlatform()) == PLATFORM_MAX) {
        CTrain myself(id);
        g_queueWaiting.emplace_back(myself);

        // Zobrazeni
        displayStation();

        // uspani & odemceni zamku, pri probuzeni kontrolujeme, ze jsme opravdu prvni na rade.
        // (Mohlo by se stat,
        // A pro extra spatne planovani by se mohlo stat, ze druhy probuzeny cekajici vlak
        // ziska cpu drive nez prvni.
        myself.m_Cond.wait(locker, [&pl, id] { return (void)( (pl = findPlatform()) != PLATFORM_MAX && g_queueWaiting.front().get().m_ID == id); });// tedy pokud nejsme prvni na rade nebo neni volno, zase se uspavame


        // mame jistotu, ze odstranujeme sebe z fronty cekajicich
        g_queueWaiting.pop_front();

        // nepravdepodobne, ale mozne. Pokud by se podarilo probudit najednou dva cekajici vlaky
        // a druhy ve fronte by se dostal k CPU drive nez prvni, pak se opet uspal (podminka u cond_wait)
        // v takovem pripade by jej ale jiz nikdo neprobudil. Tedy z vlakna vlaku, ktery vjizdi na nadrazi
        // jeste probudime pripadny dalsi vlak za nami ve fronte. Pokud dalsi vlak najde volnou kolej, obsadi ji
        // (a pripadne probouzi i dalsi, ...)
        if (!g_queueWaiting.empty())
            g_queueWaiting.front().get().m_Cond.notify_one();
    }


    // stale vlastnime mutex, tedy bezpecne obsazujeme kolej
    g_StationPlatform[pl] = id;

    displayStation();
    printf("Train %d loading/unloading, platform %d\n", id, pl);
    return pl;
}

/*---------------------------------------------------------------------------*/
void leaveStation(int platform) {
    // Odjezd z nadrazi
    unique_lock<mutex> m(g_Switch);
    printf("Train %d departed, platform %d now empty\n", g_StationPlatform[platform], platform);
    // Uvolnime kolej
    g_StationPlatform[platform] = 0;
    displayStation();

    if (!g_queueWaiting.empty()) // Pokud nejaky vlak ceka, probudime jej.
        g_queueWaiting.front().get().m_Cond.notify_one();
}

/*---------------------------------------------------------------------------*/
// Funkce vlakna -simulator vlaku. Vlakno dostane  svuj identifikator - cislo <1; cntTrains>
int randomRange(int min, int max) { return (int) (min + 1.0 * rand() / RAND_MAX * (max - min));}
void trainThreadFunc(int id) {
    while (true) {
        int platform = waitStation(id);

        // Nakladka/vykladka
        usleep(static_cast<__useconds_t>(1000 * randomRange(1000, 5000)));

        leaveStation(platform);

        // Jizda vlaku
        usleep(static_cast<__useconds_t>(1000 * randomRange(2000, 10000)));

        if(id == 100)
            break;
    }
}

/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    vector<thread> trains;
    int thr = 5;

    for (int i = 0; i < thr; i++)
        trains.emplace_back(trainThreadFunc, i + 1);

    for (auto &t : trains)
        t.join();

    return 0;
}
