# Kostra projektu
Ta obsahuje základní architekturu GP (definované originálně v [SVN](../trunk/fpga/chips/architecture_gp/)). UCF soubor pak definuje propojení výstupů - pro použití je nejdůležitější práce s portem X. Tam je pak možné např rozblikat LED a podobně.

Potřebujete pracovat s ISE (otevřít proj.xise)

## Nainstalování ISE
Je možné Xilinx ISE nainstalovat do Linuxu nebo Windows WSL2 (pozor, je nutná vyšší verze 2).

- Stáhněte si ISE 14.7 pro Linux. JE nutná registrace. https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive-ise.html
- Vytvořte složku /opt/Xilinx a určete jí práva

```sh
mkdir /opt/Xilinx
chown $USER:USER /opt/Xilinx
```
- Rozbalte kdekoliv stažený archiv a spusťte ./xsetup
- Vyberte WebPack verzi a postupujte dle instalace

```sh
# načtení nastacení
. /opt/Xilinx/14.7/ISE_DS/settings64.sh
xlcm # správce licencí, nutné pouze poprvé
ise
```

Pokud by nebyla nalezena licence, na stránce https://www.xilinx.com/getlicense.html se přihlaste, vyberte ISE WebPAck a stáhněte ji (jako Xilinx.lic). V licenčním manažeru (xlcm) klikněte na Manage licences a importujte váš stažený soubor.

## Nahravání do FITkitu 
Poznámka: z ISE vytvoříte soubor .bin. Soubory hex jsou pro MCU, které nevyužíváte. Proto jsou už přiložené ve složce [upload](upload). Pro MCU musíte vytvořit soubor pro používané MCU msp430. Můžete využít multiplatformní nástroj pro nahrávání binárního kódu.

https://github.com/janchaloupka/fitkit-serial/releases

```
fitkit-serial-win-amd64.exe -hex1 testled_f1xx.hex -hex2 testked_f2xx.hex -bin testled.bin"
```

## Práce s QDevKit
https://youtu.be/0Q8TvkvfDHQ

https://www.fit.vut.cz/study/thesis-file/22463/22463.pdf 

