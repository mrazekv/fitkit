-- top_level.vhd: Picoblaze demo
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

library IEEE;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

architecture arch_demo of tlv_bare_ifc is

   -- Komponenta pro rizeni RUN/STOP cpu
   component SPI_adc
      generic (
         DELAY      : integer;
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

   component picocpu
      port (
         -- Hodinovy sig. + reset
         CLK           : in std_logic;
         RST           : in std_logic;
         -- CPU run (1) vs. reset (0) stav, 0 musi byt pro zmenu programu v RAM!
         CORE_EN       : in std_logic;
         -- Identifikator portu      
         PORT_ID       : out std_logic_vector(7 downto 0);
         -- Pozadavek na cteni/zapis dat na port
         WRITE_STROBE  : out std_logic;
         READ_STROBE   : out std_logic;
         -- Vstupni/vystupni data portu (PORT_ID)
         OUT_PORT      : out std_logic_vector(7 downto 0);
         IN_PORT       : in std_logic_vector(7 downto 0);
         -- Signal pro preruseni + zpetne potvrzeni
         INTERRUPT     : in std_logic;
         INTERRUPT_ACK : out std_logic;

         RAM_ADDR      : in std_logic_vector(9 downto 0);
         RAM_DATA_IN   : in std_logic_vector(17 downto 0);
         RAM_DATA_OUT  : out std_logic_vector(17 downto 0);
         RAM_DATA_WE   : in std_logic
      );
   end component;
   
   signal cpu_ctrl_reg     : std_logic_vector(7 downto 0);
   signal port_reg         : std_logic_vector(7 downto 0);

   signal spi_ctrl_we      : std_logic;
   signal spi_ctrl_data    : std_logic_vector(7 downto 0);

   signal spi_ram_addr     : std_logic_vector(9 downto 0);
   signal spi_data_out     : std_logic_vector(23 downto 0);
   signal spi_data_in      : std_logic_vector(23 downto 0);
   signal cpu_ram_data_out : std_logic_vector(17 downto 0);
   signal spi_ram_we       : std_logic;

   signal cpu_port_we      : std_logic;
   signal cpu_port_out     : std_logic_vector(7 downto 0);
   signal cpu_port_addr    : std_logic_vector(7 downto 0);
      
begin
   -- SPI_ADC pro pristup do instrukcni pameti
   spi_cpu_ram: SPI_adc
      generic map (
         DELAY          => 1,
         ADDR_WIDTH     => 16,
         DATA_WIDTH     => 24,
         ADDR_OUT_WIDTH => 10,
         BASE_ADDR      => 16#F800# -- 1111100a aaaaaaaa
      )
      port map (
         CLK      => CLK,
         
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => spi_ram_addr,
         DATA_OUT => spi_data_out,
         DATA_IN  => spi_data_in,

         WRITE_EN => spi_ram_we,
         READ_EN  => open
      );

   spi_data_in <= "000000" & cpu_ram_data_out;

   -- SPI_ADC pro pristup do ridiciho registru
   spi_cpu_control: SPI_adc
      generic map (
         DELAY      => 1,
         ADDR_WIDTH => 8,      -- sirka adresy 8 bitu
         DATA_WIDTH => 8,      -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1,  -- sirka adresy na vystupu 1 bit (nevyuzito)
         BASE_ADDR  => 16#C0#  -- adresovy prostor od 0x0000
      )
      port map (
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => spi_ctrl_data,
         DATA_IN  => cpu_ctrl_reg,
         WRITE_EN => spi_ctrl_we,
         READ_EN  => open
      );

   -- PicoCPU
   pico_cpu: picocpu
      port map (
         -- Hodinovy signal + reset
         CLK           => CLK,
         RST           => RESET,
         -- Povoleni cinnosti procesoru
         CORE_EN       => cpu_ctrl_reg(0),
         -- I/O port
         PORT_ID       => cpu_port_addr,
         WRITE_STROBE  => cpu_port_we,
         OUT_PORT      => cpu_port_out,
         READ_STROBE   => open,
         IN_PORT       => (others => '0'),
         -- System preruseni
         INTERRUPT     => '0',
         INTERRUPT_ACK => open,
         -- Pristup k instrukcni pameti
         RAM_ADDR      => spi_ram_addr,
         RAM_DATA_IN   => spi_data_out(17 downto 0),
         RAM_DATA_OUT  => cpu_ram_data_out,
         RAM_DATA_WE   => spi_ram_we
      );
   
   -- Ridici registr
   ctrl_reg: process(CLK, RESET)
   begin
      if RESET = '1' then
         cpu_ctrl_reg <= (others => '0');
      elsif CLK'EVENT and CLK = '1' then
         if (spi_ctrl_we = '1') then
            cpu_ctrl_reg <= spi_ctrl_data;
         end if;
      end if;
   end process;
   
   -- Registr pripojeny na port 0x80 procesoru PicoBlaze
   port_led: process(CLK, RESET)
   begin
      if RESET = '1' then
         port_reg <= (others => '0');
      elsif CLK'EVENT and CLK = '1' then
         if (cpu_port_we = '1') and (cpu_port_addr = "10000000") then 
            -- zapis na port 0x80
            port_reg <= cpu_port_out;
         end if;
      end if;
   end process;

   LEDF <= port_reg(0);
   
end arch_demo;
