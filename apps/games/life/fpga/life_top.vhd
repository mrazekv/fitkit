-- life_top.vhd : VGA - 80x60 block graphics
-- Copyright (C) 2008 Brno University of Technology,
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

-- 640x480
-- 80x60=4800  8x8pix blocks
-- 7+6=13 bitu addr
-- BRAM 16384 bitu

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;

architecture arch_life of tlv_pc_ifc is

   signal irgb : std_logic_vector(8 downto 0);
   signal rrow : std_logic_vector(11 downto 0);
   signal rcol : std_logic_vector(11 downto 0);

   -- Port A VIDEORAM
   signal spi_vram_addr    : std_logic_vector (12 downto 0); 
   signal spi_vram_data_out: std_logic_vector (7 downto 0); 
   signal spi_vram_data_in : std_logic_vector (7 downto 0); 
   signal spi_vram_write_en: std_logic; 
   signal spi_vram_read_en : std_logic;

   -- Port B VGARAM
   signal vga_bram_addr    : std_logic_vector (12 downto 0); 
   signal vga_bram_dout    : std_logic_vector (1 downto 0); 
  
   signal vga_mode  : std_logic_vector(60 downto 0); -- default 640x480x60
   
   -- Ridici registr
   signal ctrl_we      : std_logic;
   signal ctrl_reg     : std_logic_vector(7 downto 0);
   signal ctrladc_din  : std_logic_vector(7 downto 0);

   -- Citac 
   signal sync_cntr : std_logic_vector(22 downto 0);
   signal sync_cmp  : std_logic;
   
   -- Displej
   signal dis_addr     : std_logic_vector(0 downto 0);
   signal dis_data_out : std_logic_vector(15 downto 0);
   signal dis_write_en : std_logic;

   -- Ostatne
   --signal mx_bit       : std_logic;
   --signal reg_bram_bit : std_logic;

   -- Pouzite komponenty
   component SPI_adc
      generic (
         ADDR_WIDTH : integer;
         DATA_WIDTH : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR  : integer;
         DELAY : integer := 0
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
   -- Nastaveni grafickeho rezimu (640x480, 60 Hz refresh)
   setmode(r640x480x60, vga_mode);
    
   -- spi decoder (videoRAM)
   SPI_adc_ram: SPI_adc
      generic map(
         ADDR_WIDTH => 16,  -- sirka adresy 16bitu
         DATA_WIDTH => 8,  -- sirka dat 8bitu
         ADDR_OUT_WIDTH => 13, -- sirka adresy na vystupu 13bitu
         BASE_ADDR  => 16#8000#, -- adresovy prostor 0x8000 - 0x9FFF 
         DELAY => 1
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,    

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ, 

         ADDR     => spi_vram_addr,
         DATA_OUT => spi_vram_data_out,
         DATA_IN  => spi_vram_data_in,
         WRITE_EN => spi_vram_write_en,
         READ_EN  => open
      );

   spi_vram_data_out(7 downto 2) <= (others => '0');

   -- SPI dekoder pro displej
   spidecd: SPI_adc
         generic map (
            ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
            DATA_WIDTH => 16,     -- sirka dat 8 bitu
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#00# -- adresovy prostor od 0x00-0x01
         )
         port map (
            CLK      => CLK,
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
            CLK    =>  CLK, -- 25MHz

            -- ridici signaly
            DATA_IN  => dis_data_out,
            WRITE_EN => dis_write_en,

            --- signaly displeje
            DISPLAY_RS   => LRS,
            DISPLAY_DATA => LD,
            DISPLAY_RW   => LRW,
            DISPLAY_EN   => LE
         );
         
   -- SPI dekoder pro ovladani
   spidecc: SPI_adc
      generic map (
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 8,     -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#A0# -- adresa - 0xA0 a 0xA1
      )
      port map (
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => ctrladc_din,
         DATA_IN  => ctrl_reg,
         WRITE_EN => ctrl_we,
         READ_EN  => open
      );
 
   --Ridici registr
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         ctrl_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (ctrl_we = '1') then
            ctrl_reg <= ctrladc_din;
         end if;
      end if;
   end process;

   -- VGA radic se zpozdenim 1 takt (data platna nasledujici takt po vystaveni adresy)
   vga: entity work.vga_controller(arch_vga_controller) 
      generic map (REQ_DELAY => 1)
      port map (
         CLK    => CLK, 
         RST    => RESET,
         ENABLE => '1',
         MODE   => vga_mode,

         DATA_RED    => irgb(8 downto 6),
         DATA_GREEN  => irgb(5 downto 3),
         DATA_BLUE   => irgb(2 downto 0),
         ADDR_COLUMN => rcol,
         ADDR_ROW    => rrow,

         VGA_RED   => RED_V,
         VGA_BLUE  => BLUE_V,
         VGA_GREEN => GREEN_V,
         VGA_HSYNC => HSYNC_V,
         VGA_VSYNC => VSYNC_V
      );

   vga_bram_addr <= rrow(8 downto 3) & rcol(9 downto 3);
  
   -- generovani barvy pixelu dle vystupu z bram
   irgb <= "111000000" when (vga_bram_dout = "01") else  --umirajici    (cervena)
 	        "111100000" when (Vga_bram_dout = "10") else  --obzivla      (oranzova)
           "000111000" when (vga_bram_dout = "11") else  --ziva bunka   (zelena)
           "000000111";                                  --mrtva bunka  (modra)

   life: entity work.ca_life
      port map (
         CLK         => CLK,
         RESET       => RESET,
         ADDRESS_VGA => vga_bram_addr,
         WRITE_EN    => spi_vram_write_en,
         FSM_EN      => ctrl_reg(0),
         DATA_IN     => spi_vram_data_out(1 downto 0),
         ADDRESS_WR  => spi_vram_addr,
         TIME_FLAG   => sync_cmp,
         DATA_OUT    => vga_bram_dout
      );


   -- citac pro synchronizaci
   synccntr: process (RESET, CLK)
   begin
      if (RESET = '1') then
         sync_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (sync_cmp = '1') then
            sync_cntr <= (others => '0');
         else
            sync_cntr <= sync_cntr + 1;
         end if;
      end if;
   end process;

   sync_cmp <= '1' when conv_integer(sync_cntr) = 25000000/32 else '0';
  
end arch_life;
