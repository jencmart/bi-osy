#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
#include "common.h"
using namespace std;
#endif /* __PROGTEST__ */



/**
 Odevzdávejte zdrojový kód s implementací funkce MemMgr, podtřídy CCPU a dalších podpůrných tříd/metod. Za základ řešení použijte zdrojový soubor solution.cpp z přiložené ukázky. Pokud zachováte bloky podmíněného překladu, můžete zdrojový soubor solution.cpp rovnou odevzdávat na Progtest.

Všimněte si, že pojmenování Vaší podtřídy CCPU není vůbec důležité. Instance této třídy vyrábí výhradně Vaše část implementace a testovací prostředí pracuje pouze s polymorfním rozhraním předka.


 Nepokoušejte se pro datové stránky/adresáře stránek používat paměť mimo přidělený blok paměti předaný při volání MemMgr. Implementace CCPU v testovacím prostředí kontroluje, zda jsou použité adresy stránek/adresářů stránek z rozsahu spravované paměti. Pokud jsou mimo, řešení bude odmítnuto.



 Základní verze programu musí umět spouštět procesy bez kopírování adresního prostoru (parametr copyMem bude v těchto testech vždy false). Takové řešení neprojde nepovinným a bonusovým testem, tedy bude hodnoceno méně body.



 Řešení, které bude správně (ale neefektivně) zpracovávat parametr copyMem, projde i nepovinným testem a dostane nominální hodnocení 100% bodů. Pro zvládnutí posledního (bonusového) testu je potřeba správně a hlavně efektivně kopírovat obsah adresního prostoru volajícího do nově vzniklého procesu (je-li to požadováno). Prosté kopírování nestačí, nebude mít k dispozici dost paměti. Správné řešení bude muset použít techniku copy-on-write.


 Pro testování využijte přiložený archiv s několika připravenými testy. Tyto testy (a některé další testy) jsou použité v testovacím prostředí. Dodané testy dále ukazují použití požadovaných funkcí/tříd.

 Ve zdrojových kódech přiloženého archivu je vidět seznam dostupných hlavičkových souborů. Všimněte si, že STL není dostupná. Reálný OS také nemá STL k dispozici (není new/delete), navíc byste si museli implementovat vlastní alokátory. Pro vytváření vláken použijte pthread rozhraní, C++11 thread API není v této úloze k dispozici.



 */
class CWhateverNiceNameYouLike : public CCPU
{
  public:

    virtual uint32_t         GetMemLimit                   ( void ) const;
    virtual bool             SetMemLimit                   ( uint32_t          pages );
    virtual bool             NewProcess                    ( void            * processArg,
                                                             void           (* entryPoint) ( CCPU *, void * ),
                                                             bool              copyMem );
  protected:
   /*
    if copy-on-write is implemented:

    virtual bool             pageFaultHandler              ( uint32_t          address,
                                                             bool              write );
    */
};



void               MemMgr                                  ( void            * mem,
                                                             uint32_t          totalPages,
                                                             void            * processArg,
                                                             void           (* mainProcess) ( CCPU *, void * ))
{
  // todo
}
