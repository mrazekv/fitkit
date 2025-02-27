# -*- coding: UTF-8 -*-
# vim: set ts=4 et:
from channel import Channel
import sys
import fitkit
import time

class ChannelFTDI(Channel):
    def __init__(self, dev_id, baud_rate=460800, logoutput=None, defaultfile=None):
        Channel.__init__(self, dev_id, baud_rate, defaultfile) 

        self.devices = fitkit.pyDeviceMgr(False)
        if logoutput:
            logoutput.write("Discover devices ")
            logoutput.flush() 

        for i in range(0,5):
            ret = self.devices.discover()
            if (ret < 1):
                time.sleep(1)
                if logoutput:
                    logoutput.write(".")
                    logoutput.flush() 
        if (ret < 1):
            if logoutput:
                logoutput.write("\n")
                logoutput.flush() 
            raise Exception("No device found")

        if logoutput:
            logoutput.write("OK\n")
            logoutput.flush() 

        #acquire the device
        fk = self.devices.acquire(dev_id)
        if (fk is None):
            raise Exception("Can't open device %d" % dev_id)

        self.serialdevice = fk
        self.channel = None

    def deviceInfo(self):
        if (self.serialdevice):
            return "FTDI #%04X %s" % (self.serialdevice.vid(),self.serialdevice.channel('b').product())

    def isopen(self):
        return self.channel and self.channel.isOpen()

    def open(self):
        if (self.serialdevice):
            self.channel = self.serialdevice.channel('b')

            self.channel.open()

            #set RESET
            self.channel.setRts(False)
            self.channel.setDtr(False)

            #clear RESET
            self.channel.setRts(True)
            self.channel.setDtr(True)

            #start reader thread
            Channel.open(self)
        
    #recv se vola z vlastniho threadu, cte data ze serioveho portu a preposila
    #pres events
    def recv(self):
        while (self.channel) and (self.channel.isOpen()):
            try:
                data = self.channel.read(1)
                if (data <> None):
                   #print "R(",data,")",
               
                   #zavolat udalosti navesene na prijem znaku
                   self.events.CharReceived(data, self)
            except RuntimeError, e: #read terminated
                #print "Exception",e
                break

    def send(self, data):
        if (self.channel):
            #print "S(",data,")",
            self.channel.write(data, len(data))

    def close(self):
        #Terminate the read thread
        self.channel.close()
        self.treader.join()

        try:
            #set RESET
            self.channel.setRts(False)
            self.channel.setDtr(False)

            #self.channel.flush(True, True)
            self.channel.close()
        except KeyboardInterrupt:
            pass

        self.channel = None
        self.devices.release(self.serialdevice)
        self.serialdevice = None
