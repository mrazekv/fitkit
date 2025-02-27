.. article:: apps_modeth_http_server
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Aplikace ukazuje jak je mo�n� vyrobit jednoduch� webov� server.

========================
Webov� server
========================
Tato �loha ukazuje, jak lze vyu��t knihovnu libenc28j60 tak, abychom na FITkitu vytvo�ili jednoduch� HTTP server, kter� obsahuje dv� www str�nky. 
Str�nku index.htm, kter� obsahuje z�kladn� informace a odkaz na str�nku teplota.htm, str�nka teplota.htm zobrazuje aktu�ln� stav intern�ho teplotn�ho �idla.

Webov� str�nky jsou p�en�eny pomoc� pom�rn� jednoduch�ho textov�ho protokolu ozna�ovan�ho jako HTTP (Hyper Text Transfer Protocol).
Sta�en� webov� str�nky prob�h� n�sledovn�. 
Nejprve je ustaveno TCP spojen� na HTTP server, kter� b�� obvykle na portu ��slo 80. 
Pot� je odesl�n tzv. **GET** request, kter� spo��v� v odesl�n� textu nap�. ``GET /index.htm HTTP/1.1`` n�sledovan�ho dv�my od��dkov�n�mi (CR LF). 
Server odpov� a vr�t� str�nku index.htm nach�zej�c� se v ko�enov�m adres��i. Pokud str�nka neexistuje, vr�t� chybu.
Odpov�� se skl�d� ze dvou ��sti, hlavi�ky a  vlastn�ch dat. Hlavi�ka obsahuje informace jako je stavov� k�d, datum posledn� modifikace, apod.

Popis aplikace
=================
Ve funkci ``main()`` je nainicializov�n DHCP klient tak, abychom z�skali IP adresu z DHCP serveru a nemuseli ji konfigurovat ru�n�. 
V pozn�mce je uveden k�d umo��uj�c� t� nastavit statickou IP adresu. 
N�sleduje vol�n� funkce ``create_tcp_socket(80,tcp_handler)`` a ``tcp_bind()``, kter� zaregistruje callback funkci pojmenovanou ``tcp_handler``, kter� bude vyvol�na p�i p��chodu TCP paketu s c�lov�m portem ��slo 80. 
Funkce ``tcp_hadler`` dost�v� p��mo data, kter� jsou p�en�ena v TCP paketu; o ustaven� TCP spojen�, jeho ukon�en� a potvrzov�n� paket� se star� navr�en� knihovna. 
Pro zjednodu�en� rutina ``tcp_handler`` obsahuje kr�tk� buffer a prov�d� jednoduch� porovn�n� po�adavku pomoc� funkce strcmp. 

V hlavn� smy�ce je postupn� vol�na funkce ``ENC28J60_idle()`` a n�sledn� testov�n stav DHCP a TCP socketu pomoc� funkce ``tcp_connected()``.

Zprovozn�n� aplikace
========================
1. p�ipojte k FITkitu roz�i�uj�c� modul Ethernet obsahuj�c� ENC28J60 a p�ipojte modul do s�t� LAN   
2. p�elo�te aplikaci
3. naprogramujte MCU a FPGA a spus�te termin�lov� program.
4. jakmile aplikace obdr�� odpov�� od DHCP serveru, je mo�n� tuto IP adresu zadat v prohl�e�i

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.
