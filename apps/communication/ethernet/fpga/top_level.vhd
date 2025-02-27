-- top_level.vhd : 
-- Copyright (C) 2008 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek  <vasicek AT fit.vutbr.cz>
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
-- This software is provided ``as is'', and any express or implied
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
use IEEE.std_logic_ARITH.ALL;
use IEEE.std_logic_UNSIGNED.ALL;
use work.clkgen_cfg.all;

-- pragma translate_off 
library unisim;
use unisim.vcomponents.all;
-- pragma translate_on  

architecture main of tlv_gp_ifc is

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

   component RAMB16_S9_S9
      port (
         DOA   : out std_logic_vector(7 downto 0);
         DOPA  : out std_logic_vector(0 downto 0);
         ADDRA : in std_logic_vector(10 downto 0);
         CLKA  : in std_ulogic;
         DIA   : in std_logic_vector(7 downto 0);
         DIPA  : in std_logic_vector(0 downto 0);
         ENA   : in std_ulogic;
         SSRA  : in std_ulogic;
         WEA   : in std_ulogic;

         DOB   : out std_logic_vector(7 downto 0);
         DOPB  : out std_logic_vector(0 downto 0);
         ADDRB : in std_logic_vector(10 downto 0);
         CLKB  : in std_ulogic;
         DIB   : in std_logic_vector(7 downto 0);
         DIPB  : in std_logic_vector(0 downto 0);
         ENB   : in std_ulogic;
         SSRB  : in std_ulogic;
         WEB   : in std_ulogic
      );
   end component; 

   component ethernet
      port( 
         CLK      : in std_logic; -- synchronizace (40 MHz)
         RESET    : in std_logic; -- asynchronnni reset
   
         -- zapis dat
         DATA_WE  : in std_logic;
         DATA_LEN : in std_logic_vector(10 downto 0);
   
         -- interface do BRAM
         RAM_ADDR : out std_logic_vector(10 downto 0);
         RAM_DATA : in std_logic_vector(7 downto 0);

         BUSY     : out std_logic;
   
         -- diferencialni interface   
         TxN : out std_logic;
         TxP : out std_logic
      ); 
   end component;

   -- deklarace signalu
   signal eth_we : std_logic;
   signal eth_dlen : std_logic_vector(15 downto 0);

   signal eth_bram_addr : std_logic_vector(10 downto 0);
   signal eth_bram_dout : std_logic_vector(7 downto 0);

   -- spi-bram
   signal spi_bram_addr    : std_logic_vector (10 downto 0); 
   signal spi_bram_data_out: std_logic_vector (7 downto 0); 
   signal spi_bram_data_in : std_logic_vector (7 downto 0); 
   signal spi_bram_write_en: std_logic; 
   signal spi_bram_read_en : std_logic; 
   
begin
   --zluta LED
   LEDF  <= '0';

   -- SPI dekoder 
   spi_eth: SPI_adc
      generic map (
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 16,     -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#C0# -- adresovy prostor: 0xC0-0xC1
      )
      port map (
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_IN  => "0000000000000000",
         DATA_OUT => eth_dlen,
         WRITE_EN => eth_we,
         READ_EN  => open
      );

   eth: ethernet      
      port map ( 
         CLK     => CLK,
         RESET   => RESET,

         DATA_WE  => eth_we,
         DATA_LEN => eth_dlen(10 downto 0),
        
         RAM_ADDR => eth_bram_addr,
         RAM_DATA => eth_bram_dout,

         BUSY => open, 
         TxN => X(44),
         TxP => X(45)
      ); 

   -- spi decoder (data RAM)
   spi_bram: SPI_adc
      generic map(
         ADDR_WIDTH => 16,  -- sirka adresy 16bitu
         DATA_WIDTH => 8,  -- sirka dat 8bitu
         ADDR_OUT_WIDTH => 11, -- sirka adresy na vystupu 11bitu (2048B)
         BASE_ADDR  => 16#8000# -- adresovy prostor 0x8000 - 0x87FF 
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,    

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ, 

         ADDR     => spi_bram_addr,
         DATA_OUT => spi_bram_data_out,
         DATA_IN  => spi_bram_data_in,
         WRITE_EN => spi_bram_write_en,
         READ_EN  => spi_bram_read_en
      ); 

   -- RAMB pro data paketu
   bram_data: RAMB16_S9_S9
      port map (
         DOA             => spi_bram_data_in,
         DOPA            => open,
         ADDRA           => spi_bram_addr,
         CLKA            => CLK,
         DIA             => spi_bram_data_out,
         DIPA            => (others => '0'),
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_bram_write_en,

         DOB             => eth_bram_dout,
         DOPB            => open,
         ADDRB           => eth_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         DIPB            => (others => '0'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );
 

      
end main;
