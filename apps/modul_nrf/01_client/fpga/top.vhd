-- top.vhd: RF module control register
-- Copyright (C) 2010 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
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

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

architecture main of tlv_gp_ifc is

  signal spi_dout : std_logic_vector(7 downto 0);
  signal ctrl_reg : std_logic_vector(7 downto 0);
  signal spi_we : std_logic; 

  signal miso : std_logic;
  signal mosi : std_logic;
  signal sck : std_logic;
  signal cs : std_logic;
  signal rstce : std_logic;
  signal reg : std_logic_vector(7 downto 0);
  signal cnt : std_logic_vector(21 downto 0);
  
   -- pouzite komponenty
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
  
begin

   -- SPI decoder (access to the control register)
   spidec: SPI_adc
     generic map (
        ADDR_WIDTH => 8,      -- address width 8 bits
        DATA_WIDTH => 8,      -- data width 8 bits
        ADDR_OUT_WIDTH => 1,  -- unused
        BASE_ADDR  => 16#E0#  -- base address 0xE0
     )
     port map (
        CLK      => CLK,
        CS       => SPI_CS,

        DO       => SPI_DO,
        DO_VLD   => SPI_DO_VLD,
        DI       => SPI_DI,
        DI_REQ   => SPI_DI_REQ,

        ADDR     => open,
        DATA_OUT => spi_dout,
        DATA_IN  => ctrl_reg,
        WRITE_EN => spi_we,
        READ_EN  => open
     );  

   control_reg:process (RESET, CLK)
   begin
     if (RESET = '1') then
         ctrl_reg(6 downto 0) <= (others => '0');
     elsif (CLK'event) and (CLK = '1') then
         if (spi_we = '1') then
            ctrl_reg(6 downto 0) <= spi_dout(6 downto 0);
         end if;
     end if;
   end process;

   sck  <= P3M(3);
   mosi <= P3M(1);
   cs   <= P3M(6);
   P3M(2) <= miso;


   X(6)   <= 'Z';
   X(10)  <= 'Z';
   X(13)  <= 'Z';

   X(8)  <= sck when ctrl_reg(0) = '1' else '0'; --SCK
   miso  <= X(10)  when ctrl_reg(0) = '1' else '1';  --MISO
   X(11) <= mosi when ctrl_reg(0) = '1' else 'Z'; --MOSI
   X(9)  <= cs when ctrl_reg(0) = '1' else '1'; --CS
   rstce <= ctrl_reg(1) when ctrl_reg(0) = '1' else '0'; --RESET / CE
   X(7)  <= rstce;
   IRQ   <= X(6) when ctrl_reg(0) = '1' else '0';
 
end main;

