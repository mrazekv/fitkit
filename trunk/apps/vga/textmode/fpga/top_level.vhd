-- top_level.vhd : Text mode (VGA demo)
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
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;

architecture arch_vga_text_mode of tlv_pc_ifc is

   signal inclk          : std_logic;
   signal vga_clk        : std_logic;
   signal vga_clk_locked : std_logic;
   signal ireset : std_logic;

   signal irgb : std_logic_vector(8 downto 0);
   signal rrow : std_logic_vector(11 downto 0);
   signal rcol : std_logic_vector(11 downto 0);

   -- Port A FONTRAM
   signal spi_bram_addr    : std_logic_vector (10 downto 0); 
   signal spi_bram_data_out: std_logic_vector (7 downto 0); 
   signal spi_bram_data_in : std_logic_vector (7 downto 0); 
   signal spi_bram_write_en: std_logic; 
   signal spi_bram_read_en : std_logic;

   -- Port B FONTRAM
   signal font_bram_addr    : std_logic_vector (10 downto 0); 
   signal font_bram_dout    : std_logic_vector (7 downto 0); 
   signal font_out        : std_logic;
   signal font_raddr_reg      : std_logic_vector(3 downto 0);
   signal font_caddr_reg  : std_logic_vector(2 downto 0);
   signal font_caddr_reg2 : std_logic_vector(2 downto 0);

   -- Port A VGARAM
   signal spi_vram_addr    : std_logic_vector (11 downto 0); 
   signal spi_vram_data_out: std_logic_vector (7 downto 0); 
   signal spi_vram_data_in : std_logic_vector (7 downto 0); 
   signal spi_vram_data_in0 : std_logic_vector (7 downto 0); 
   signal spi_vram_data_in1 : std_logic_vector (7 downto 0); 
   signal spi_vram_write_en: std_logic; 
   signal spi_vram_read_en : std_logic;
   signal vram0_en : std_logic;
   signal vram1_en : std_logic;

   -- Port B VGARAM
   signal vga_bram_addr    : std_logic_vector (10 downto 0); 
   signal vga_bram_dout    : std_logic_vector (7 downto 0); 
   signal vga_bram_dout0    : std_logic_vector (7 downto 0); 
   signal vga_bram_dout1    : std_logic_vector (7 downto 0); 
  
   signal vga_mode  : std_logic_vector(60 downto 0); -- default 640x480x60

   -- pouzite komponenty
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
   
begin
   -- Nastaveni grafickeho rezimu (640x480, 60 Hz refresh)
   setmode(r640x480x60, vga_mode);
    
   -- spi decoder (fontRAM)
   SPI_adc_font: SPI_adc
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
   -- spi decoder (videoRAM)
   SPI_adc_ram: SPI_adc
      generic map(
         ADDR_WIDTH => 16,  -- sirka adresy 16bitu
         DATA_WIDTH => 8,  -- sirka dat 8bitu
         ADDR_OUT_WIDTH => 12, -- sirka adresy na vystupu 12bitu (4096B)
         BASE_ADDR  => 16#A000#, -- adresovy prostor 0xA000 - 0xAFFF 
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
         READ_EN  => spi_vram_read_en
      );

   -- RAMB pro font 128 znaku (8*16 bitu)
   blkram_font: RAMB16_S9_S9
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

         DOB             => font_bram_dout,
         DOPB            => open,
         ADDRB           => font_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         DIPB            => (others => '0'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   vram0_en <= (spi_vram_write_en or spi_vram_read_en) and (not spi_vram_addr(11));
   vram1_en <= (spi_vram_write_en or spi_vram_read_en) and spi_vram_addr(11);

   -- 2x RAMB pro obraz (videoRAM)
   blkram_vram0: RAMB16_S9_S9
      port map (
         DOA             => spi_vram_data_in0,
         DOPA            => open,
         ADDRA           => spi_vram_addr(10 downto 0),
         CLKA            => CLK,
         DIA             => spi_vram_data_out,
         DIPA            => (others => '0'),
         ENA             => vram0_en,
         SSRA            => '0',
         WEA             => spi_vram_write_en,

         DOB             => vga_bram_dout0,
         DOPB            => open,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         DIPB            => (others => '0'),
         ENB             => not rrow(8),
         SSRB            => '0',
         WEB             => '0'
      );
   blkram_vram1: RAMB16_S9_S9
      port map (
         DOA             => spi_vram_data_in1,
         DOPA            => open,
         ADDRA           => spi_vram_addr(10 downto 0),
         CLKA            => CLK,
         DIA             => spi_vram_data_out,
         DIPA            => (others => '0'),
         ENA             => vram1_en,
         SSRA            => '0',
         WEA             => spi_vram_write_en,

         DOB             => vga_bram_dout1,
         DOPB            => open,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         DIPB            => (others => '0'),
         ENB             => rrow(8),
         SSRB            => '0',
         WEB             => '0'
      );

   spi_vram_data_in <= spi_vram_data_in1 when vram1_en='1' else spi_vram_data_in0;
   vga_bram_dout    <= vga_bram_dout1 when rrow(8)='1' else vga_bram_dout0;

   -- VGA radic, zpozdeni 2 takty
   vga: entity work.vga_controller(arch_vga_controller) 
      generic map (REQ_DELAY => 2)
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

   --vyber bitu fontu
   process (RESET, CLK)
   begin
      if (RESET='1') then
         font_caddr_reg  <= (others=>'0');
         font_caddr_reg2 <= (others=>'0');
         font_raddr_reg  <= (others=>'0');
      elsif (CLK='1') and (CLK'event) then
         font_caddr_reg  <= rcol(2 downto 0);
         font_caddr_reg2 <= font_caddr_reg; 
         font_raddr_reg  <= rrow(3 downto 0);
      end if;      
   end process;

   vga_bram_addr <= rrow(7 downto 4) & rcol(9 downto 3);
   font_bram_addr(10 downto 4) <= vga_bram_dout(6 downto 0); --znak
   --font (radek)
   font_bram_addr(3 downto 0)  <= font_raddr_reg;
   --font (vyber bitu ze sloupce) multiplexor
   font_out <= font_bram_dout(7-conv_integer(font_caddr_reg2));

   --barva pixelu
   irgb <= "111111111" when (font_out='1') else  --popredi (bila)
           "000000111";                          --pozadi  (modra)

end arch_vga_text_mode;
