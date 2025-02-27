TARGET = library
BASE = ../../
FCMAKE = C:/Program Files/FITkit/bin/fcmake.exe
PROJECT = library.xml
OUTPUTPREFIX = libfitkit

all: build/libfitkit_f1xx.a build/libfitkit_f2xx.a

#MCU part
#=====================================================================
build/cmd_f1xx.o: cmd.c
	$(comp_tpl_f1xx)

build/cmd_f2xx.o: cmd.c
	$(comp_tpl_f2xx)

build/fitkitlib_f1xx.o: fitkitlib.c
	$(comp_tpl_f1xx)

build/fitkitlib_f2xx.o: fitkitlib.c
	$(comp_tpl_f2xx)

build/flash_f1xx.o: flash.c
	$(comp_tpl_f1xx)

build/flash_f2xx.o: flash.c
	$(comp_tpl_f2xx)

build/flash_fpga_f1xx.o: flash_fpga.c
	$(comp_tpl_f1xx)

build/flash_fpga_f2xx.o: flash_fpga.c
	$(comp_tpl_f2xx)

build/fpga_f1xx.o: fpga.c
	$(comp_tpl_f1xx)

build/fpga_f2xx.o: fpga.c
	$(comp_tpl_f2xx)

build/globfun_f1xx.o: globfun.c
	$(comp_tpl_f1xx)

build/globfun_f2xx.o: globfun.c
	$(comp_tpl_f2xx)

build/spi_f1xx.o: spi.c
	$(comp_tpl_f1xx)

build/spi_f2xx.o: spi.c
	$(comp_tpl_f2xx)

build/terminal_f1xx.o: terminal.c
	$(comp_tpl_f1xx)

build/terminal_f2xx.o: terminal.c
	$(comp_tpl_f2xx)

build/timer_b_f1xx.o: timer_b.c
	$(comp_tpl_f1xx)

build/timer_b_f2xx.o: timer_b.c
	$(comp_tpl_f2xx)

build/uart_f1xx.o: uart.c
	$(comp_tpl_f1xx)

build/uart_f2xx.o: uart.c
	$(comp_tpl_f2xx)

build/filetransfer_f1xx.o: filetransfer.c
	$(comp_tpl_f1xx)

build/filetransfer_f2xx.o: filetransfer.c
	$(comp_tpl_f2xx)

OBJFILES_F1XX = build/cmd_f1xx.o build/fitkitlib_f1xx.o build/flash_f1xx.o build/flash_fpga_f1xx.o build/fpga_f1xx.o build/globfun_f1xx.o build/spi_f1xx.o build/terminal_f1xx.o build/timer_b_f1xx.o build/uart_f1xx.o build/filetransfer_f1xx.o
OBJFILES_F2XX = build/cmd_f2xx.o build/fitkitlib_f2xx.o build/flash_f2xx.o build/flash_fpga_f2xx.o build/fpga_f2xx.o build/globfun_f2xx.o build/spi_f2xx.o build/terminal_f2xx.o build/timer_b_f2xx.o build/uart_f2xx.o build/filetransfer_f2xx.o

include $(BASE)/base/Makefile.inc
