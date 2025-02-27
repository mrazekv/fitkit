-- vga_demo: VGA demo application
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
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
use work.vga_controller_cfg.all;

architecture arch_vga_controller_demo of tlv_pc_ifc is

   -- Stavy kresliciho automatu
   type state_vgademo is (stNormalDraw, stPicOnLine, stPic);

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
   signal pos_x             : resolution := conv_std_logic_vector(1, 12);
   signal pos_y             : resolution := conv_std_logic_vector(1, 12);

   -- Port A BRAM (nacitani textury z SPI do BRAM)
   signal spi_bram_addr     : std_logic_vector(11 downto 0);
   signal spi_bram_data_out : std_logic_vector(15 downto 0);
   signal spi_bram_data_in  : std_logic_vector(15 downto 0);
   signal spi_bram_write_en : std_logic;

   -- Port B BRAM (zobrazovani textury z BRAM)
   signal bram_vga_addr     : std_logic_vector(11 downto 0);
   signal bram_vga_data_out : std_logic_vector(8 downto 0);

   -- Port A BRAM (nacitani masky z SPI do BRAM)
   signal spi_mask_addr     : std_logic_vector(11 downto 0);
   signal spi_mask_data_out : std_logic_vector(15 downto 0);
   signal spi_mask_data_in  : std_logic_vector(15 downto 0);
   signal spi_mask_write_en : std_logic;

   -- Port B BRAM (vyuziti masky pri vykreslovani)
   signal mask_vga_addr     : std_logic_vector(11 downto 0);
   signal mask_vga_data_out : std_logic_vector(1 downto 0);

   -- Obsluha klavesnice pomoci MCU
   signal spi_kbrd_addr     : std_logic_vector(0 downto 0);
   signal spi_kbrd_data_out : std_logic_vector(31 downto 0);
   signal spi_kbrd_data_in  : std_logic_vector(15 downto 0);
   signal spi_kbrd_write_en : std_logic;

   -- Nevyuzite signaly
   signal not_used          : std_logic_vector(9 downto 0);

   -- MX pro levy horni pixel (X/Y) pohyblive textury
   signal move_pic_x_mx     : std_logic := '0';
   signal move_pic_y_mx     : std_logic := '0';

   -- MX pro pocitadlo sirky/vysky (W/H) pohyblive textury
   signal move_pic_w_mx     : std_logic := '0';
   signal move_pic_h_mx     : std_logic := '0';

   -- Citace vysky/sirky pohyblive textury
   signal wcnt, hcnt        : std_logic_vector(4 downto 0) := (others => '0');

   -- Signaly vysky/sirky spozdene o 1 takt
   signal wcnt_t1, hcnt_t1  : std_logic_vector(4 downto 0) := (others => '0');

   -- Signaly pozadavku X/Y spozdene o 1 takt
   signal x_t1, y_t1        : std_logic_vector(4 downto 0) := (others => '0');

   -- Aktivace citacu H/W sirky pohyblive textury
   signal wcnt_en, hcnt_en  : std_logic := '0';

   -- Stav zobrazovani pohyblive textury GEN_MOV
   -- Stav zobrazoveni posledniho pixelu GEN_FIN pohyblive textury GEN_MOV
   signal gen_mov, gen_fin  : std_logic := '0';

   -- Zobrazovaci FSM
   signal cstate, nstate    : state_vgademo;

   -- Pouzite komponenty
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

begin

   -- Nastaveni grafickeho rezimu (640x480, 60 Hz refresh)
   SetMode(r640x480x60, isettings);

   -- spi adapter - textures
   SPI_adc_tex: SPI_adc
      generic map(
         ADDR_WIDTH => 16,      -- sirka adresy 16 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 12,  -- sirka adresy na vystupu 12 bitu
         BASE_ADDR  => 16#0000# -- adresovy prostor od 0x0000
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
         READ_EN  => open
      );

   -- spi adapter - mask
   SPI_adc_mask: SPI_adc
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

         ADDR     => spi_mask_addr,
         DATA_OUT => spi_mask_data_out,
         DATA_IN  => spi_mask_data_in,
         WRITE_EN => spi_mask_write_en,
         READ_EN  => open
      );

   -- spi adapter - keyboard
   SPI_adc_kbrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 32,      -- sirka dat 32 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#40#   -- adresovy prostor 0x40
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => spi_kbrd_addr,
         DATA_OUT => spi_kbrd_data_out,
         DATA_IN(15 downto 0)  => spi_kbrd_data_in(15 downto 0),
         DATA_IN(31 downto 16) => "0000000000000000",
         WRITE_EN => spi_kbrd_write_en,
         READ_EN  => open
      );

   -- RAMB pro cervenou barvu
   blkram_r: RAMB16_S4_S4
      port map (
         DOA(2 downto 0) => spi_bram_data_in(2 downto 0),
         DOA(3)          => not_used(0),
         ADDRA           => spi_bram_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_bram_data_out(2 downto 0),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_bram_write_en,

         DOB(2 downto 0) => bram_vga_data_out(2 downto 0),
         DOB(3)          => not_used(3),
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
         DOA(2 downto 0) => spi_bram_data_in(5 downto 3),
         DOA(3)          => not_used(1),
         ADDRA           => spi_bram_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_bram_data_out(5 downto 3),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_bram_write_en,

         DOB(2 downto 0) => bram_vga_data_out(5 downto 3),
         DOB(3)          => not_used(4),
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
         DOA(2 downto 0) => spi_bram_data_in(8 downto 6),
         DOA(3)          => not_used(2),
         ADDRA           => spi_bram_addr,
         CLKA            => CLK,
         DIA(2 downto 0) => spi_bram_data_out(8 downto 6),
         DIA(3)          => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_bram_write_en,

         DOB(2 downto 0) => bram_vga_data_out(8 downto 6),
         DOB(3)          => not_used(5),
         ADDRB           => bram_vga_addr,
         CLKB            => CLK,
         DIB             => (others => 'Z'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );

   -- RAMB pro masku (rozlozeni textur v obraze)
   blkram_mask: RAMB16_S4_S4
      port map (
         DOA(1 downto 0) => spi_mask_data_in(1 downto 0),
         DOA(3 downto 2) => not_used(7 downto 6),
         DOB(1 downto 0) => mask_vga_data_out,
         DOB(3 downto 2) => not_used(9 downto 8),

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
         WEA             => spi_mask_write_en,
         WEB             => '0'
      );

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
         VGA_VSYNC => VSYNC_V);

   -- Radic klavesnice
   kbrd_ctrl: entity work.keyboard_controller(arch_keyboard)
      port map (
         CLK => CLK,
         RST => RESET,

         DATA_OUT => spi_kbrd_data_in(15 downto 0),
         DATA_VLD => open,
         
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

   -- Cteni X/Y pozice pohyblive textury z SPI (MCU)
   keyb: process(CLK)
   begin
      if CLK'event and CLK = '1' then
        if spi_kbrd_write_en = '1' then
           pos_x <= spi_kbrd_data_out(11 downto 0);
           pos_y <= spi_kbrd_data_out(27 downto 16);
        end if;
      end if;
   end process;

   -- MX pro pohyblivou texturu
   move_pic_x_mx <= '1' when (req_col = pos_x) else '0';
   move_pic_y_mx <= '1' when (req_lin = pos_y) else '0';
   move_pic_w_mx <= '1' when (wcnt_t1 = 31) else '0';
   move_pic_h_mx <= '1' when (hcnt_t1 = 31) else '0';

   -- Pocitadlo sirky
   cnt_x: process(CLK, RESET)
   begin
       if RESET = '1' then
           wcnt <= (others => '0');
       elsif CLK'event and CLK = '1' then
           if wcnt_en = '1' then
               wcnt <= wcnt + 1;
           end if;
       end if;
   end process;

   -- Pocitadlo vysky
   cnt_y: process(CLK, RESET)
   begin
       if RESET = '1' then
           hcnt <= (others => '0');
       elsif CLK'event and CLK = '1' then
           if hcnt_en = '1' then
               hcnt <= hcnt + 1;
           end if;
       end if;
   end process;

   -- Spozdeni souradnic z duvodu cekani na masku z BRAM
   position_wait: process(CLK)
   begin
       if CLK'event and CLK = '1' then
           x_t1 <= req_col(4 downto 0);
           y_t1 <= req_lin(4 downto 0);
           wcnt_t1 <= wcnt;
           hcnt_t1 <= hcnt;
       end if;
   end process;

   ired   <= bram_vga_data_out(2 downto 0);
   igreen <= bram_vga_data_out(5 downto 3);
   iblue  <= bram_vga_data_out(8 downto 6);
   
   
   -- Zobrazovani pohyblive textury vs. pozadi
   bram_vga_ADDR(11 downto 0) <= '1' & hcnt_t1 & '1' & wcnt_t1 when gen_mov = '1' else
                                 mask_vga_data_out(1) & y_t1 & mask_vga_data_out(0) & x_t1;

   mask_vga_addr(4 downto 0) <= req_col(9 downto 5);
   mask_vga_addr(9 downto 5) <= req_lin(9 downto 5);
   mask_vga_addr(11 downto 10) <= "00";

   -- Automat
   FSM_ctrl: process(CLK, RESET)
   begin
       if RESET = '1' then
           cstate <= stNormalDraw;
       elsif CLK'event and CLK = '1' then
           cstate <= nstate;
       end if;
   end process;

   -- Rizeni automatu
   FSM: process(cstate, move_pic_x_mx, move_pic_y_mx, move_pic_w_mx, move_pic_h_mx)
   begin
      nstate <= stNormalDraw;
      wcnt_en <= '0';
      hcnt_en <= '0';
      gen_mov <= '0';
      gen_fin <= '0';
      case cstate is
         when stNormalDraw =>
            if move_pic_y_mx = '1' then
               if move_pic_x_mx = '1' then
                  -- Special request 0
                  wcnt_en <= '1';
                  nstate <= stPic;
               else
                  -- Normal request
                  nstate <= stPicOnLine;
               end if;
            else
               -- Normal request
               nstate <= stNormalDraw;
            end if;

         when stPicOnLine =>
            if move_pic_x_mx = '1' then
               -- Special request 0
               wcnt_en <= '1';
               nstate <= stPic;
            else
               -- Normal request
               nstate <= stPicOnLine;
            end if;

         when stPic =>
            gen_mov <= '1';
            if move_pic_w_mx = '1' then -- Posleni pixel se requestuje
               -- Special request 31
               gen_fin <= '1';
               hcnt_en <= '1';
               if move_pic_h_mx = '1' then
                  nstate <= stNormalDraw;
               else
                  nstate <= stPicOnLine;
               end if;
            else
               -- Special request 1-30
               wcnt_en <= '1';
               nstate <= stPic;
            end if;

         when others =>
            nstate <= cstate;
      end case;
   end process;

end arch_vga_controller_demo;
