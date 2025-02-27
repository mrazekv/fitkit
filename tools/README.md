# Nastroje
Toto je kopie SVN repozitare se vsemi nastroji. Nektere jiz neni mozne zkompilovat.


Pokud by se podařilo zkompilovat FCMAKE, tak by byl problém vyřešen a funkčnost fitkitu společně s nahrávacím zařízením by byla obnovena

## FCMAKE
Utilita fcmake je multiplatformní nástroj sloužící ke generování Makefile souboru a skriptů nezbytných pro syntézu z popisového souboru project.xml. Nástroj je napsán jazyce C++ s pomocí Qt 4.

Podrobnější popis překladového systému naleznete v dokumentaci k překladovému systému.

## Kde aplikaci získat
V prostředí Windows je fcmake součástí instalačního balíku QDevKitu (viz QDevKit - Windows), v prostředí Linux je možné si jej stáhnout pomocí nabízených repozitářů nebo si jej stáhnout jako archiv (viz Zprovoznění FITkitu v nativním prostředí Linuxu).

## Návod k použití
Přejdeme do adresáře FITKit aplikace a program spustíme

```
user@pc:~/svn-fitkit/apps/demo/led$ fcmake

Project make files successfully created.
libfitkit make files successfully created.
libfitkit is compiled
libfitkit is up-to-date
```

Na základě informací obsažených v souboru project.xml je vytvořen seznam závislostí a vygenerován soubor Makefile. Kromě tohoto souboru se vytvářejí skripty a konfigurační soubory potřebné pro syntézu VHDL kódu a jeho simulaci.

Nápovědu k programu vyvoláme pomocí fcmake -h
```
Usage: fcmake [options] [project.xml]
Options:
 -g, --generate   Generate Makefile from project.xml. (default action)
 -u, --updatelist Create a list of all files and store it in base/filelist.
 -c, --clean      Clean generated files excluding Makefile.
 -a, --cleanall   Clean all generated files including Makefile.
 -h, --help       Show this help.

When no project.xml path given, FCMake looks in current directory.
```

## Návod na instalaci (starý)
https://web.archive.org/web/20221129045409/https://merlin.fit.vutbr.cz/FITkit/docs/navody/qdevkitlinux.html

## Úvod
Ačkoliv pro FITkit existuje aplikace s grafickým rozhraním (QDevKit), umožňující pohodlně spravovat projekty a provádět základní činnosti obsluhy FITkitu z PC, není tato součást bez reimplementace použitelná v současných distribucích Linuxu vzhledem k nekompatibilitě některých knihoven. Základní úkony s FITkitem je však možné provádět (někdy i efektivněji) pomocí příkazového řádku, takže i bez QDevKitu je možné FITkit v Linuxu plnohodnotně používat.

Upozorňuji: Před zahájením berte v úvahu skutečnost, že prostředí Xilinx ISE, které potřebujeme pro překlad VHDL, simulaci obvodů a syntézu konfiguračních bitstreamů pro FPGA, vyžaduje v poslední verzi 14.7 pro instalaci 17 GB volného místa na disku, dalších asi 6 GB zabere stažení instalátoru. Je to dáno rozsahem portfolia programovatelného HW Xilinx a složitostí návrhových prostředků. O něco méně náročné jsou starší verze, které jsou v archivu Xilinx též dostupné, nicméně nedoporučujeme používat nižší verzi ISE než 13.1.

## Instalace potřebných systémových prostředků a knihoven
Před započetím překladu nástrojů pro FITkit je třeba ověřit, případně do systému doinstalovat následující software, který je v Linuxu dostupný v podobě balíčků. Jejich názvy, uvedené v závorkách, se mohou v různých distribucích lišit:

- překladače C/C++ (gcc, g++)
- nástroj pro správu verzí (subversion)
- nástroje pro správu překladu (make, cmake)
- prostředí Python (python, python-dev)
- systém pro správu rozhraní (swig)
- vývojové knihovny (libqt4-dev, libusb-dev, libftdi-dev, libssh-dev)
- RedHat Package Manager (rpm)

Instalaci vybraných balíčků v prostředí Debian a příbuzných distribucích provedete pomocí systému apt. Instalaci je třeba provádět jako root, případně přes sudo. Například, pro doinstalaci g++, cmake a python-dev lze použít následující příkaz:

`sudo apt-get install g++ cmake python-dev`

## Instalace nástrojů pro FITkit
Kromě aplikace QDevKit lze ostatní nástroje, vytvořené specificky pro FITkit, v Linuxu bez problému používat. Konkrétně se jedná o překladový systém fcmake, knihovnu libkitclient a nástroj pro programování FITkitu fkflash. Dále je třeba nainstalovat překladový systém pro mikrokontroler MSP430 a systém Xilinx ISE pro práci s VHDL a syntézu obvodů pro FPGA (bude popsáno dále).

Pro usnadnění překladu nástrojů pro FITkit byl vytvořen jednoduchý skript Build_FKtools.sh, který provede stažení potřebných zdrojových souborů z SVN, nastavení konfigurace překladu a vygenerování balíčků .deb s přeloženými nástroji. Uživatel má možnost se přesvědčit o úspěšném dokončení překladu před provedením jakékoliv modifikace v OS.

Stáhněte si skript Build_FKtools.sh a uložte jej do nějakého prázdného adresáře, např. FITkit.

Vstupte do tohoto adresáře a skript spusťte. Po dokončení byste měli vidět podadresář DEBS a v něm tři vytvořené balíčky .deb s názvy příslušných nástrojů (konkrétně fcmake, libkitclient a fkflash). Pokud některý chybí či zjistíte jinou anomálii, je třeba podívat se na výpis skriptu a identifikovat problém. Nejčastější příčinou selhání překladu je absence některé z komponent zmíněných v sekci 2.

Dostanete-li hlášku:

error: PyString_FromStringAndSize was not declared in this scope

resultobj = PyString_FromStringAndSize(arg2, result);

otevřte soubor libkitclient/build/python/fitkitPYTHON_wrap.cxx, v něm řádek č. 15262, kde se uvedená chyba vyskytuje, nahraďte řádkem:

resultobj = PyUnicode_FromStringAndSize(arg2, result);

a spusťte překladový skript Build_FKtools.sh znovu.

Po úspěšném překladu je možné přistoupit k instalaci. Nejjednodušší je spustit skript FK_Linux_install, který byl vygenerován překladovým skriptem, zadáním:

sudo ./FK_Linux_install
Na Debian-based systémech můžete využít vytvořených .deb balíčků. Vstupte do podadresáře DEBS a zadejte příkaz:

sudo dpkg -i *.deb
Tím je instalace "FITkit-specific" nástrojů dokončena. Dále je potřeba ještě nainstalovat vybavení třetích stran.

## Instalace MSP430-gcc a Xilinx ISE
Pro MSP430 je k dispozici .deb archiv dle architektury systému, který po stažení nainstalujete pomocí dpkg:

POZNAMKA : možná už je v msp430-gcc balíčku

sudo dpkg -i nazev_deb_souboru
Pokud prohlížeč nenabídne uložení souboru na disk, použijte pravé tlačítko myši a zvolte položku Uložit odkaz jako...

Pro 32-bit: msp430-devtools-1.0-2_i386.deb
Pro 64-bit: msp430-devtools-1.0-2_amd64.deb

Alternativně lze mspgcc nainstalovat ze zdrojových kódů, tento postup však doporučuji použít pouze u distribucí, které neumí pracovat s .deb balíčky. Návod, jak na to, je k dispozici zde.


POZNáMKA: Instalace ISE je popsaná v IVH projektu

Stažení a instalace Xilinx ISE vyžaduje registraci na www.xilinx.com, aby následně mohla být vygenerována licence, která je pro vzdělávací účely zdarma. V registračním formuláři vyplňte své jméno, jako instituci můžete zadat název a adresu fakulty, v poli Job function zvolte popložku Student. Po úspěšné registraci budete mít přístup k produktům Xilinx, které jsou poskytovány ke stažení. Pro potřeby FITkitu nainstalujeme balík ISE WebPack, který je součástí prostředí ISE Design Suite.

Pozor, NEinstalujte balík Vivado - ten je určen pro vyšší řady FPGA, starší modely v něm nejsou popdporovány!

Z download sekce Xilinx stáhněte ISE Design Suite - Full Installer for Linux (podporuje 32- i 64-bit), po vybalení jako root (či přes sudo) spusťte setup a řiďte se pokyny instalátoru.

V nabídce instalátoru zaškrtněte variantu ISE WebPack, další nastavení ponechte na implicitních hodnotách.

V případě, že se v průběhu instalace ani po jejím dokončení nespustí nástroj pro správu licencí xlcm (Xilinx License Configuration Manager), budete jej muset spustit ručně. xlcm se po instalaci nachází v /opt/Xilinx/14.7/ISE_DS/common/bin/lin(64 pokud jedete na 64-bit OS), spusťte jej a nechce si vygenerovat licenci pro ISE WebPack - možno provést též na webu Xilinx. Licence je obvykle doručena na mail v podobě souboru Xilinx.lic, který následně na kartě xlcm Manage Licenses nahrajete pomocí tlačítka Load License... Nahrání bude potvrzeno dialogovým oknem ...successfully loaded, čímž je proces s licencí dokončen.

Pokud xlcm nelze spustit z důvodu absence sdílené knihovny libQt_Network.so, je potřeba jako root v OS vyhledat soubor libQtNetwork.so (bez podtržítka) a na tento soubor vytvořit symbolický odkaz s názvem libQt_Network.so. Vstupte do adresáře se souborem libQtNetwork.so a zadejte příkaz: sudo ln -s libQtNetwork.so libQt_Network.so Potom by spuštění xlcm již mělo proběhnout bez problémů.

Dle příručky byste ještě měli provést spuštění příslušné varianty skriptu settings v /opt/Xilinx/14.7/ISE_DS/ (dle architektury pro Linux-shell/Bash soubor s příponou .sh, ostatní .csh jsou pouze pro C-shell). Jako poslední je třeba nastavit cestu k binárkám ISE do proměnné PATH. Ve vašem domovském adresáři se nachází soubor .bashrc, případně .profile. Na jeho konec přidejte následující řádek:

export PATH=$PATH:/opt/Xilinx/14.7/ISE_DS/ISE/bin/lin(64 pokud jedete na 64-bit OS)
Nyní proveďte opětovné přihlášení do OS, v grafickém prostředí spusťte terminál a zadejte:

ise
Měl by se spustit ISE Project Navigator bez zobrazení varovného okna týkajícího se nenalezení licence. Tím ověříte, že je prostředí ISE korektně zadáno v PATH a má správně nainstalovánu licenci.

## Kontrola systému před použitím FITkitu
Pro komunikaci s FITkitem z úrovně non-root uživatele musí v OS existovat skupina plugdev a uživatelé s povolením práce s FITkitem musí být přiřazeni do této skupiny. Obvykle je již zajištěno, přesto doporučuji ještě ověřit. Zadáním následujícího příkazu v terminálu byste měli získat odezvu podobnou té o řádek níže, tj. s vaším loginem viditelným u této skupiny.

getent group plugdev
plugdev:x:46:michal
Pokud tomu tak není, provedete přidání skupiny plugdev do systému zadáním

sudo groupadd plugdev
Přidání uživatele do skupiny zajistíte pomocí

gpasswd -a uzivatel plugdev
Restartujte počítač.

## Zahájení práce s FITkitem
Nejprve stáhneme repozitář k FITkitu ze svn do vhodného adresáře, např. FITkit-svn, zadáním:

svn checkout http://merlin.fit.vutbr.cz/svn/FITkit/trunk FITkit-svn
Toto stačí provést pouze jednou, repozitář již budete mít uložen na disku. Případnou synchronizaci s novou verzí repozitáře můžete provést zadáním svn update v adresáři FITkit-svn.

Nyní ověříme funkci nainstalovaných komponent překladem vybrané vzorové aplikace a jejím nahráním do FITkitu. Vstupte např. do adresáře FITkit/apps/demo/led a postupně zadejte:

fcmake
make
make load
fcmake provede vygenerování všech potřebných Makefiles, make provede překlad (pomocí mspgcc a xst), na konci byste měli vidět hlášku Bitstream generation is complete, což indikuje správnou činnost syntézy VHDL pomocí ISE. Před zadáním make load je již nutné mít připojen FITkit, proběhne jeho naprogramování. Vzhledem k tomu, že nemáme k dispozici QDevKit, můžeme provést aktivaci aplikace nahrané do FITkitu následujícími způsoby.

POZNAMKA: load dělejte podle poznámek k IVH projektu

Vytažením propojek J8 a J9 (viz popis propojek). Výhodou tohoto přístupu je fakt, že pro standalone aplikace již nahrané do FITkitu je možné jejich aktivace bez nutnosti přípojení FITkitu k PC, stačí pouze napájení. Nevýhodou je nemožnost komunikace s FITkitem přes terminál.

Spuštěním příkazu make term. Je to v podstatě totéž, co připojení terminálu v QDevKitu poklepáním myší na ikonu FITkitu. Spustí se terminál v textovém režimu, který má obdobnou funkcionalitu jako připojení z QDevKitu.

Pozor, v této verzi pravděpodobně nebude fungovat připojení k FITkitu přes terminál (make term), spuštění aplikace naprogramované ve FITkitu je možné provést vytažením propojky J8.

Pro naprogramování FITkitu pomocí make load i připojení přes terminál musí být propojky J8 a J9 uzavřeny.

Některé další užitečné funkce překladového systému (podrobnosti viz popis překladového systému, sekce 3)

make purge
Provede odstranění všech souborů vzniklých překladem u daného projektu.

make isim
Spustí simulaci VHDL modelu pomocí nástroje ISIM.

## Závěr
Kromě zatím nefunkčního terminálu byste měli být schopni rozjet FITkit jak pro syntézu VHDL, tak pro tvorbu vestavných systémů pomocí MCU MSP430 či v kombinaci s VHDL. Pokud narazíte na problém, dejte mi vědět na bidlom AT fit.vutbr.cz nebo (ještě lépe) mě navštivte v L330 přímo s Vaším NB, kde budeme moci problémy důkladněji posoudit. Upozorňuji však, že ne vždy budu schopen poskytnout řešení okamžitě, mějte proto strpení, případně se o řešení zkuste pozajímat sami a přispět tak k dalšímu rozvoji kitu. V případě zjištění závažnějších problémů, které se potenciálně mohou na některých distribucích vyskytnout, stále platí doporučení použít předinstalovaný systém ve VirtualBoxu, jenž je po přihlášení dostupný včetně popisu instalace v sekci ke stažení.