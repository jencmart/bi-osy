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
 CPU
bude implementováno jako C++ třída CCPU, respektive její podtřída. Simulovaný proces bude volat metody této třídy, metody zprostředkují čtení/zápis do paměti.


Základem řešení je třída CCPU. Tato třída zjednodušeně simuluje chování procesoru i386 při překladu adres. Část metod je implementovaná (v testovacím prostředí a v přiloženém zdrojovém kódu). Vaším úkolem bude od této třídy odvodit potomka a v něm implementovat metody, které jsou v CCPU abstraktní. Rozhraní třídy je navrženo takto:
:
Konstruktor CCPU(memStart, pageTableRoot). Konstruktor inicializuje členské proměnné podle parametrů. Parametr memStart udává "opravdový" ukazatel na počátek bloku paměti, který byl simulaci předán při volání MemMgr. Druhým parametrem je rámec stránky, kde je umístěn adresář stránek nejvyšší úrovně (toto nastavení stránkování bude použito pro přepočet adres v tomto simulovaném CPU).
Destruktor můžete v odvozené třídě implementovat, pokud budete potřebovat uvolňovat alokované prostředky.
Metoda GetMemLimit zjistí, kolik stránek má alokovaných proces, pro který je používána tato instance CCPU. Tato metoda je abstraktní, její implementace v odvozené třídě je Váš úkol.
Metoda SetMemLimit nastaví paměťový limit (v počtu stránek) pro tento proces. Metoda může být použita jak pro zvětšení, tak pro zmenšení paměťového prostoru simulovaného procesu. Návratovou hodnotou je true pro úspěch, false pro neúspěch (např. není dostatek paměti pro alokaci). Implementace je Váš úkol.
Metoda NewProcess vytvoří nový simulovaný proces (vlákno). Parametrem volání je adresa funkce spouštěné v novém vlákně, její parametr a příznak copyMem. Význam prvních parametrů je zřejmý. Třetí parametr udává, zda má být nově vzniklému "procesu" vytvořen prázdný adresní prostor (hodnota false, GetMemLimit v novém procesu bude vracet 0) nebo zda má získat paměťový obsah jako kopii paměťového prostoru stávajícího procesu (true). Úspěch je signalizován návratovou hodnotou true, neúspěch false. Metodu budete implementovat v odvozené třídě.
Metoda ReadInt přečte hodnotu typu uint32_t ze zadané simulované virtuální adresy. Návratovou hodnotou je hodnota true pro úspěch nebo hodnota false pro selhání. Selháním je např. pokus o čtení mimo hranice alokovaného adresního prostrou (reálný OS by v takové situaci vyvolal signál "Segmentation fault", simulace bude reagovat takto mírně). Metoda je kompletně implementovaná v dodané třídě, Vaše implementace ji nebude nijak měnit. Pro zjednodušení předpokládáme pouze zarovnaný přístup (zadaná virtuální adresa je násobek 4, tedy celé čtení se odehraje v jedné stránce).
Metoda WriteInt zapíše hodnotu typu int na zadanou simulovanou virtuální adresu. Návratová hodnota je true pro úspěch nebo false pro neúspěch. Metoda je opět hotová v dodané třídě. Opět předpokládáme pouze virtuální adresy jako násobky 4, tedy opět celý zápis proběhne v jedné stránce.
Metoda virtual2physical přepočítává simulovanou adresu ("virtuální" adresa v procesu) na adresu "fyzickou". Metoda je implementovaná v dodané třídě. Implementace odpovídá chování procesoru i386 pro základní variantu stránkování (2 úrovně adresářů stránek, 4KiB stránka, 1024 odkazů v adresáři stránek). Vaše implementace nebude tuto metodu nijak měnit. Budete ale muset odpovídajícím způsobem vyplnit adresáře stránek, aby je metoda správně zpracovala. V reálném OS je tato funkce "ukryta" v HW procesoru.
Metoda pageFaultHandler je vyvolána, pokud při přepočtu adres dojde k chybě - výpadku stránky. Může se jednat o skutečnou chybu (neoprávněný přístup) nebo o záměr (odložení stránky na disk, implementace strategie copy-on-write). Metoda je vyvolána s parametry přepočítávané virtuální adresy a příznaku, zda se jedná o čtení nebo zápis. Návratovou hodnotou metody je true pokud se podařilo odstranit příčinu výpadku stránky (např. načtení stránky z disku) nebo false pro indikaci trvalého neúspěchu (přístup k paměti mimo alokovaný rozsah). Pokud je vráceno true, je přepočet adres zopakován, pro navrácenou hodnotu false je přepočet ukončen a programu je vrácen neúspěch (ReadInt/WriteInt vrátí false). Implicitní chování vrací vždy false, toto chování postačuje pro základní implementaci. Pokud se ale rozhodnete implementovat strategii copy-on-write, budete muset tuto metodu v podtřídě změnit. Pozor, pokud trvale vracíte true a neodstraníte příčinu výpadku, skončí program v nekonečné smyčce. Reálný HW signalizuje výpadek stránky nějakým přerušením, obsluha přerušení odpovídá této metodě. Pokud na reálném HW obsluha přerušení neodstraní příčinu výpadku a požaduje opakování přepočtu adres, může též dojít k zacyklení nebo k přerušení typu double-fault.
Konstanta OFFSET_BITS udává počet bitů použitých pro adresaci uvnitř stránky (zde 12 bitů).
Konstanta PAGE_SIZE udává velikost stránky v bajtech (4096 B).
Konstanta PAGE_DIR_ENTRIES udává počet záznamů v adresáři stránek (zde 1024).
Položka adresáře stránek má 4 bajty (32 bitů). Její struktura je:
    31                             12       6 5 4 3 2 1 0
   +---------------------------------+-----+-+-+-+-+-+-+-+
   | fyzická adresa stránky/adresáře |xxxxx|D|R|x|x|U|W|P|
   +---------------------------------+-----+-+-+-+-+-+-+-+

Konstanta ADDR_MASK obsahuje masku, která přiložená k položce adresáře stránek zachová pouze adresu stránky/adresu podřízeného adresáře stránek.
Konstanta BIT_DIRTY je nastavena CPU v případě zápisu do stránky (v obrázku bit D).
Konstanta BIT_REFERENCED je nastavena CPU v případě čtení ze stránky (bit R v obrázku).
Konstanta BIT_USER určuje, zda ke stránce má přístup i uživatel (1) nebo pouze supervisor (0). Pro naší simulaci bude potřeba bit vždy nastavit na 1 (bit U v obrázku).
Konstanta BIT_WRITE určuje, zda lze do stránky zapisovat (1) nebo pouze číst (0), bit W v obrázku.
Konstanta BIT_PRESENT určuje, zda je stránka přítomná (1) nebo odložená na disk/nepřístupná (0), bit P v obrázku.
Zbývající bity nejsou v naší simulaci použité. Reálný procesor jimi řídí cache (write-through/write-back/cache-disable), execute-disable a další (i486+).
 */
//-------------------------------------------------------------------------------------------------
                   CCPU::CCPU                              ( uint8_t   * memStart,
                                                             uint32_t    pageTableRoot )
{
  m_MemStart = memStart;
  m_PageTableRoot = pageTableRoot;
}
//-------------------------------------------------------------------------------------------------
bool               CCPU::ReadInt                           ( uint32_t          address,
                                                             uint32_t        & value )
{
  if ( address & 0x3 ) return false; // not aligned
  uint32_t * addr = virtual2Physical ( address, false );
  if ( ! addr ) return false;
  value = *addr;
  return true;
}
//-------------------------------------------------------------------------------------------------
bool               CCPU::WriteInt                          ( uint32_t          address,
                                                             uint32_t          value )
{
  if ( address & 0x3 ) return false; // not aligned
  uint32_t * addr = virtual2Physical ( address, true );
  if ( ! addr ) return false;
  *addr = value;
  return true;
}
//-------------------------------------------------------------------------------------------------
uint32_t         * CCPU::virtual2Physical                  ( uint32_t          address,
                                                             bool              write )
{
  const uint32_t reqMask = BIT_PRESENT | BIT_USER | (write ? BIT_WRITE : 0 );
  const uint32_t orMask = BIT_REFERENCED | (write ? BIT_DIRTY : 0);



  while ( 1 )
  {
    uint32_t * level1 = (uint32_t *)(m_MemStart + (m_PageTableRoot & ADDR_MASK)) + (address >> 22);

    if ( (*level1 & reqMask ) != reqMask )
    {
      if ( pageFaultHandler ( address, write ) ) continue;
      return NULL;
    }
    uint32_t * level2 = (uint32_t *)(m_MemStart + (*level1 & ADDR_MASK )) + ((address >> OFFSET_BITS) & (PAGE_DIR_ENTRIES - 1));

    if ( (*level2 & reqMask ) != reqMask )
    {
      if ( pageFaultHandler ( address, write ) ) continue;
      return NULL;
    }
   *level1 |= orMask;
   *level2 |= orMask;
    return (uint32_t *)(m_MemStart + (*level2 & ADDR_MASK) + (address & ~ADDR_MASK));
  }
}
//-------------------------------------------------------------------------------------------------

