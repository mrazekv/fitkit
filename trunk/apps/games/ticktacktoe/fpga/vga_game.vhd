-- vga_demo: Tick tack toe
-- Copyright (C) 2007 Brno University of Technology,
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
use IEEE.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;
use work.serial_cfg.all; 

-- pragma translate_off 
library unisim;
use unisim.vcomponents.all;
-- pragma translate_on   

architecture arch_vga_game of tlv_pc_ifc is
   -- Nastaveni pro VGA
   signal isettings         : std_logic_vector(60 downto 0);

   -- Barvy pro VGA
   signal ired              : std_logic_vector(2 downto 0) := (others => '0');
   signal igreen            : std_logic_vector(2 downto 0) := (others => '0');
   signal iblue             : std_logic_vector(2 downto 0) := (others => '0');
   -- Pozadavky na barvu pro radek/sloupec
   signal req_lin           : std_logic_vector(11 downto 0);
   signal req_col           : std_logic_vector(11 downto 0);

   -- Pozice pohyblive textury
   signal pos_x             : std_logic_vector(4 downto 0) := (others => '0');
   signal pos_y             : std_logic_vector(4 downto 0) := (others => '0');

   -- Port A BRAM (nacitani textury z SPI do BRAM)
   signal spi_tex_addr      : std_logic_vector(11 downto 0);
   signal spi_tex_data_out  : std_logic_vector(15 downto 0);
   signal spi_tex_data_in   : std_logic_vector(15 downto 0);
   signal spi_tex_we        : std_logic;
   signal bram_tmp          : std_logic_vector(9 downto 0);

   -- Port B BRAM (zobrazovani textury z BRAM)
   signal bram_vga_addr     : std_logic_vector(11 downto 0);
   signal bram_vga_data_out : std_logic_vector(8 downto 0);

   -- Port A BRAM (nacitani masky z SPI do BRAM)
   signal spi_mask_addr     : std_logic_vector(11 downto 0);
   signal spi_mask_data_out : std_logic_vector(7 downto 0);
   signal spi_mask_data_in  : std_logic_vector(7 downto 0);
   signal spi_mask_we       : std_logic;

   -- Port B BRAM (vyuziti masky pri vykreslovani)
   signal mask_vga_addr     : std_logic_vector(11 downto 0);
   signal mask_vga_data_out : std_logic_vector(1 downto 0);

   -- Obsluha klavesnice pomoci MCU
   signal spi_kbrd_data_out : std_logic_vector(15 downto 0);
   signal spi_kbrd_data_in  : std_logic_vector(15 downto 0);
   signal spi_kbrd_we       : std_logic;

   -- Obsluha displeje pomoci MCU
   signal spi_dis_addr     : std_logic_vector(0 downto 0);
   signal spi_dis_data_out : std_logic_vector(15 downto 0);
   signal spi_dis_write_en : std_logic; 

   -- Komparatory pro kurzor
   signal x_cmp              : std_logic := '0';
   signal y_cmp              : std_logic := '0';
   signal cursor_cmp         : std_logic := '0';

   -- Signaly pozadavku X/Y spozdene o 1, 2 takty
   signal x_t1, y_t1        : std_logic_vector(4 downto 0) := (others => '0');
   signal c_t1              : std_logic := '0';
   signal x_t2, y_t2        : std_logic_vector(4 downto 0) := (others => '0');
   signal c_t2              : std_logic := '0';

   -- IRQ
   signal int_data_out: std_logic_vector ( 7 downto 0);
   signal int_data_in : std_logic_vector ( 7 downto 0);
   signal int_write_en: std_logic;
   signal int_read_en : std_logic;
   signal interrupt   : std_logic_vector ( 7 downto 0);
   signal interrupt_out : std_logic;

   signal serial_data_out: std_logic_vector ( 7 downto 0);
   signal serial_data_in : std_logic_vector ( 7 downto 0);
   signal serial_write_en: std_logic;
   signal serial_read_en : std_logic;
   signal serial_vld     : std_logic;
   signal serial_busy    : std_logic;

   signal datareg : std_logic_vector(7 downto 0);
   signal busyreg : std_logic;

   -- Pouzite komponenty
   component SPI_adc
      generic (
         ADDR_WIDTH     : integer;
         DATA_WIDTH     : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR      : integer
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

   component RAMB16_S4_S4
      port (
         DOA   : out std_logic_vector(3 downto 0);
         ADDRA : in std_logic_vector(11 downto 0);
         CLKA  : in std_ulogic;
         DIA   : in std_logic_vector(3 downto 0);
         ENA   : in std_ulogic;
         SSRA  : in std_ulogic;
         WEA   : in std_ulogic;

         DOB   : out std_logic_vector(3 downto 0);
         ADDRB : in std_logic_vector(11 downto 0);
         CLKB  : in std_ulogic;
         DIB   : in std_logic_vector(3 downto 0);
         ENB   : in std_ulogic;
         SSRB  : in std_ulogic;
         WEB   : in std_ulogic
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

   -- interrupt controller
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

   -- serial transceiver
   component serial_transceiver
      generic (
         SPEED     : serial_speed   := s460800Bd;
         DATAWIDTH : serial_databit := 8;
         STOPBITS  : serial_stopbit := 1;
         PARITY    : serial_parity  := sParityEven
      );
      port (
         CLK      : in std_logic;
         RESET    : in std_logic;
   
         DATA_IN  : in std_logic_vector(DATAWIDTH-1 downto 0);
         WRITE_EN : in std_logic;
   
         DATA_OUT : out std_logic_vector(DATAWIDTH-1 downto 0);
         DATA_VLD : out std_logic;
   
         BUSY     : out std_logic; 
         ERR      : out std_logic;
         ERR_RST  : in  std_logic;
   
         -- RS232
         RXD      : in  std_logic;
         TXD      : out std_logic;
         RTS      : in  std_logic;
         CTS      : out std_logic
      );
   end component;

begin
   -- spi decoder - textures
   SPI_adc_tex: SPI_adc
      generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 12,  -- sirka adresy na vystupu 12 bitu
         BASE_ADDR  => 16#1000# -- adresovy prostor od 0x1000 do 0x13FF
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => spi_tex_addr,
         DATA_OUT => spi_tex_data_out,
         DATA_IN  => spi_tex_data_in,
         WRITE_EN => spi_tex_we,
         READ_EN  => open
      );

   -- spi decoder - mask
   SPI_adc_mask: SPI_adc
      generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16 bitu
         DATA_WIDTH => 8,       -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 12,  -- sirka adresy na vystupu 12 bitu
         BASE_ADDR  => 16#2000# -- adresovy prostor od 0x2000 do 0x23FF
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => spi_mask_addr,
         DATA_OUT => spi_mask_data_out,
         DATA_IN  => spi_mask_data_in,
         WRITE_EN => spi_mask_we,
         READ_EN  => open
      );

   -- spi decoder - keyboard
   SPI_adc_kbrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 32 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#0040#   -- adresovy prostor 0x40 - 0x41
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => spi_kbrd_data_out,
         DATA_IN  => spi_kbrd_data_in,
         WRITE_EN => spi_kbrd_we,
         READ_EN  => open
      );

   -- Uschovani X a Y pozice kurzoru do registru
   keyb: process(CLK)
   begin
      if CLK'event and CLK='1' then
        if spi_kbrd_we = '1' then
           pos_x <= spi_kbrd_data_out(4 downto 0);
           pos_y <= spi_kbrd_data_out(12 downto 8);
        end if;
      end if;
   end process;

   -- Radic klavesnice
   kbrd_ctrl: entity work.keyboard_controller(arch_keyboard)
      port map (
         CLK => SMCLK,
         RST => RESET,

         DATA_OUT => spi_kbrd_data_in(15 downto 0),
         DATA_VLD => open,
         
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

   -- SPI decoder - display
   spidecd: SPI_adc
         generic map (
            ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
            DATA_WIDTH => 16,     -- sirka dat 8 bitu
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#00# -- adresovy prostor od 0x00 - 0x01
         )
         port map (
            CLK      => CLK,
            CS       => SPI_CS,

            DO       => SPI_DO,
            DO_VLD   => SPI_DO_VLD,
            DI       => SPI_DI,
            DI_REQ   => SPI_DI_REQ,

            ADDR     => spi_dis_addr,
            DATA_OUT => spi_dis_data_out,
            DATA_IN  => "0000000000000000",
            WRITE_EN => spi_dis_write_en,
            READ_EN  => open
         );

   -- radic LCD displeje
   lcdctrl: lcd_raw_controller
         port map (
            RST    =>  RESET,
            CLK    =>  CLK, --max 8MHz

            -- ridici signaly
            DATA_IN  => spi_dis_data_out,
            WRITE_EN => spi_dis_write_en,

            --- signaly displeje
            DISPLAY_RS   => LRS,
            DISPLAY_DATA => LD,
            DISPLAY_RW   => LRW,
            DISPLAY_EN   => LE
         );

   -- nevyuzite preruseni
   interrupt(7 downto 2) <= (others => '0');

   -- vystupni preruseni se nahodi jen pokud neprobiha prenos na SPI
   IRQ <= interrupt_out and (not SPI_CS);

   -- adresovy dekoder radice preruseni
   SPI_adc_INT: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 8,       -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#80#   -- adresovy prostor od 0x80
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

   -- radic preruseni
   IRQctrl: interrupt_controller
      port map(
         CLK    =>  CLK,
         RST    =>  RESET,

         -- interrupt
         IRQ_IN  => interrupt,
         IRQ_OUT => interrupt_out,   -- celkovy vystupni interupt

         -- vystupni vektor preruseni
         DATA_OUT => int_data_in,
         READ_EN  => int_read_en,

         -- nastaveni masky preruseni
         DATA_IN  => int_data_out,
         WRITE_EN => int_write_en
      );

   -- adresovy dekoder serial
   SPI_adc_RS232: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 8,       -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#04#   -- adresovy prostor od 0x04 - 0x05
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,

         DATA_IN  => datareg,
         READ_EN  => serial_read_en,

         DATA_OUT => serial_data_in,
         WRITE_EN => serial_write_en
      );

   process (RESET, CLK)
   begin
      if (RESET = '1') then
         busyreg <= '0';
      elsif (CLK'event) and (CLK = '1') then
         busyreg <= serial_busy;
      end if;
   end process;

   process (RESET, CLK)
   begin
      if (RESET = '1') then
         datareg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (serial_vld='1') then
            datareg <= serial_data_out;
         end if;
      end if;
   end process;

   --odeslano
   interrupt(1) <= busyreg and (not serial_busy);
   --prijmuto
   interrupt(0) <= serial_vld; 

   -- transceiver, 3*9600Bd, 8 bitu, jeden stop bit, licha parita
   serial: serial_transceiver
      generic map (
         SPEED     => s9600Bd,
         DATAWIDTH => 8,
         STOPBITS  => 1,
         PARITY    => sParityOdd
      )
      port map(
         RESET    => RESET,
         CLK      => CLK,
      
         DATA_OUT => serial_data_out,
         DATA_VLD => serial_vld,
      
         DATA_IN  => serial_data_in,
         WRITE_EN => serial_write_en,
      
         BUSY     => serial_busy,
         ERR      => open,
         ERR_RST  => '0',

         RXD      => RXD_232,
         TXD      => TXD_232,

         RTS      => '1',
         CTS      => open
      );   

   -- =========================================================================
   --  Tvorba obrazu
   -- =========================================================================

   -- RAMB pro cervenou barvu
   blkram_r: RAMB16_S4_S4
      port map (
         DOA(2 downto 0) => spi_tex_data_in(2 downto 0),
         DOA(3)          => bram_tmp(0),
         ADDRA           => spi_tex_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_tex_data_out(2 downto 0),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_tex_we,

         DOB(2 downto 0) => bram_vga_data_out(2 downto 0),
         DOB(3)          => bram_tmp(1),
         ADDRB           => bram_vga_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   -- RAMB pro zelenou barvu
   blkram_g: RAMB16_S4_S4
      port map (
         DOA(2 downto 0) => spi_tex_data_in(5 downto 3),
         DOA(3)          => bram_tmp(2),
         ADDRA           => spi_tex_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_tex_data_out(5 downto 3),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_tex_we,

         DOB(2 downto 0) => bram_vga_data_out(5 downto 3),
         DOB(3)          => bram_tmp(3),
         ADDRB           => bram_vga_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   -- RAMB pro modrou barvu
   blkram_b: RAMB16_S4_S4
      port map (
         DOA(2 downto 0) => spi_tex_data_in(8 downto 6),
         DOA(3)          => bram_tmp(4),
         ADDRA           => spi_tex_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_tex_data_out(8 downto 6),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_tex_we,

         DOB(2 downto 0) => bram_vga_data_out(8 downto 6),
         DOB(3)          => bram_tmp(5),
         ADDRB           => bram_vga_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   -- RAMB pro indexy textur (rozlozeni textur v obraze)
   blkram_mask: RAMB16_S4_S4
      port map (
         DOA(1 downto 0) => spi_mask_data_in(1 downto 0),
         DOA(3 downto 2) => bram_tmp(7 downto 6),
         DOB(1 downto 0) => mask_vga_data_out,
         DOB(3 downto 2) => bram_tmp(9 downto 8),

         ADDRA           => spi_mask_addr,
         ADDRB           => mask_vga_addr,
         CLKA            => CLK,
         CLKB            => CLK,
         DIA(1 downto 0) => spi_mask_data_out(1 downto 0),
         DIA(3 downto 2) => "00",
         DIB             => (others => 'Z'),
         ENA             => '1',
         ENB             => '1',
         SSRA            => '0',
         SSRB            => '0',
         WEA             => spi_mask_we,
         WEB             => '0'
      );

   -- Vyber grafickeho rezimu (640x480, 60 Hz refresh)
   SetMode(r640x480x60, isettings);

   -- VGA radic
   vga: entity work.vga_controller(arch_vga_controller)
      generic map (REQ_DELAY => 2) -- Delay 4 tiky
      port map (
         CLK    => CLK,
         RST    => RESET,
         ENABLE => '1',
         MODE   => isettings,

         DATA_RED    => ired,
         DATA_GREEN  => igreen,
         DATA_BLUE   => iblue,
         ADDR_COLUMN => req_col,
         ADDR_ROW    => req_lin,

         VGA_RED   => RED_V,
         VGA_BLUE  => BLUE_V,
         VGA_GREEN => GREEN_V,
         VGA_HSYNC => HSYNC_V,
         VGA_VSYNC => VSYNC_V
      );

   -- komparatory - test, zda-li aktualni textura lezi pod kurzorem
   x_cmp      <= '1' when (req_col(9 downto 5) = pos_x) else '0';
   y_cmp      <= '1' when (req_lin(9 downto 5) = pos_y) else '0';
   cursor_cmp <= x_cmp and y_cmp;

   -- Zpozdeni souradnic z duvodu cekani na masku z BRAM
   position_wait: process(CLK, req_col, req_lin, x_t1, y_t1, c_t1)
   begin
       if CLK'event and CLK = '1' then
           x_t1 <= req_col(4 downto 0);
           y_t1 <= req_lin(4 downto 0);
           c_t1 <= cursor_cmp;

           x_t2 <= x_t1;
           y_t2 <= y_t1;
           c_t2 <= c_t1;
       end if;
   end process;

   -- Radek, sloupec generuje adresu do pameti indexu textur
   mask_vga_addr(4 downto 0)   <= req_col(9 downto 5);
   mask_vga_addr(9 downto 5)   <= req_lin(9 downto 5);
   mask_vga_addr(11 downto 10) <= "00";

   -- Adresa do pameti textur je slozena z indexu, ktery generuje mask_vga_data_out
   -- a offsetu  <x_t1, y_t1>
   bram_vga_addr(4 downto 0)  <= x_t1;
   bram_vga_addr(10 downto 6) <= y_t1;
   bram_vga_addr(5)           <= mask_vga_data_out(0);
   bram_vga_addr(11)          <= mask_vga_data_out(1);
   
   -- Generovani RGB obrazu
   -- barevne slozky jsou invertovany, je-li nad texturou kurzor (c_t2 = 1)
   ired <= not bram_vga_data_out(2 downto 0) when c_t2 = '1' else
           bram_vga_data_out(2 downto 0);

   igreen <= not bram_vga_data_out(5 downto 3) when c_t2 = '1' else
           bram_vga_data_out(5 downto 3);

   iblue <= not bram_vga_data_out(8 downto 6) when c_t2 = '1' else
           bram_vga_data_out(8 downto 6);

end arch_vga_game;
