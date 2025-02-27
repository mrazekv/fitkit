#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: set ts=4 et:
import sys
import os
import thread
import threading
import Events
import time

class Channel:
    sdata = []    
    cond=threading.Condition()
    def __init__(self, dev_id, baud_rate=460800, defaultfile=None):

        self._trx = False
        self.events = Events.Events()
        
        # Nepredavam nastaveni pres konstruktor Serial, aby nedoslo k otevreni portu.
        # Ten se otevira rucne pomoci metody open pozdeji, az se nastavi spravny timeout 
        # v zavislosti na platforme.
        self.serialdevice = None
        self.dev_id = dev_id
        
        self.textlockcnt = 0
        self.kblockcnt = 0
        self.treader = None

        # Cesta k souboru, ktery se prenese pri vyvolani pozadavku na prenos souboru
        self.afile = defaultfile

    #===================================================================================
    def isopen(self):
        return False

    def open(self):
        self.wcstate = 0
        self.escseq  = ""
        self.fkcmd   = ""

        self.startReaderThread()

    #recv se vola z vlastniho threadu, cte data ze serioveho portu a preposila pres events
    def recv(self):
        pass

    def send(self, data):
        pass

    def close(self):
        pass

    def deviceInfo(self):
        return None
    #===================================================================================
    def startReaderThread(self):
        self.treader = threading.Thread(name="Channel.treader", target=self.recv)
        self.treader.start()

    def isLocked(self, type=''):
        if (type == 'k'):
            return (self.kblockcnt > 0)
        else:
            return (self.textlockcnt > 0)
        
#TODO: vyresit lepe lock (vystupu na konzoli a vstupu z kozole), nejlepe jej nejak pridat do events
    def lock(self, type='t'):
        if (type == 'k'):
            self.kblockcnt += 1
        else:
            self.textlockcnt += 1

    def unlock(self, type='t'):
        if (type == 'k'):
            if (self.kblockcnt == 0):
                raise Exception("Lock KB");
            self.kblockcnt -= 1
        else:
            if (self.textlockcnt == 0):
                raise Exception("Lock TEXT");
            self.textlockcnt -= 1

    def dataIn_add(self, data, chan):
        self.cond.acquire() #vstup do kriticke sekce

        self.sdata.append(data)

        self.cond.notifyAll() #oznamit cekajicim zmenu sdata
        self.cond.release() 

    def dataIn_remove(self):
        self.cond.acquire() #vstup do kriticke sekce
        while (len(self.sdata) == 0): #dale jen pokud neni fronta prazdna
              self.cond.wait()
        ch = self.sdata.pop(0)
        self.cond.release() 
        return ch

    def dataIn_flush(self):
        self.cond.acquire() #vstup do kriticke sekce
        self.sdata = []
        self.cond.release() 
            
    def waitForPrompt(self):
        self.events.CharReceived += self.dataIn_add
        while (self.dataIn_remove() != '>'):
            pass
        self.events.CharReceived -= self.dataIn_add
        #pockat, nez se bude pokracovat dale
        time.sleep(.1)
    

    def _handleChar(self, ch):
        if (self.wcstate == 0):
            if (ch == '\x1b'):
                self.wcstate = 1
                self.escseq  = ""
            elif (ch == '\x01'): #STX (start command transfer)
                self.wcstate = 3
                self.fkcmd = ""
            else:
                if (ch == '\r'): return False
                #write to stdout
                return True
        elif (self.wcstate == 1):
            self.escseq += ch
            if (ch == '\x5b'):
                self.wcstate = 2
            else:
                self.wcstate = 0
        elif (self.wcstate == 2):
            self.escseq += ch
            #ANSI ESCAPE sekvence konci vzdy pismenem
            if ch.isalpha():
                #print "ESCseq:",escseq
                self.wcstate = 0
        elif (self.wcstate == 3): #command
            if (ch == '\x03'):
                self.wcstate = 0
                if not self._processCommand(self.fkcmd):
                    #send NAK (commands are not supported)
                    self.send("\x16")
                    self.wcstate = 4
            else:
                self.fkcmd += ch

        elif (self.wcstate == 4): #receive command answer
            self.wcstate = 0
            sys.stdout.write("\nINFO: Neznamy prikaz {"+self.fkcmd.split(';')[0]+"}\n")
            sys.stdout.flush()

        return False

    def _processCommand(self, cmd): #fitkit command
        cmd = [a.strip() for a in cmd.split(';')]
        par = {}
        for cp in cmd[1:]:
            cp = cp.split('=',1)
            if len(cp) == 2:
                par[cp[0].strip()] = cp[1].strip()

        if (cmd[0] == 'UPLOADFILE'):
            return self._fileTransfer(par)

        return False

        #UPLOADFILE;title=Program pro PicoBlaze (7168B);mask=*.hex;maxsize=8448

    def _fileTransfer(self, params):
        #define STX  0x01
        #define ETX  0x03
        #define SOT  0x02
        #define EOT  0x04
        #define ACK  0x06
        #define NAK  0x15        
        PACKET_SIZE = 264

        if (self.afile == None):
            sys.stdout.write("\nINFO: Neni zvolen soubor\n")
            sys.stdout.flush()
            return False

        self.lock()

        try:

            self.send("\x06") #confirm the request; send ACK
            ch = self.channel.read(1)
            if (ch != '\x06'): #ACK
                return False

            data = open(self.afile,"rb").read()
            packets = (len(data) + PACKET_SIZE - 1) // PACKET_SIZE

            for i in range(packets):
                pckt = data[i*PACKET_SIZE:(i+1)*PACKET_SIZE]
                while len(pckt) < PACKET_SIZE: pckt += '\0'
                pckt = chr(sum([ord(a) for a in pckt]) & 0xFF) + pckt

                tries = 3
                while tries:
                    self.send("\x02") #SOT
                    ch = self.channel.read(1)
                    if (ch != '\x06'): #ACK
                       return False

                    self.send(pckt)

                    sys.stdout.write(".")
                    sys.stdout.flush()

                    ch = self.channel.read(1)
                    if (ch == '\x06'): #ACK
                        tries = 0
                    elif (ch == '\x15'): #invalid checksum, try again
                        tries -= 1
                        if (tries == 0): 
                            sys.stdout.write("Timeout\n")
                            sys.stdout.flush()
                            return False
                    else:
                        return False

            print
            self.send("\x04") #EOT
            ch = self.channel.read(1)

        finally:
            self.unlock()

        return True

    def sendCmd(self, cmd):
        self.lock('k')
        try:
            if (cmd['type'] == 'cmd'):
                par = cmd['par'].rstrip()

                self.events.CharReceived += self.dataIn_add
                #print "<"+par+">"
                self.send(par+"\r\n")
                while (True):
                    ch = self.dataIn_remove()
                    if (ch == '>'): break;
                self.events.CharReceived -= self.dataIn_add

            elif (cmd['type'] == 'file'):
                self.afile = cmd['par']

        finally:
            self.unlock('k')
