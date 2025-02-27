-- spi_reg.vhd: 
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
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

package spi_cfg is
   type port_dir is (PORT_IN, PORT_OUT);
end spi_cfg; 

library IEEE;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
--use work.spi_cfg.all;

entity SPI_reg is
   generic (
      ADDR_WIDTH : integer := 8; 
      DATA_WIDTH : integer := 8;  
      BASE_ADDR  : integer := 16#10#;
      BYPASS     : boolean := false
--      DIRECTION  : port_dir := PORT_OUT
   );
   port (
      -- Synchronizace
      CLK      : in  std_logic;

      -- Rozhrani SPI (z SPI_control)
      CS       : in  std_logic;
      DO       : in  std_logic;
      DO_VLD   : in  std_logic;
      DI       : out std_logic;
      DI_REQ   : in  std_logic;

      --Datove rozhrani
      DATA_IN  : in  std_logic_vector (DATA_WIDTH-1 downto 0);
      DATA_OUT : out std_logic_vector (DATA_WIDTH-1 downto 0)
   );
end SPI_reg;
  

architecture beh of SPI_reg is

   component SPI_adc
      generic (
         DELAY      : integer := 1;
         ADDR_WIDTH : integer := ADDR_WIDTH;
         DATA_WIDTH : integer := DATA_WIDTH;
         ADDR_OUT_WIDTH : integer := 1;
         BASE_ADDR  : integer := BASE_ADDR
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

   signal data_reg : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
   signal spi_data_out : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
   signal spi_data_in  : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
   signal spi_we, spi_re : std_logic;

begin

   -- SPI_ADC pro pristup do instrukcni pameti
   spi_cpu_ram: SPI_adc
      port map (
         CLK      => CLK,
         
         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => open,
         DATA_OUT => spi_data_out,
         DATA_IN  => spi_data_in,

         WRITE_EN => spi_we,
         READ_EN  => spi_re
      );
   
      process (CLK)
      begin
         if (CLK'event) and (CLK = '1') then

            if (spi_re = '1') then
               if (BYPASS) then
                  spi_data_in <= data_reg;
               else
                  spi_data_in <= DATA_IN;
               end if;
            end if;

            if (spi_we = '1') then
               data_reg <= spi_data_out;
            end if;

         end if;
      end process;

      DATA_OUT <= (others=>'Z') when BYPASS else 
                  data_reg;

end architecture;
