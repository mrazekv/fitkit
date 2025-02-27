.. article:: apps_modeth_sntp
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Jednoduch� aplikace umo��uj�c� zjistit aktu�ln� �as prost�ednictv�m s�t� Ethernet

=================================
Synchronizace �asu pomoc� SNTP
=================================

Tato aplikace umo��uje synchronizaci �asu pomoc� protokolu SNTP (Simple Network Time Protocol), co� je zjednodu�en� verze NTP protokolu, kter� je ur�en� pro embedded za��zen�. 
Protokol NTP je navr�en tak, aby byl schopen zajistit, aby v�echny po��ta�e v s�ti obdr�eli stejnou �asovou informaci i p�esto, �e v doru�ov�n� paket� n�st�vaj� obecn� prom�nliv� zpo�d�n�. 
K tomuto ��elu se pou��v� iterativn� dotazov�n�.
SNTP protokol pracuje nad protokolem UDP a nevy�aduje proto slo�it� operace.
Nejjednodu��� mo�nost, jak z�skat aktu�ln� �as je odesl�n� SNTP paketu s ��dost� o aktu�ln� stav na port s ��slem 123 a dle odpov�di z�skat pot�ebn� �daje (nap�. datum, �as apod). 
K zji�t�n� pot�ebn�ch �daj� se pou��v� SNTP server ntp.nic.cz.

Popis aplikace
=================
V hlavn� funkci main aplikace nejprve nainicializuje DHCP klienta tak, abychom z�skali IP adresu v lok�ln� s�ti z DHCP serveru a nemuseli ji konfigurovat ru�n�. 
Pot� n�sleduje vol�n� funkce `udp_bind(123, sntp)`, kter� zaregistruje callback funkci pojmenovanou `sntp`, kter� bude vyvol�na p�i p��chodu UDP paketu s c�lov�m portem ��slo 123 (��slo c�lov�ho portu lze libovoln� zm�nit). 
V hlavn� smy�ce je postupn� vol�na funkce `ENC28J60_idle()` a n�sledn� testov�n stav DHCP pomoc� funkce `dhcp_ready()`.

P�i zad�n� p��kazu `GET TIME` do termin�lu se vyvol� postupn� n�sleduj�c� sekvence p��kaz�. 
Pomoc� `tx_init(UDP_PROTO)` se zinicializuje UDP paket v odes�lac�m bufferu, pot� se zap�e pomoc� vol�n� funkce `tx_write(&sntp_header, SNTP_HEADER_LEN)` SNTP paket do bufferu.
Z�pis je n�sledov�n dokon�en�m paketu vol�n�m funkce `tx_close()` a odesl�n�m UDP paketu na port ��slo 123 (zdrojov� port jsme nastavili t� na hodnotu 123). 
Odesl�n� je realizov�no pomoc� vol�n� `udp_send(123, 123, CHAR2IP(217,31,205,226))`. 
Makro CHAR2IP p�ev�d� IP adresu zadanou jako �tve�ici byt� na ��slo typu long int.
P�i p��chodu odpov�di na port ��slo 123 je na�tena odpov�� do struktury sntp_header pomoc� vol�n� funkce `rx_read` a n�sleduje zji�t�n� informac� o datu a �ase a jejich v�pis do termin�lu.

Zprovozn�n� aplikace
========================
1. p�ipojte k FITkitu roz�i�uj�c� modul Ethernet obsahuj�c� ENC28J60 a p�ipojte modul do s�t� LAN   
2. p�elo�te aplikaci
3. naprogramujte MCU a FPGA a spus�te termin�lov� program
4. jakmile aplikace obdr�� odpov�� od DHCP serveru, je odesl�n po�adavek na aktu�ln� �as

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.
