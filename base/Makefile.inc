# Makefile.inc : Main makefile script
# vim: set syn=make:
# Copyright (C) 2009 Brno University of Technology,
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

#Path to file remove binary
RM ?= rm
#Path to fkflash binary
FKFLASH ?= fkflash
#Path to fkterm binary
FKTERM  ?= fkterm
#Path to fcmake binary
FCMAKE  ?= fcmake
FKTEST  ?= 
#fkflash USB device (0 = first device)
USBDEVID ?= 0 
#project file name
PROJECT ?= project.xml
TARGET  ?= project

LIBDIR  = $(BASE)/mcu/libfitkit
LIBDIR2 = $(BASE)/mcu/libs

LIBNAME_F1XX = fitkit_f1xx
LIBNAME_F2XX = fitkit_f2xx

LIBFITKIT_F1XX = $(LIBDIR)/build/lib$(LIBNAME_F1XX).a
LIBFITKIT_F2XX = $(LIBDIR)/build/lib$(LIBNAME_F2XX).a

.PHONY: load forceload term clean purge
.SILENT: clean cleanmcu purge

term:
	"$(FKTERM)" --devid=$(USBDEVID)

clean:
	"$(FCMAKE)" --cleanall --generate $(PROJECT)

cleanmcu:
	"$(FCMAKE)" --cleanmcu

purge:
	"$(FCMAKE)" --cleanall $(PROJECT)

PKGS_LIST += $(PROJECT)

Makefile: $(PKGS_LIST)
	"$(FCMAKE)" $(PROJECT)

load:	$(HEXFILE_F1XX) $(HEXFILE_F2XX) $(BINFILE)
	$(info *****************************************************************************)
	$(info * FITKIT PROGRAMMING                                                       *)
	$(info *****************************************************************************)
	"$(FKFLASH)" --devid=$(USBDEVID) --hexfile="$(HEXFILE_F1XX)" --hexfile2="$(HEXFILE_F2XX)" --binfile="$(BINFILE)"

forceload:	$(HEXFILE_F1XX) $(HEXFILE_F2XX) $(BINFILE)
	$(info *****************************************************************************)
	$(info * FITKIT PROGRAMMING                                                       *)
	$(info *****************************************************************************)
	"$(FKFLASH)" --devid=$(USBDEVID) --force --hexfile="$(HEXFILE_F1XX)" --hexfile2="$(HEXFILE_F2XX)" --binfile="$(BINFILE)"

#======================================================================
# MCU part
#======================================================================
CC      = msp430-gcc

CPU_F1XX    ?= msp430x168
CPU_F2XX    ?= msp430x2617

ASMOPT_F1XX  = -mmcu=$(CPU_F1XX)
ASMOPT_F2XX  = -mmcu=$(CPU_F2XX)

INCLUDE_ARGS += -I$(LIBDIR) -I$(LIBDIR2)

COPT_F1XX    = $(ASMOPT_F1XX) -DFITKIT_VERSION=1 -Wall $(INCLUDE_ARGS)
COPT_F2XX    = $(ASMOPT_F2XX) -DFITKIT_VERSION=2 -Wall $(INCLUDE_ARGS)
#COPT    = -Wall -I$(LIBDIR) -O2
#LDOPT   = -Wl,-Map=$(basename $@).map -L$(LIBDIR)/build
LDOPT   = -Wl,-Map=build/mcu/$(basename $(@F)).map -L$(LIBDIR)/build

FIRST_F1XX=yes
FIRST_DEPINFO=yes
define comp_tpl_f1xx
	$(if $(DEPPROJ),
		$(if $(FIRST_DEPINFO), 
			$(info Info: project $(DEPPROJ) depends on project $(OUTPUTPREFIX) which has been changed)
			$(eval FIRST_DEPINFO=)
		,)
	,)
	$(if $(FIRST_F1XX),
		$(info *****************************************************************************)
		$(info * COMPILE $(TARGET) $(OUTPUTPREFIX) [C -> OBJ -> A] - MSP430F1XX)
		$(info *****************************************************************************)
		$(eval FIRST_F1XX=)
	,)
	$(CC) $(COPT_F1XX) -o $@ -c $< 
endef

FIRST_F2XX=yes
define comp_tpl_f2xx
	$(if $(DEPPROJ),
		$(if $(FIRST_DEPINFO), 
			$(info Info: project $(DEPPROJ) depends on project $(OUTPUTPREFIX) which has been changed)
			$(eval FIRST_DEPINFO=)
		,)
	,)
	$(if $(FIRST_F2XX),
		$(info *****************************************************************************)
		$(info * COMPILE $(TARGET) $(OUTPUTPREFIX) [C -> OBJ -> A] - MSP430F2XX)
		$(info *****************************************************************************)
		$(eval FIRST_F2XX=)
	,)
	$(CC) $(COPT_F2XX) -o $@ -c $< 
endef

%_f1xx.a %_f1xx.nm:	$(OBJFILES_F1XX)
	$(RM) $@
	msp430-ar rcv $@ $(OBJFILES_F1XX) 
	msp430-ranlib $@
	msp430-nm --demangle $@ > $(basename $@).nm

%_f2xx.a %_f2xx.nm:	$(OBJFILES_F2XX)
	$(RM) $@
	msp430-ar rcv $@ $(OBJFILES_F2XX) 
	msp430-ranlib $@
	msp430-nm --demangle $@ > $(basename $@).nm 

%_f1xx.elf:	$(LIBFITKIT_F1XX) $(OBJFILES_F1XX)
	$(CC) $(ASMOPT_F1XX) $(LDOPT) -o $@ $(OBJFILES_F1XX) -l$(LIBNAME_F1XX) $(LIBRARIES)

%_f2xx.elf:	$(LIBFITKIT_F2XX) $(OBJFILES_F2XX)
	$(CC) $(ASMOPT_F2XX) $(LDOPT) -o $@ $(OBJFILES_F2XX) -l$(LIBNAME_F2XX) $(LIBRARIES)

# disassembled output
%.lst: %.elf
	$(info *****************************************************************************)
	$(info * DISASSEMBLING $< [ELF -> LST])
	$(info *****************************************************************************)
	msp430-objdump -DS $< > $@ 
	
# file for programming
%.hex: %.elf
	$(info *****************************************************************************)
	$(info * CONVERTING ELF FILE $< [ELF -> HEX])
	$(info *****************************************************************************)
	msp430-objcopy -O ihex $< $@ 
		
$(LIBFITKIT_F2XX):
	"$(MAKE)" -e -C $(LIBDIR)

$(LIBFITKIT_F1XX):
	"$(MAKE)" -e -C $(LIBDIR)

.PHONY: cleanlib buildlib
.SILENT: buildlib cleanlib

cleanlib:
	"$(MAKE)" -e -C $(LIBDIR) --no-print-directory clean

buildlib:	
	"$(MAKE)" -e -C $(LIBDIR) --no-print-directory

#======================================================================
# FPGA part
#======================================================================

TOPLEVELENTITY ?= fpga 

FPGACHIP ?= xc3s50
FPGASPEEDGRADE ?= 4
FPGAPACKAGE ?= pq208
FPGACHIPID ?= $(FPGACHIP)-$(FPGASPEEDGRADE)-$(FPGAPACKAGE) 

FPGAUCF ?= $(BASE)/fpga/chips/fpga_xc3s50.ucf
NGDBUILD ?= ngdbuild
NGDBUILDFLAGS ?= 
ifdef NGCPATH
  NGDBUILDFLAGS += -sd "$(NGCPATH)"
endif

SYNTH ?= xst
SYNTHFLAGS ?=   

MAP ?= map
MAPFLAGS ?=

PAR ?= par
PARFLAGS ?= -w -ol std

XTCLSH ?= xtclsh
ISEBIN ?= ise

BITGEN ?= bitgen
BITGENFLAGS ?= -w -g UnusedPin:PullNone -g Binary:Yes -g Compress

NETGEN ?= netgen
NETGENFLAGS ?= -intstyle ise -s $(FPGASPEEDGRADE) -rpw 100 -tpw 0 -ar timesim -a -insert_pp_buffers false -w -ofmt vhdl
NETGENFLAGSSYNTH ?= -intstyle ise -a -ar timesim -aka -w -ofmt vhdl

TRCGEN ?= trce
TRCGENFLAGS ?= -e 5 -l 1 -u 10 -s 4 # output.par.ncd output.pcf  

#XSTOPTFLAGS ?= -opt_mode SPEED -opt_level 1

.PRECIOUS: %.par.ncd %.map.ncd

%.ngc %.srp %.ngr:	%.prj %.xst %_config.vhd $(HDLFILES)
	$(info *****************************************************************************)
	$(info * SYNTHESIZE [VHDL -> EDIF]                                                 *)
	$(info *****************************************************************************)
	$(if $(dir $*.xst), $(RM) -rf $(dir $*.xst)work, )
	$(SYNTH) $(SYNTHFLAGS) -ifn $*.xst -ofn $*.srp && $(RM) -rf xilinx_device_details.xml xlnx_auto*


#=======================================================================================
%.ngd %.bld:	%.ngc $(FPGAUCF)
	$(info *****************************************************************************)
	$(info * NGDBUILD [EDIF -> NGD]                                                    *)
	$(info *      NGD ... Native Generic Database                                      *)
	$(info *****************************************************************************)
	$(NGDBUILD) $(NGDBUILDFLAGS) -uc "$(FPGAUCF)" $< $@ && $(RM) -rf netlist.lst xlnx_auto*

%.map.ncd %.map.ngm %.pcf %.map.mrp:	%.ngd
	$(info *****************************************************************************)
	$(info * MAP [NGD -> MAP.NCD, MAP.NGM, PCF]                                        *)
	$(info *      NCD ... Native Circuit Description                                   *)
	$(info *      NGM ... Structure, naming, hierarchy                                 *)
	$(info *      PCF ... Physical Constraints File                                    *)
	$(info *****************************************************************************)
	$(MAP) $(MAPFLAGS) -o $*.map.ncd $< $*.pcf && $(RM) -rf xilinx_device_details.xml fpga_map.xrpt xlnx_auto*

%.par.ncd %.par.pad %.par.par %.par.unroutes %.par.xpi %.par_pad.csv %.par_pad.txt:	%.map.ncd %.pcf
	$(info *****************************************************************************)
	$(info * PLACE & ROUTE [MAP.NCD, PCF -> PAR.NCD]                                   *)
	$(info *****************************************************************************)
	$(PAR) $(PARFLAGS) $< $@ $*.pcf && $(RM) -rf xilinx_device_details.xml fpga_par.xrpt xlnx_auto*
	$(if $(FKTEST), "$(FKTEST)" $*.par.par "WARNING.*design did not meet timing" || $(RM) -rf $*.par.ncd,)

%.bin %.bit %.bgn %.drc:	%.par.ncd %.pcf
	$(info *****************************************************************************)
	$(info * BITGEN [PAR.NCD, PCF -> BIT, BIN]                                         *)
	$(info *****************************************************************************)
	$(BITGEN) $(BITGENFLAGS) $< $*.bit $*.pcf && $(RM) -rf xilinx_device_details.xml xlnx_auto*


build/%.bin:	build/fpga/%.par.ncd build/fpga/%.pcf
	$(info *****************************************************************************)
	$(info * BITGEN [PAR.NCD, PCF -> BIT, BIN]                                         *)
	$(info *****************************************************************************)
	$(BITGEN) $(BITGENFLAGS) $< $(basename $@).bit $(word 2,$^) && $(RM) -rf xilinx_device_details.xml xlnx_auto* $(basename $@).bit $(basename $@).bgn $(basename $@).drc
		
%.timesim.vhd %.timesim.sdf:	%.pcf %.par.ncd
	$(info *****************************************************************************)
	$(info * POST PLACE & ROUTE SIMULATION MODEL [PCF, MAP.NCD -> TIMESIM.VHD]         *)
	$(info *****************************************************************************)
	$(NETGEN) $(NETGENFLAGS) -pcf $*.pcf -sim $*.par.ncd $*.timesim.vhd && $(RM) -rf xilinx_device_details.xml xlnx_auto*

%.postsynth.vhd %.postsynth.nlf:	%.ngc
	$(info *****************************************************************************)
	$(info * POST SYNTHESIS SIMULATION MODEL [NGC -> POSTSYNTH.VHD]                    *)
	$(info *****************************************************************************)
	$(NETGEN) $(NETGENFLAGSSYNTH) -sim $*.ngc $*.postsynth.vhd && $(RM) -rf xilinx_device_details.xml xlnx_auto*

.PHONY:  cleanfpga synth rtl sim ise simmodel simmodel.par simmodel.synth

simmodel:	build/fpga/$(OUTPUTPREFIX).timesim.vhd 
simmodel.par:	build/fpga/$(OUTPUTPREFIX).timesim.vhd 
simmodel.synth:	build/fpga/$(OUTPUTPREFIX).postsynth.vhd 

synth:	build/fpga/$(OUTPUTPREFIX).ngc

rtl:	build/fpga/$(OUTPUTPREFIX).ngr

sim:	fpga/sim/sim.fdo
	$(info *****************************************************************************)
	$(info * SIMULATION [MODELSIM]                                                     *)
	$(info *****************************************************************************)
	cd fpga/sim && vsim -gui -keepstdout -t ps -l modelsim_log.txt -do sim.fdo

isim:	build/fpga/$(OUTPUTPREFIX)_sim.tcl
	$(info *****************************************************************************)
	$(info * SIMULATION [ISIM]                                                         *)
	$(info *****************************************************************************)
	cd build/fpga && $(XTCLSH) $(OUTPUTPREFIX)_sim.tcl

%_project.ise:	%_ise.tcl
	$(info *****************************************************************************)
	$(info * CREATING ISE PROJECT                                                      *)
	$(info *****************************************************************************)
	cd $(@D) && $(XTCLSH) $(*F)_ise.tcl

%_project.xise:	%_ise.tcl
	$(info *****************************************************************************)
	$(info * CREATING ISE PROJECT [ISE > 12.1]                                         *)
	$(info *****************************************************************************)
	cd $(@D) && $(XTCLSH) $(*F)_ise.tcl

ifeq ($(ISE),runise)
ise:	build/fpga/$(OUTPUTPREFIX)_project.$(shell $(XTCLSH) $(BASE)/base/xilinxiseext.tcl $(BASE)/base/)
	cd $(<D) && $(ISEBIN) $(<F)
else
ise:	build/fpga
	@"$(MAKE)" ISE=runise ise
endif

FPGASFX = srp ngc ngr blf map.ncd map.ngm map.mrp pcf par.ncd par.pad par.par par.unroutes par.xpi par_pad.csv par_pad.txt bin bit bgn bld drc

cleanfpga: 
	"$(FCMAKE)" --cleanfpga
	$(RM) -rf xilinx_device_details.xml xlnx_auto* smartpreview.twr

#======================================================================
# Project dependency check
#======================================================================
.PHONY: dependencycheck deperror
.SILENT: dependencycheck

deperror:
	$(info *****************************************************************************)
	$(info ERROR: directory $(ERRDIR) doesn't contain Makefile)
	$(info *****************************************************************************)
	
define check_template
	"$(MAKE)" DEPPROJ="$(OUTPUTPREFIX)" --no-print-directory -C "$(file)" || "$(MAKE)" -s deperror ERRDIR="$(file)"

endef

THEDEPENDENCIES = $(LIBDIR) $(DEPENDENCIES)

dependencycheck: 
	$(foreach file,$(THEDEPENDENCIES),$(check_template))

