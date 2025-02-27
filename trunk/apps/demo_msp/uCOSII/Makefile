BASE = ../../../
FCMAKE = C:/Program Files/FITkit/bin/fcmake.exe
FKFLASH = C:/Program Files/FITkit/bin/fkflash.exe
PROJECT = project.xml
OUTPUTPREFIX = ucosii
FPGACHIP = xc3s50
FPGASPEEDGRADE = 4
FPGAPACKAGE = pq208

all: dependencycheck build/ucosii_f1xx.hex build/ucosii_f2xx.hex build/ucosii.bin

#MCU part
#=====================================================================
HEXFILE_F1XX = build/ucosii_f1xx.hex
HEXFILE_F2XX = build/ucosii_f2xx.hex

INCLUDE_ARGS = -Isrc/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430 -Isrc/Micrium/Software/uCOS-II/Source -Isrc/app

build/mcu/os_cpu_c_f1xx.o: src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_c.c src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cfg.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.h
	$(comp_tpl_f1xx)

build/mcu/os_cpu_c_f2xx.o: src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_c.c src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cfg.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.h
	$(comp_tpl_f2xx)

build/mcu/os_cpu_a_f1xx.o: src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.s src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cfg.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.h
	$(comp_tpl_f1xx)

build/mcu/os_cpu_a_f2xx.o: src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.s src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cfg.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu.h src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/os_cpu_a.h
	$(comp_tpl_f2xx)

build/mcu/os_core_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_core.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_core_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_core.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_flag_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_flag.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_flag_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_flag.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_mbox_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_mbox.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_mbox_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_mbox.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_mem_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_mem.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_mem_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_mem.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_mutex_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_mutex.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_mutex_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_mutex.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_q_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_q.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_q_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_q.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_sem_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_sem.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_sem_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_sem.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_task_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_task.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_task_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_task.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_time_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_time.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_time_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_time.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/os_tmr_f1xx.o: src/Micrium/Software/uCOS-II/Source/os_tmr.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f1xx)

build/mcu/os_tmr_f2xx.o: src/Micrium/Software/uCOS-II/Source/os_tmr.c src/Micrium/Software/uCOS-II/Source/ucos_ii.h
	$(comp_tpl_f2xx)

build/mcu/keyboard_f1xx.o: ../../../mcu/libs/keyboard/keyboard.c
	$(comp_tpl_f1xx)

build/mcu/keyboard_f2xx.o: ../../../mcu/libs/keyboard/keyboard.c
	$(comp_tpl_f2xx)

build/mcu/display_f1xx.o: ../../../mcu/libs/lcd/display.c
	$(comp_tpl_f1xx)

build/mcu/display_f2xx.o: ../../../mcu/libs/lcd/display.c
	$(comp_tpl_f2xx)

build/mcu/app_f1xx.o: src/app/app.c src/app/includes.h src/app/app_cfg.h
	$(comp_tpl_f1xx)

build/mcu/app_f2xx.o: src/app/app.c src/app/includes.h src/app/app_cfg.h
	$(comp_tpl_f2xx)

OBJFILES_F1XX = build/mcu/os_cpu_c_f1xx.o build/mcu/os_cpu_a_f1xx.o build/mcu/os_core_f1xx.o build/mcu/os_flag_f1xx.o build/mcu/os_mbox_f1xx.o build/mcu/os_mem_f1xx.o build/mcu/os_mutex_f1xx.o build/mcu/os_q_f1xx.o build/mcu/os_sem_f1xx.o build/mcu/os_task_f1xx.o build/mcu/os_time_f1xx.o build/mcu/os_tmr_f1xx.o build/mcu/keyboard_f1xx.o build/mcu/display_f1xx.o build/mcu/app_f1xx.o
OBJFILES_F2XX = build/mcu/os_cpu_c_f2xx.o build/mcu/os_cpu_a_f2xx.o build/mcu/os_core_f2xx.o build/mcu/os_flag_f2xx.o build/mcu/os_mbox_f2xx.o build/mcu/os_mem_f2xx.o build/mcu/os_mutex_f2xx.o build/mcu/os_q_f2xx.o build/mcu/os_sem_f2xx.o build/mcu/os_task_f2xx.o build/mcu/os_time_f2xx.o build/mcu/os_tmr_f2xx.o build/mcu/keyboard_f2xx.o build/mcu/display_f2xx.o build/mcu/app_f2xx.o

#FPGA part
#=====================================================================
BINFILE = build/ucosii.bin
HDLFILES  = ../../../fpga/units/clkgen/clkgen_config.vhd
HDLFILES += ../../../fpga/units/clkgen/clkgen.vhd
HDLFILES += ../../../fpga/units/math/math_pack.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_entity.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_adc_autoincr.vhd
HDLFILES += ../../../fpga/ctrls/spi/spi_ctrl.vhd
HDLFILES += ../../../fpga/chips/fpga_xc3s50.vhd
HDLFILES += ../../../fpga/chips/architecture_bare/arch_bare_ifc.vhd
HDLFILES += ../../../fpga/chips/architecture_bare/tlv_bare_ifc.vhd
HDLFILES += ../../../fpga/ctrls/lcd/lcd_raw.vhd
HDLFILES += ../../../fpga/ctrls/keyboard/keyboard_ctrl.vhd
HDLFILES += ../../../fpga/ctrls/keyboard/keyboard_ctrl_high.vhd
HDLFILES += fpga/top_level.vhd

build/ucosii.bin: build/fpga/ucosii.par.ncd build/fpga/ucosii.pcf

PKGS_LIST = src/Micrium/Software/uCOS-II/Ports/MSP430/MSPGCC/MSP430/msp_specific.xml
PKGS_LIST += src/Micrium/Software/uCOS-II/Source/os_source.xml
PKGS_LIST += ../../../mcu/libs/keyboard/package.xml
PKGS_LIST += ../../../mcu/libs/lcd/package.xml
PKGS_LIST += ../../../fpga/units/clkgen/package.xml
PKGS_LIST += ../../../fpga/units/math/package.xml
PKGS_LIST += ../../../fpga/ctrls/spi/package.xml
PKGS_LIST += ../../../fpga/chips/architecture_bare/package.xml
PKGS_LIST += ../../../fpga/ctrls/lcd/package.xml
PKGS_LIST += ../../../fpga/ctrls/keyboard/package.xml

include $(BASE)/base/Makefile.inc
