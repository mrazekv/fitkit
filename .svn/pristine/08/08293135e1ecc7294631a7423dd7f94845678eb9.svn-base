-- sokoban_top: Sokoban
-- Copyright (C) 2007 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Jiri Subr <xsubrj00 AT stud.fit.vutbr.cz>
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
use IEEE.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;

architecture arch_vga_game of tlv_pc_ifc is
   -- Nastaveni pro VGA
   signal isettings         : std_logic_vector(60 downto 0);

   -- Barvy pro VGA
   -- kazda 3 bity
   signal ired              : std_logic_vector(2 downto 0) := (others => '0');
   signal igreen            : std_logic_vector(2 downto 0) := (others => '0');
   signal iblue             : std_logic_vector(2 downto 0) := (others => '0');
   -- Pozadavky na barvu pro radek/sloupec
   signal req_lin           : std_logic_vector(11 downto 0);
   signal req_col           : std_logic_vector(11 downto 0);

   -- ----------------------------TEXTURA---------------------------------------
   -- Port A BRAM (nacitani textury z SPI do BRAM)
   signal spi_tex_addr          : std_logic_vector(11 downto 0);
   signal spi_tex_data_out      : std_logic_vector(15 downto 0);   -- 16bit. sirka kvuli 9bit. texture
   signal spi_tex_data_in       : std_logic_vector(15 downto 0); 
   signal spi_tex_write_en            : std_logic;

   -- Port B BRAM (zobrazovani textury z BRAM)
   signal tex_bram_addr         : std_logic_vector(11 downto 0);
   signal tex_bram_data_out     : std_logic_vector(8 downto 0);
   -- --------------------------KONEC TEXTURA-----------------------------------
   -- --------------------------VIDEO PAMET-------------------------------------
   -- Port A BRAM (nacitani video pameti z SPI do BRAM)
   signal spi_vram_addr    : std_logic_vector (11 downto 0); 
   signal spi_vram_data_out: std_logic_vector (7 downto 0); 
   signal spi_vram_data_in : std_logic_vector (7 downto 0); 
   signal spi_vram_data_in0 : std_logic_vector (7 downto 0); 
   signal spi_vram_data_in1 : std_logic_vector (7 downto 0); 
   signal spi_vram_write_en: std_logic; 
   signal spi_vram_read_en : std_logic;
   -- vyber video pameti
   signal bram_vram0_en : std_logic;
   signal bram_vram1_en : std_logic;

   -- Port B (zobrazovani video pameti z BRAM)
   signal vga_bram_addr    : std_logic_vector (10 downto 0); 
   signal vga_bram_data_out    : std_logic_vector (7 downto 0); 
   signal vga_bram_data_out0    : std_logic_vector (7 downto 0); 
   signal vga_bram_data_out1    : std_logic_vector (7 downto 0); 
   -- -------------------------KONEC VIDEO PAMET--------------------------------
      -- Port A FONTRAM
   signal spi_font_addr    : std_logic_vector (10 downto 0); 
   signal spi_font_data_out: std_logic_vector (7 downto 0); 
   signal spi_font_data_in : std_logic_vector (7 downto 0); 
   signal spi_font_write_en: std_logic; 
   signal spi_font_read_en : std_logic;

   -- Port B FONTRAM
   signal font_bram_addr    : std_logic_vector (10 downto 0); 
   signal font_bram_data_out    : std_logic_vector (7 downto 0); 
   signal font_out          : std_logic;

   -- Ridici registr
   signal spi_reg_write_en      : std_logic;
   signal reg_data_out     : std_logic_vector(7 downto 0);
   signal spi_reg_data_in  : std_logic_vector(7 downto 0);

   -- Obsluha klavesnice pomoci MCU
   signal spi_kbrd_data_out     : std_logic_vector(15 downto 0);
   signal spi_kbrd_data_in      : std_logic_vector(15 downto 0);
   signal spi_kbrd_we           : std_logic;

   -- Obsluha displeje pomoci MCU
   signal spi_dis_addr     : std_logic_vector(0 downto 0);
   signal spi_dis_data_out : std_logic_vector(15 downto 0);
   signal spi_dis_write_en : std_logic; 

   -- Komparatory pro vybrany text zpzodene o 1, 2 takty
   signal sel_font              : std_logic := '0';
   signal sel_font_t1           : std_logic := '0';
   
   signal invert_tex_t1             : std_logic;
   signal invert_tex                : std_logic;

   -- Signaly pozce v texture  zpozdene o 1, 2 takty
   signal text_x_t1, text_y_t1  : std_logic_vector(4 downto 0) := (others => '0');
   signal text_x_t2, text_y_t2  : std_logic_vector(4 downto 0) := (others => '0');
   -- font
   signal font_raddr_reg        : std_logic_vector(3 downto 0);
   signal font_caddr_reg        : std_logic_vector(2 downto 0);
   signal font_caddr_reg2       : std_logic_vector(2 downto 0);

   -- Pouzite komponenty
   -- SPI dekoder
   -- Popis: prevadi vnitrni serioiviu komunikaci 
   --        na jednoduche paralelni rozhrani
   component SPI_adc
      generic (
         ADDR_WIDTH     : integer;
         DATA_WIDTH     : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR      : integer;
         DELAY          : integer := 0
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
         DOPA0 : out std_logic;
         ADDRA : in std_logic_vector(10 downto 0);
         CLKA  : in std_ulogic;
         DIA   : in std_logic_vector(7 downto 0);
         DIPA0 : in std_logic;
         ENA   : in std_ulogic;
         SSRA  : in std_ulogic;
         WEA   : in std_ulogic;

         DOB   : out std_logic_vector(7 downto 0);
         DOPB0  : out std_logic;
         ADDRB : in std_logic_vector(10 downto 0);
         CLKB  : in std_ulogic;
         DIB   : in std_logic_vector(7 downto 0);
         DIPB0  : in std_logic;
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

begin
   -- spi decoder - textures
   -- stejny i pro mou pamet
   SPI_adc_tex: SPI_adc
      generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 12,  -- sirka adresy na vystupu 12 bitu
         BASE_ADDR  => 16#1000# -- adresovy prostor od 0x1000
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
         WRITE_EN => spi_tex_write_en,
         READ_EN  => open
      );

   -- spi decoder - vram
   SPI_adc_vram: SPI_adc
   
         generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16bitu
         DATA_WIDTH => 8,       -- sirka dat 8bitu
         ADDR_OUT_WIDTH => 12,  -- sirka adresy na vystupu 12bitu (4096B)
         BASE_ADDR  => 16#2000#,-- adresovy prostor 0x2000 - 0x2FFF
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
      -- spi decoder (fontRAM)
   SPI_adc_font: SPI_adc
      generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16bitu
         DATA_WIDTH => 8,       -- sirka dat 8bitu
         ADDR_OUT_WIDTH => 11,  -- sirka adresy na vystupu 11bitu (2048B)
         BASE_ADDR  => 16#8000# -- adresovy prostor 0x8000 - 0x87FF 
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,    

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ, 

         ADDR     => spi_font_addr,
         DATA_OUT => spi_font_data_out,
         DATA_IN  => spi_font_data_in,
         WRITE_EN => spi_font_write_en,
         READ_EN  => spi_font_read_en
    );
   -- spi decoder - keyboard
   SPI_adc_kbrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#0040# -- adresovy prostor 0x40
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

   -- Radic klavesnice
   kbrd_ctrl: keyboard_controller
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
            DATA_WIDTH => 16,    -- sirka dat 8 bitu
            ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
            BASE_ADDR  => 16#00# -- adresovy prostor od 0x00
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
         
   -- -------------------REGISTR-----------------------------------
   
     -- SPI dekoder pro ridici registr
   spi_adc_reg: SPI_adc
      generic map (
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 8,     -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#30# -- adresa - 0xA0 a 0xA1
      )
      port map (
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => spi_reg_data_in,
         DATA_IN  => reg_data_out,
         WRITE_EN => spi_reg_write_en,
         READ_EN  => open
      );
      
   --Registr ridici zobrazeni textu/grafiky
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         reg_data_out <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (spi_reg_write_en = '1') then
            reg_data_out <= spi_reg_data_in;
         end if;
      end if;
   end process;
-- -------------------REGISTR KONEC-----------------------------------
   -- =========================================================================
   --  Tvorba obrazu
   -- =========================================================================   
   -- BRAM pro font 128 znaku (8*16 bitu)
   blkram_font: RAMB16_S9_S9
      port map (
         DOA             => spi_font_data_in,
         DOPA0           => open,
         ADDRA           => spi_font_addr,
         
         CLKA            => CLK,
         DIA             => spi_font_data_out,
         DIPA0           => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_font_write_en,

         DOB             => font_bram_data_out,
         DOPB0           => open,
         ADDRB           => font_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         DIPB0           => '0',
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );
   -- BRAM pro vsechny textury
   -- jeden bod ma 9 bitu, proto pro ulozeni bodu vyuzivame i paritni bit
   bram_texture: RAMB16_S9_S9
      port map (
         DOA(7 downto 0) => spi_tex_data_in(8 downto 1),    -- vstup dat do SPI dekoderu
         DOPA0           => spi_tex_data_in(0),             -- vstup paritnich dat do SPI dekoderu
         ADDRA           => spi_tex_addr(10 downto 0),      -- adresa pametove bunky 2048 pametovych bunek (kazda 8 bitu), 
                                                            -- (16x1024)/8 = 2048
         CLKA            => CLK,                            -- hodinovy signal
         DIA(7 downto 0) => spi_tex_data_out(8 downto 1),   -- data prijmuta dekoderem SPI, hornich 8 bitu
         DIPA0           => spi_tex_data_out(0),           -- data prijmuta dekoderem SPI, nejnizsi bit ulozen do parity bitu
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_tex_write_en,

         DOB(7 downto 0) => tex_bram_data_out(8 downto 1),
         DOPB0           => tex_bram_data_out(0),
         ADDRB           => tex_bram_addr(10 downto 0),
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         DIPB0           => 'Z',
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   -- povoleni video pameti
   bram_vram0_en <= (spi_vram_write_en or spi_vram_read_en) and (not spi_vram_addr(11));
   bram_vram1_en <= (spi_vram_write_en or spi_vram_read_en) and spi_vram_addr(11);

   -- 2x RAMB pro obraz (videoRAM)
   bram_vram0: RAMB16_S9_S9
      port map (
         DOA             => spi_vram_data_in0,
         DOPA0           => open,
         ADDRA           => spi_vram_addr(10 downto 0),
         CLKA            => CLK,
         DIA             => spi_vram_data_out,
         DIPA0           => '0',
         ENA             => bram_vram0_en,
         SSRA            => '0',
         WEA             => spi_vram_write_en,

         DOB             => vga_bram_data_out0,
         DOPB0           => open,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         DIPB0           => '0',
         ENB             => not req_lin(8),
         SSRB            => '0',
         WEB             => '0'
      );

      bram_vram1: RAMB16_S9_S9
      port map (
         DOA             => spi_vram_data_in1,
         DOPA0           => open,
         ADDRA           => spi_vram_addr(10 downto 0),
         CLKA            => CLK,
         DIA             => spi_vram_data_out,
         DIPA0           => '0',
         ENA             => bram_vram1_en,
         SSRA            => '0',
         WEA             => spi_vram_write_en,

         DOB             => vga_bram_data_out1,
         DOPB0           => open,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         DIPB0           => '0',
         ENB             => req_lin(8),          -- druha pulka displeje
         SSRB            => '0',
         WEB             => '0'
      );
   
   
   
      -- Block RAM 1 and 2 data output select (PORT A)
  bram_data_out_mux : process(CLK, spi_vram_data_in1, spi_vram_data_in0, spi_vram_addr(11))   
  begin
     if (spi_vram_read_en = '1') then
        case(spi_vram_addr(11)) is 
             when '0'    =>
                    spi_vram_data_in <= spi_vram_data_in0;
             when '1'    =>
                    spi_vram_data_in <= spi_vram_data_in1;
             when others =>
                 null;
         end case;
     end if;
  end process bram_data_out_mux;

   -- vyber vram pro zapis a pro vystup
--   spi_vram_data_in     <= spi_vram_data_in1 when (spi_vram_addr(11) = '1') else spi_vram_data_in0;
   vga_bram_data_out    <= vga_bram_data_out1 when req_lin(8)='1' else vga_bram_data_out0;
   
   
   
   -- Vyber grafickeho rezimu (640x480, 60 Hz refresh)
   SetMode(r640x480x60, isettings);

   -- VGA radic
   vga: entity work.vga_controller(arch_vga_controller)
      generic map (REQ_DELAY => 2) -- Delay 2 tiky
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

   -- Zpozdeni souradnic z duvodu cekani na masku z BRAM, 
   -- nejdrive se zjisti jaka data cist coz se ziska dotazem na index ulozeny v BRAM
   position_wait: process(CLK, req_col, req_lin, text_x_t1, text_y_t1)
   begin
       if CLK'event and CLK = '1' then
       
            if reg_data_out(0) = '1' then
                text_x_t1 <= req_col(4 downto 0);       -- souradnice x v texture
                text_y_t1 <= req_lin(4 downto 0);       -- souradnice y v texture

                text_x_t2 <= text_x_t1;                 -- zpozdeni o 2 takty
                text_y_t2 <= text_y_t1;                 -- zpozdeni o 2 takty
                invert_tex_t1 <= invert_tex;
            elsif reg_data_out(0) = '0' then
                font_caddr_reg  <= req_col(2 downto 0);
                font_caddr_reg2 <= font_caddr_reg;      -- zpozdeni o 2 takty
                font_raddr_reg  <= req_lin(3 downto 0); 
                sel_font_t1 <= sel_font;        -- vybrany text
            end if;
            
       end if;
   end process;

   
   create_bram_addr : process(reg_data_out, req_lin, req_col)   
   begin
      case(reg_data_out(0)) is 
         when '0'    => 
            vga_bram_addr <= req_lin(7 downto 4) & req_col(9 downto 3);
         when '1'    =>
            vga_bram_addr   <= req_lin(7 downto 5) & '0' & req_col(9 downto 5) & "00";
         when others =>
            null;
      end case;
   end process create_bram_addr;
         
   -- vyber grafickych dat z pameti fontu nebo textur
   process_bram_data : process(reg_data_out, text_x_t1, text_y_t1, vga_bram_data_out, font_raddr_reg)
   begin
      case (reg_data_out(0)) is
        when '0'    =>
            -- adresace pametoveho mista v BRAM fontu
            font_bram_addr(10 downto 4) <= vga_bram_data_out(6 downto 0); --znak
            sel_font <= vga_bram_data_out(7);                         -- zvyrazneni
            font_bram_addr(3 downto 0)  <= font_raddr_reg;                -- radek
        when '1'    =>
            -- adresace pametoveho mista v BRAM textury
            
            -- Adresu textury ziskame ze signalu vga_bram_data_out, coz je zaroven index textury
            -- pro adresaci bodu v texture pouzijeme signaly <text_x_t1, text_y_t1>

            -- textura je soumerna, takze staci pulka textury
            if text_x_t1(4) = '1' then
                tex_bram_addr(3 downto 0) <= not text_x_t1(3 downto 0);
            else
                tex_bram_addr(3 downto 0) <= text_x_t1(3 downto 0);
            end if;
            tex_bram_addr(10 downto 6) <= text_y_t1;
            tex_bram_addr(4)          <= vga_bram_data_out(0);
            tex_bram_addr(5)          <= vga_bram_data_out(1);
            invert_tex                <= vga_bram_data_out(2);
        when others =>
            null;
      end case;
   end process process_bram_data;
   
   -- vyber vstupnich dat VGA radice
   gen_graphic_data : process(tex_bram_data_out, font_bram_data_out, reg_data_out, sel_font_t1, invert_tex_t1)
   begin
      case (reg_data_out(0)) is
        when '0'    =>
            -- zobrazovani fontu
            if (font_bram_data_out(7-conv_integer(font_caddr_reg2)) = '1') then
            -- na vybrane pozici se nachazi font
                if (sel_font_t1 = '1') then
                -- zvyrazneni vybraneho textu
                    ired    <= "000";
                    igreen  <= "000";
                    iblue   <= "111";
                else
                -- klasicky text
                    ired    <= "111";
                    igreen  <= "111";
                    iblue   <= "111";
                end if;
            else
            -- zobrazeni pozadi
                if (sel_font_t1 = '1') then
                    ired    <= "111";
                    igreen  <= "111";
                    iblue   <= "111";
                else
                    ired    <= "000";
                    igreen  <= "000";
                    iblue   <= "111";
                end if;
            end if;
        when '1'    =>
            -- zobrazovani obrazove textury
            if (invert_tex_t1 = '1') then
                ired    <=  not(tex_bram_data_out(2 downto 0));
                igreen  <=  not(tex_bram_data_out(5 downto 3));
                iblue   <=  not(tex_bram_data_out(8 downto 6));
            else
                ired    <=  tex_bram_data_out(2 downto 0);
                igreen  <=  tex_bram_data_out(5 downto 3);
                iblue   <=  tex_bram_data_out(8 downto 6);
            end if;
        when others =>
            null;
      end case;
   end process gen_graphic_data;
end arch_vga_game;
