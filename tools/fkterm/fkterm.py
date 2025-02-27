#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: set ts=4 et:
import sys
import threading
import getopt
import os.path
from fkterm import terminal, ChannelFTDI

def terminal_usage():
    print "FITkit terminal v 0.8  (C) 2008 Zdenek Vasicek"
    print "----------------------------------------------------------------------"
    print "Pouziti: fkterm [-d 0] [-f file] [-h] [-s] [-c cmd]"
    print """
--devid=x, -d x  - ID cislo FITkitu pripojeneho k PC.
--file=xxx, -f xxx      - vyber souboru xxx pro prenos
--silent, -s            - po provedeni prikazu uzavre terminal
--cmd=xxx, -c xxx       - posle na terminal prikaz 'xxx', pro provedeni vice 
                          prikazu muze byt parametr opakovan
--help, -h              - vypise tuto napovedu
--noint                 - neinteraktivni rezim

Priklady:
    fkterm -d 0      
      spusti terminal a ceka na zadani prikazu

    fkterm -d 0 -c "reset mcu"
      zavola prikaz 'reset mcu' a ceka na prikaz

    fkterm -d 0 -c help
      spusti prikaz 'help' a ceka na dalsi prikazy

    fkterm -d 0 -f textura.bin -c "flash w tex" -f level.bin -c "flash w lev"
      prenese do FITkitu soubor textura.bin a dale level.bin

    fkterm -d 0 -c help -c quit
      spusti prikaz 'help' a ukonci
      terminal
"""

def terminal_exit(str):
    print "CHYBA:",str
    sys.exit(1)

def terminal_main():
    dev_id = 0        #Seriove zarizeni
    flash_fpga = False #Zda-li se ma provest naprogramovani do FLASH
    prog_fpga = False  #Zda-li se ma provest naprogramovani FPGA
    silent = False     #Zda-li se ma uzavrit po provedeni prikazu terminal
    interactive = True
    dflt_file = None
    fname = "output.bin"  # Konfiguracni soubor
    cmds  = []

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                "hd:spc:f:",["help","devid=","silent","cmd=","noint","file="])
    except getopt.GetoptError:
        terminal_usage()
        sys.exit(1) 

    if (len(args) > 0) and (args[0] <> ''):
       fname = args[0]

    for o, a in opts:
        print o, a
        if o in ("-h", "--help"):
            terminal_usage()
            sys.exit(1)
#        if o in ("-f", "--flash"):
#            flash_fpga = True
#        if o in ("-p", "--prog"):
#            prog_fpga = True
        elif o in ("-d", "--devid"):
            try:
                dev_id = int(a)
            except:
                terminal_exit('Zarizeni "'+a+'" neni podporovano')
        elif o in ("-s", "--silent"):
            silent = True
        elif o in ("-c", "--cmd"):
            if (a): cmds.append({'type':'cmd', 'par': a})
        elif o in ("--noint"):
            interactive = False
        elif o in ("-f", "--file"):
            if not os.path.isfile(a):
                terminal_exit('Soubor "'+a+'" nenalezen')
            if not dflt_file:
                dflt_file = a
            cmds.append({'type':'file', 'par': a})

    if (dev_id == -1):
        terminal_usage()
        sys.exit(1)

#    if ((flash_fpga or prog_fpga) and (not os.path.isfile(fname))):
#        terminal_exit('Konfiguracni soubor "'+fname+'" nenalezen.')

    #Vytvoreni a otevreni komunikacniho kanalu
    try:
        chan = ChannelFTDI.ChannelFTDI(dev_id, logoutput=sys.stdout, defaultfile=dflt_file)
    except Exception, msg:
        if (msg == "No device found"):
            terminal_exit("Nenalezeno zadne zarizeni")
        elif (str(msg).startswith("Can't open device")):
            terminal_exit("Nelze navazat spojeni se zarizenim %d" % dev_id)
        else:
            terminal_exit("Pripojeni skoncilo chybou <"+str(msg)+">")


    chan.lock('k') #uzamknout vstup z klavesnice

    #TODO: pro linux vynechat, terminal by mel sam umet zpracovavat ESC sekvence    
    os.system('cls')
    print "Pripojuji se k FITkitu pomoci zarizeni: ", chan.deviceInfo()
    try:
        chan.open()
    except Exception, msg:
        amsg = str(msg)[0:32]
        if (amsg == "could not open windows port: (2,"):
            terminal_exit("Uvedene zarizeni nebylo nalezeno")
        elif (amsg == "could not open windows port: (5,"):
            terminal_exit("Uvedene zarizeni pouziva jina aplikace")
        else:
            terminal_exit("Pripojeni skoncilo chybou <"+str(msg)+">")

    #Vytvoreni threadu, ktery se bude starat o komunikaci a konzoli
    if (interactive):
        t = threading.Thread(name="terminal", target=terminal.interactive_shell, args=(chan,))
    else:
        t = threading.Thread(name="terminal", target=terminal.line_shell, args=(chan,))
    t.start()

    if (flash_fpga or prog_fpga):
        chan.waitForPrompt() #cekani na prompt
        cmd = "flash w"
        if (prog_fpga): 
            cmd = "prog"
        cmd = cmd + " fpga"
        chan.fileTransmitt(fname, cmd) #prenos souboru
        chan.waitForPrompt() #cekani na prompt
    
    if (len(cmds) > 0):
        if (not flash_fpga and not prog_fpga):
            chan.waitForPrompt() #cekani na prompt

        for cmd in cmds:
            if (cmd['type'] == 'cmd') and (cmd['par'] == "quit"): 
                silent = True
            else:
                chan.sendCmd(cmd)

    chan.unlock('k') #povolit vstup z klavesnice
    if (silent):
        if (len(cmds) == 0):
            chan.waitForPrompt()
        terminal.close()

    try:
       t.join()
    except KeyboardInterrupt:
       pass

    chan.close()

if (__name__ == '__main__'):
    terminal_main()    
