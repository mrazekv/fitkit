-- top_level.vhd : keyboard
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek   vasicek AT fit.vutbr.cz
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
use ieee.std_logic_ARITH.ALL;
use ieee.std_logic_UNSIGNED.ALL;



architecture main of tlv_bare_ifc is
   -----------------------------------------------------------------------------
   -- Signaly
   signal adc_addr     : std_logic_vector(1 downto 0);
   signal adc_data_out : std_logic_vector(15 downto 0);
   signal adc_data_in  : std_logic_vector(15 downto 0);
   signal adc_we       : std_logic;

   -- Keyboard 4x4 
   signal key_data_in  : std_logic_vector (15 downto 0);

   -- LCD Display
   signal lcd_we       : std_logic;

   -----------------------------------------------------------------------------
   -- POUZITE KOMPONENTY
   component SPI_adc
      generic (
         ADDR_WIDTH : integer;
         DATA_WIDTH : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR  : integer
      );
      port (
         CLK      : in  std_logic;

         CS       : in  std_logic;
         DO       : in  std_logic;
         DO_VLD   : in  std_logic;
         DI       : out std_logic;
         DI_REQ   : in  std_logic;

         ADDR     : out  std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out  std_logic_vector (DATA_WIDTH-1 downto 0);
         DATA_IN  : in   std_logic_vector (DATA_WIDTH-1 downto 0);

         WRITE_EN : out  std_logic;
         READ_EN  : out  std_logic
      );
   end component;

   -- Keyboard 4x4
   component keyboard_controller
      port(
         CLK      : in std_logic;
         RST      : in std_logic;

         DATA_OUT : out std_logic_vector(15 downto 0);
         DATA_VLD : out std_logic;

         KB_KIN   : out std_logic_vector(3 downto 0);
         KB_KOUT  : in  std_logic_vector(3 downto 0)
      );
   end component;

   component lcd_raw_controller
      port (
         RST      : in std_logic;
         CLK      : in std_logic;

         DATA_IN  : in std_logic_vector (15 downto 0);
         WRITE_EN : in std_logic;

         DISPLAY_RS   : out   std_logic;
         DISPLAY_DATA : inout std_logic_vector(7 downto 0);
         DISPLAY_RW   : out   std_logic;
         DISPLAY_EN   : out   std_logic
      );
   end component;

begin

   LEDF  <= '0';

   -- Adresovy dekoder pro KB
   SPI_adc_keybrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 2,   -- sirka adresy na vystupu 2 bity
         BASE_ADDR  => 16#0000# -- adresovy prostor od 0x00 = LCD, 0x02 = KB
      )
      port map(
         CLK   =>  CLK,
   
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
   
         ADDR     => adc_addr,
         DATA_OUT => adc_data_out,
         DATA_IN  => adc_data_in,
         WRITE_EN => adc_we,
         READ_EN  => open
      );
     
   adc_data_in <= key_data_in when adc_addr(1) = '1' else (others => '0');
   lcd_we <= (not adc_addr(1)) and adc_we;

   -- Radic klavesnice
   keybrd: keyboard_controller
      port map(
         CLK      => CLK,
         RST      => RESET,       -- reset
         
         DATA_OUT => key_data_in,         
         DATA_VLD => open,

         -- Keyboart
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );
  
   -- Radic LCD displeje
   lcdctrl: lcd_raw_controller
         port map (
            RST    =>  RESET,
            CLK    =>  CLK,

            -- ridici signaly
            DATA_IN  => adc_data_out,
            WRITE_EN => lcd_we,

            --- signaly displeje
            DISPLAY_RS   => LRS,
            DISPLAY_DATA => LD,
            DISPLAY_RW   => LRW,
            DISPLAY_EN   => LE
         );
 

end main;

