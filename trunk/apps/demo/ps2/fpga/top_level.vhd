-- top_level.vhd : PS/2 demo
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek  xvasic11 AT stud.fit.vutbr.cz
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

architecture ps2demo of tlv_pc_ifc is
   -----------------------------------------------------------------------------
   -- Signaly

   -- interrupt
   signal int_data_out  : std_logic_vector ( 7 downto 0);
   signal int_data_in   : std_logic_vector ( 7 downto 0);
   signal int_write_en  : std_logic;
   signal int_read_en   : std_logic;
   signal irqvector     : std_logic_vector ( 7 downto 0);
   signal irq_out : std_logic;

   -- keyboard 4x4
   signal key_data_in   : std_logic_vector(7 downto 0);
   signal key_data_out  : std_logic_vector(7 downto 0);
   signal key_write_en  : std_logic;

   signal kbout : std_logic_vector(7 downto 0);
   signal kbout_en : std_logic;
   signal iled : std_logic;
   signal kberr : std_logic;

   -----------------------------------------------------------------------------
   -- Pouzite komponenty
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

         ADDR     : out  std_logic_vector(ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out  std_logic_vector(DATA_WIDTH-1 downto 0);
         DATA_IN  : in   std_logic_vector(DATA_WIDTH-1 downto 0);

         WRITE_EN : out  std_logic;
         READ_EN  : out  std_logic
      );
   end component;

   -- interrupt
   component interrupt_controller
      generic (
         WIDTH : integer := 8
      );
      port(
         CLK      : in  std_logic;
         RST      : in  std_logic;

         IRQ_IN  : in std_logic_vector(WIDTH-1 downto 0);
         IRQ_OUT : out std_logic;

         DATA_IN  : in std_logic_vector(WIDTH-1 downto 0);
         WRITE_EN : in std_logic;

         DATA_OUT : out std_logic_vector(WIDTH-1 downto 0);
         READ_EN  : in std_logic
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
   
   -- PS/2 controller
   component PS2_controller is
      port ( 
         -- Reset a synchronizace
         RST     : in   std_logic;
         CLK     : in   std_logic;
   
         -- Rozhrani PS/2
         PS2_CLK  : inout std_logic;
         PS2_DATA : inout std_logic;
    
         -- Vstup (zapis do zarizeni)
         DATA_IN  : in  std_logic_vector(7 downto 0); 
         WRITE_EN : in  std_logic;
   
         -- Vystup (cteni ze zarizeni)
         DATA_OUT : out  std_logic_vector(7 downto 0); 
         DATA_VLD : out  std_logic;
   
         DATA_ERR : out  std_logic
      );
   end component;

   for ps2kb: PS2_controller use entity work.PS2_controller(full);

begin
   irqvector <= "00000" & kbout_en & "00";

   -- vystupni IRQ se nastavi jen pokud neprobiha prenos na sbernici SPI
   IRQ <= irq_out and not SPI_CS;
     
   -- Keyboard
   SPI_adc_keybrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,     
         DATA_WIDTH => 8,     
         ADDR_OUT_WIDTH => 1, 
         BASE_ADDR  => 16#02# 
      )
      port map(
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_IN  => kbout,
         READ_EN  => open,

         DATA_OUT => key_data_out,
         WRITE_EN => key_write_en
      );

   ps2kb: PS2_controller
      port map (
         RST => RESET,
         CLK => CLK,
   
         PS2_CLK  => K_CLK,
         PS2_DATA => K_DATA,

         DATA_IN  => key_data_out,
         WRITE_EN => key_write_en,

         DATA_OUT => kbout,
         DATA_VLD => kbout_en,

         DATA_ERR => kberr
      );

   -- Data Error LED
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         iled <= '1';
      elsif (CLK'event) and (CLK = '1') then
         if (kberr='1') then
            iled <= '0';
         end if;
      end if;
   end process;

   LEDF  <= iled;

   -- SPI -> IRQ
   SPI_adc_INT: SPI_adc
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

   -- IRQ radic
   IRQctrl: interrupt_controller
      port map(
         CLK    =>  CLK,
         RST    =>  reset,

         IRQ_IN  => irqvector,
         IRQ_OUT => irq_out,   -- celkove vystupni preruseni

         -- vystupni vektor preruseni
         DATA_OUT    => int_data_in,
         READ_EN     => int_read_en,

         -- nastaveni masky preruseni
         DATA_IN     => int_data_out,
         WRITE_EN    => int_write_en
      );

end ps2demo;
