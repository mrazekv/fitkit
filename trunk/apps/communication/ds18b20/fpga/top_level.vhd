-- top.vhd : DS18B20
-- Copyright (C) 2009 Brno University of Technology,
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

library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.std_logic_ARITH.ALL;
use IEEE.std_logic_UNSIGNED.ALL;
use work.clkgen_cfg.all; 

architecture main of tlv_gp_ifc is

   -----------------------------------------------------------------------------
   -- Signaly
   signal cmd : std_logic_vector(1 downto 0);
   signal cmd_we : std_logic;
   signal dataout : std_logic_vector(7 downto 0);
   signal datain : std_logic_vector(7 downto 0);
   signal busy : std_logic;
   signal dq   : std_logic;
   signal presence : std_logic;
  
   -- displej
   signal dis_addr     : std_logic_vector(0 downto 0);
   signal dis_data_out : std_logic_vector(15 downto 0);
   signal dis_write_en : std_logic;

   -- ds18b20
   signal ds_addr     : std_logic_vector(0 downto 0);
   signal ds_data_out : std_logic_vector(15 downto 0);
   signal ds_data_in  : std_logic_vector(15 downto 0);
   signal ds_write_en : std_logic;

   signal locked : std_logic;
   signal lf : std_logic;  

   -----------------------------------------------------------------------------
   -- SPI address decoder
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
         WRITE_EN : out std_logic;

         DATA_IN  : in  std_logic_vector (DATA_WIDTH-1 downto 0);
         READ_EN  : out std_logic
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
   -- SPI dekoder pro ds
   spidecds: SPI_adc
         generic map (
            ADDR_WIDTH => 8,     -- sirka adresy
            DATA_WIDTH => 16,    -- sirka dat 
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#A0# -- adresovy prostor od 0x00
         )
         port map (
            CLK      => clk,
            CS       => SPI_CS,

            DO       => SPI_DO,
            DO_VLD   => SPI_DO_VLD,
            DI       => SPI_DI,
            DI_REQ   => SPI_DI_REQ,

            ADDR     => ds_addr,
            DATA_OUT => ds_data_out,
            DATA_IN  => ds_data_in,
            WRITE_EN => ds_write_en,
            READ_EN  => open
         );

   ds_data_in(15 downto 8) <= presence & "000" & busy & "000";
   ds_data_in(7 downto 0) <= dataout;
   datain <= ds_data_out(7 downto 0);
   cmd <= ds_data_out(9 downto 8);
   cmd_we <= ds_write_en;


   LEDF <= lf;
   process (reset, clk)
   begin
      if (reset = '1') then
         lf  <= '1';
      elsif (clk'event) and (clk = '1') then
         if (cmd_we = '1') then
            lf  <= not cmd(0);
         end if;
      end if;
   end process;

   onew: entity work.onewire
         generic map (
            CLK_DIV => 200
         )
         port map (
            CLK  => clk,
            RESET => reset,
            CMD => cmd,
            CMD_WE => cmd_we,
            DATA_IN => datain,
            DATA_OUT => dataout,
            PRESENCE => presence,
            BUSY => busy,
            DQ => dq
         );

   -- datovy vodic one-wire
   X(43) <= dq;

   -- SPI dekoder pro displej
   spidecd: SPI_adc
         generic map (
            ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
            DATA_WIDTH => 16,    -- sirka dat 16 bitu
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#00# -- adresovy prostor od 0x00-0x01
         ) 
         port map (
            CLK      => clk,
            CS       => SPI_CS,

            DO       => SPI_DO,
            DO_VLD   => SPI_DO_VLD,
            DI       => SPI_DI,
            DI_REQ   => SPI_DI_REQ,

            ADDR     => dis_addr,
            DATA_OUT => dis_data_out,
            DATA_IN  => "0000000000000000", 
            WRITE_EN => dis_write_en,
            READ_EN  => open
         );

   -- radic LCD displeje
   lcdctrl: lcd_raw_controller
         port map (
            RST    =>  RESET,
            CLK    =>  CLK,

            -- ridici signaly
            DATA_IN  => dis_data_out,
            WRITE_EN => dis_write_en,

            --- signaly displeje
            DISPLAY_RS   => LRS,
            DISPLAY_DATA => LD,
            DISPLAY_RW   => LRW,
            DISPLAY_EN   => LE
         ); 
 
end main;
