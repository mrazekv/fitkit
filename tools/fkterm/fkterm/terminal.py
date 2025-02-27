# -*- coding: UTF-8 -*-
# Copyright (C) 2003-2005 Robey Pointer <robey@lag.net>
#                         Zdenek Vasicek <xvasic11@fit.vutbr.cz>
# vim: set ts=4 et enc=utf8:
#
# This file is part of paramiko.
#
# Paramiko is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# Paramiko is distrubuted in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Paramiko; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.


#import socket
import os.path
import sys
import time
import xmodem

# windows does not have termios...
try:
    import termios
    import tty
    has_termios = True
except ImportError:
    has_termios = False

#s=''

terminal_close = False
def close():
    global terminal_close
    terminal_close = True

def interactive_shell(chan):
    if has_termios:
        posix_shell(chan)
    else:
        windows_shell(chan)

def posix_shell(chan):
    global terminal_close
    import select

    #chan.settimeout(0)  
    #chan.open()  

    sys.stdout.write("Ukonceni terminalu: ^Z nebo ^D. Pripadne prikazem quit\r\n\r\n")
    oldtty = termios.tcgetattr(sys.stdin)
    try:
        tty.setraw(sys.stdin.fileno())
        tty.setcbreak(sys.stdin.fileno())
        preneseno = False
        while (not terminal_close):
            r, w, e = select.select([chan, sys.stdin], [] ,[])
            if chan in r:     # dorazila data z FITkitu, vytisknou se na stdout
                try:
                    x = chan.recv(1024)
                    #if len(x) == 0:
                    #    print '\r\n*** EOF\r\n',
                    #    break
                    #if x[len(x)-1] == '>' and not preneseno:
                    #    chan.fileTransmitt('output.bin','flash w fpga')
                    #    preneseno=True
                    sys.stdout.write(x)
                    sys.stdout.flush()

                except socket.timeout:
                    sys.stdout.write("timeout\r\n")
            if sys.stdin in r:     # v terminálu byla stisknuta klávesa
                x = sys.stdin.read(1)
                if len(x) == 0:
                    break
                
                # některé funkční klávesy se musí pro FITkit překládat:
                # ENTER: očekává se \n, přichází \r
                if ord(x) == 13:
                    x=chr(10)
                # BACKSPACE: očekává se 8, přichází 127
                if ord(x) == 127:
                    x=chr(8)
                # CTRL^D: ukončení terminálu
                if ord(x) == 4:
                    print "\n"
                    break
                # TODO: odchytávat šipky
                # ESCAPE sekvence: 033
                #print ord(x)
                chan.send(x)

    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, oldtty)

def writechar(ch, chan):
    if (chan.isLocked()):
       return

    if (chan._handleChar(ch)):
       sys.stdout.write(ch)
       sys.stdout.flush()
        

# thanks to Mike Looijmans for this code
def windows_shell(chan):
    global terminal_close
    import msvcrt
    
    sys.stdout.write("Ukonceni terminalu: ^D pripadne prikazem quit\r\n\r\n")

    s = ""
    chan.events.CharReceived += writechar
    try:
#        global s
        while (not terminal_close):
            if (chan.isLocked('k')):
                continue

            if (not msvcrt.kbhit()):
               time.sleep(0.05)
               continue

            d = msvcrt.getch()
            if (d == '\x04'): #^D
                break

            if (d == '\x14'): #^T
                if (os.path.isfile('./output.bin')):
                    sys.stdout.write('\n[[[xmodem]]] ...\n')
                    sys.stdout.flush()
                    xmodem.transmitt_file(chan,'./output.bin','')
                s = ""
            elif ((d == '\n') or (d == '\r')):
                if (s == 'quit'):
                    break
                s = ""
            else:
                if (d == '\x08'): #back space - umazat znak na konci bufferu
                    if (len(s) > 0):
                        s = s[0:len(s)-1]
                        #sys.stdout.write('\x08 \x08')
                else:
                    s = s + d

            #Na FK neposilat ridici znaky (pouze CR, LF a backspace)
            if ((ord(d) > 31) or (d == '\r') or (d == '\n') or (d == '\x08')):
                if chan.isopen():
                    if (d == '\r'): d = '\n'
                    chan.send(d)
                else:
                    return

    except EOFError:
        # user hit ^Z or F6
        pass

    chan.events.CharReceived -= writechar


def line_shell(chan):
    global terminal_close
    
    sys.stdout.write("Ukonceni terminalu: prikazem quit\r\n\r\n")

    s = ""
    chan.events.CharReceived += writechar
    try:
        while (not terminal_close):
            if (chan.isLocked('k')):
                continue

            d = sys.stdin.read(1)

            if ((d == '\n') or (d == '\r')):
                if (s == 'quit'):
                    break
                s = ""
            else:
                s = s + d

            #Na FK neposilat ridici znaky (pouze CR, LF a backspace)
            if ((ord(d) > 31) or (d == '\r') or (d == '\n') or (d == '\x08')):
                if chan.isopen():
                    if (d == '\r'): d = '\n'
                    chan.send(d)
                else:
                    return

    except EOFError:
        # user hit ^Z or F6
        pass

    chan.events.CharReceived -= writechar

if (__name__ == '__main__'):
    import test

