# Remote.inc : Remote makefile script
# vim: set syn=make:
# Copyright (C) 2011 Brno University of Technology,
#                    Faculty of Information Technology
# Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
# 
# LICENSE TERMS
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. All advertising materials mentioning features or use of this software
#    or firmware must display the following acknowledgement: 
#
#      This product includes software developed by the University of
#      Technology, Faculty of Information Technology, Brno and its 
#      contributors. 
#
# 4. Neither the name of the Company nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
# 
# This software or firmware is provided ``as is'', and any express or implied
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose are disclaimed.
# In no event shall the company or contributors be liable for any
# direct, indirect, incidental, special, exemplary, or consequential
# damages (including, but not limited to, procurement of substitute
# goods or services; loss of use, data, or profits; or business
# interruption) however caused and on any theory of liability, whether
# in contract, strict liability, or tort (including negligence or
# otherwise) arising in any way out of the use of this software, even
# if advised of the possibility of such damage.
# 
# $Id$
# 
#

FKFLASH ?= fkflash
#Path to fkterm binary
FKTERM  ?= fkterm
#Path to fcmake binary
FCMAKE  ?= fcmake 
#fkflash USB device (0 = first device)
USBDEVID ?= 0  
#MCU source files
MCUFILES ?= 
#FPGA source files
HDLFILES ?= 

.PHONY: mcu synth simmodel rtl clean cleanall load forceload term

all:
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) all
synth:
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) synth
simmodel:
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) simmodel
rtl:
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) rtl
mcu:
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) "$(HEXFILE_F1XX) $(HEXFILE_F2XX)"
clean:
	$(FCMAKE) -c
cleanall:
	$(FCMAKE) -a

$(HEXFILE_F1XX): $(MCUFILES)	
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) all

$(HEXFILE_F2XX): $(MCUFILES)
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) all

$(BINFILE): $(HDLFILES)
	$(FCMAKE) $(FCMAKEFLG) -n $(HOST) all
	

load:	$(HEXFILE_F1XX) $(HEXFILE_F2XX) $(BINFILE)
	$(info *****************************************************************************)
	$(info * FIKTKIT PROGRAMMING                                                       *)
	$(info *****************************************************************************)
	"$(FKFLASH)" --devid=$(USBDEVID) --hexfile="$(HEXFILE_F1XX)" --hexfile2="$(HEXFILE_F2XX)" --binfile="$(BINFILE)"

forceload:	$(HEXFILE_F1XX) $(HEXFILE_F2XX) $(BINFILE)
	$(info *****************************************************************************)
	$(info * FIKTKIT PROGRAMMING                                                       *)
	$(info *****************************************************************************)
	"$(FKFLASH)" --devid=$(USBDEVID) --force --hexfile="$(HEXFILE_F1XX)" --hexfile2="$(HEXFILE_F2XX)" --binfile="$(BINFILE)"

term:
	"$(FKTERM)" --devid=$(USBDEVID)
 
	
