-- Top.vhd : keyboard
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek   vasicek@fit.vutbr.cz
--
-- LICENSE TERMS
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
-- 1. Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in
--    the documentation and/or other materials provided with the
--    distribution.
-- 3. All advertising materials mentioning features or use of this software
--    or firmware must display the following acknowledgement:
--
--      This product includes software developed by the University of
--      Technology, Faculty of Information Technology, Brno and its
--      contributors.
--
-- 4. Neither the name of the Company nor the names of its contributors
--    may be used to endorse or promote products derived from this
--    software without specific prior written permission.
--
-- This software or firmware is provided ``as is'', and any express or implied
-- warranties, including, but not limited to, the implied warranties of
-- merchantability and fitness for a particular purpose are disclaimed.
-- In no event shall the company or contributors be liable for any
-- direct, indirect, incidental, special, exemplary, or consequential
-- damages (including, but not limited to, procurement of substitute
-- goods or services; loss of use, data, or profits; or business
-- interruption) however caused and on any theory of liability, whether
-- in contract, strict liability, or tort (including negligence or
-- otherwise) arising in any way out of the use of this software, even
-- if advised of the possibility of such damage.
--
-- $Id$
--
--

library IEEE;
use ieee.std_logic_1164.ALL;
use IEEE.numeric_std.all;
use ieee.std_logic_ARITH.ALL;
use ieee.std_logic_UNSIGNED.ALL;
use work.clkgen_cfg.all;
use work.serial_cfg.ALL;

architecture main of tlv_bare_ifc is
    
   -- interrupt
   signal int_data_out  : std_logic_vector ( 7 downto 0);
   signal int_data_in   : std_logic_vector ( 7 downto 0);
   signal int_write_en  : std_logic;
   signal int_read_en   : std_logic;
   signal interrupt     : std_logic_vector ( 7 downto 0);
   signal interrupt_out : std_logic;
   
   --signaly pro komunikaci se seriovym rozhranim
   signal serial_data_out: std_logic_vector ( 7 downto 0);
   signal serial_data_in : std_logic_vector ( 7 downto 0);
   signal serial_write_en: std_logic;
   signal serial_read_en : std_logic;
   signal serial_vld     : std_logic;
   signal serial_busy    : std_logic;

   signal ser_data_out  : std_logic_vector ( 7 downto 0);
   signal ser_data_in   : std_logic_vector ( 7 downto 0);
   signal ser_write_en  : std_logic;
   signal ser_read_en   : std_logic;

   signal led   : std_logic := '1';

begin

   -- SPI dekoder pro radic preruseni
   SPI_adc_INT: entity work.SPI_adc
      generic map(
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 8,     -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#80# -- adresovy prostor 0x0080
      )
      port map(
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => int_data_out,
         DATA_IN  => int_data_in,
         WRITE_EN => int_write_en,
         READ_EN  => int_read_en
      );

   -- Radic preruseni
   IRQctrl: entity work.interrupt_controller
      port map(
         CLK    =>  CLK,
         RST    =>  RESET,

         IRQ_IN  => interrupt,
         IRQ_OUT => interrupt_out,   -- celkove vystupni preruseni

         -- vystupni vektor preruseni
         DATA_OUT    => int_data_in,
         READ_EN     => int_read_en,

         -- nastaveni masky preruseni
         DATA_IN     => int_data_out,
         WRITE_EN    => int_write_en
      ); 

   -- vystupni interrupt se nahodi jen pokud neprobiha prenos na sbernici SPI
   IRQ <= interrupt_out and not SPI_CS;  

   -- nevyuzite zdroje preruseni uzemnime
   interrupt(7 downto 1) <= (others => '0');
   interrupt(0) <= serial_vld;

   -- SPI dekoder pro registr seriove linky
   SPI_adc_serial: entity work.SPI_adc
      generic map(
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 8,     -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#90#)
      port map(
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => ser_data_out,
         WRITE_EN => ser_write_en,

         DATA_IN  => ser_data_in,
         READ_EN  => open
      );

   LEDF <= led;

   we_sync: entity work.clk_sync
      port map(
        RESET => RESET,
        CLK_FAST => CLK,
        CLK_SLOW => SMCLK,

        FAST_IN  => ser_write_en,
        SLOW_OUT => serial_write_en
      );
 
   dout_reg: process (CLK)
   begin
      if (CLK='1') and (CLK'event) then
         if (ser_write_en = '1') then
            led <= not led;
            serial_data_in <= ser_data_out;
         end if;
      end if;
   end process;

   din_reg: process (SMCLK)
   begin
      if (SMCLK='1') and (SMCLK'event) then
         if (serial_vld = '1') then
            ser_data_in <= serial_data_out;
         end if;
      end if;
   end process;

   -- radic seriove linky
   serial_tr: entity work.serial_transceiver
      generic map (
         SPEED     => s57600Bd,
         DATAWIDTH => 8,
         STOPBITS  => 1,
         PARITY    => sParityOdd
      )
      port map(
         RESET    => RESET,
         CLK      => SMCLK,
         
         -- cteni dat prijatych po seriove lince
         DATA_OUT => serial_data_out,
         DATA_VLD => serial_vld,

         -- pro vysilani dat na RS232
         DATA_IN  => serial_data_in,
         WRITE_EN => serial_write_en,

         BUSY     => serial_busy,
         ERR      => open,
         ERR_RST  => '0',

         RXD      => AFBUS(0),
         TXD      => AFBUS(1),
         RTS      => '0',
         CTS      => open
      );


end main;
