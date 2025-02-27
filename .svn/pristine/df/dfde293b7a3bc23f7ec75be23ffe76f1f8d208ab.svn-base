-- top_level.vhd: BRAM demo
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <xvasic11 AT stud.fit.vutbr.cz>
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
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

architecture arch_beh of tlv_bare_ifc is

   signal bram_addr     : std_logic_vector(9 downto 0);
   signal bram_data_out : std_logic_vector(15 downto 0);
   signal bram_data_in  : std_logic_vector(15 downto 0);
   signal bram_we       : std_logic;
   signal bram_en       : std_logic;

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

         ADDR     : out std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out std_logic_vector (DATA_WIDTH-1 downto 0);
         DATA_IN  : in  std_logic_vector (DATA_WIDTH-1 downto 0);

         WRITE_EN : out std_logic;
         READ_EN  : out std_logic
      );
   end component;

   component RAMB16_S18
      port (
         DO   : out std_logic_vector(15 downto 0);
         DOP  : out std_logic_vector(1 downto 0);
         ADDR : in std_logic_vector(9 downto 0);
         CLK  : in std_ulogic;
         DI   : in std_logic_vector(15 downto 0);
         DIP  : in std_logic_vector(1 downto 0);
         EN   : in std_ulogic;
         SSR  : in std_ulogic;
         WE   : in std_ulogic
      );
   end component;

begin
   -- SPI dekoder pro displej
   spidecd: SPI_adc
      generic map (
         ADDR_WIDTH => 16,      -- sirka adresy 16 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 10,  -- sirka adresy k adresaci BRAM 10 bitu
         BASE_ADDR  => 16#0000# -- adresovy prostor od 0x0000 - 0x03FF
      )
      port map (
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => bram_addr,
         DATA_OUT => bram_data_in,
         DATA_IN  => bram_data_out,
         WRITE_EN => bram_we,
         READ_EN  => bram_en
      );

   -- Block RAM
   blkram: RAMB16_S18
      port map (
         CLK  => CLK,
         DO   => bram_data_out,
         DI   => bram_data_in,
         ADDR => bram_addr,
         EN   => '1',
         SSR  => '0',
         WE   => bram_we,
         DOP  => open,
         DIP  => "00"
      );

end arch_beh;
