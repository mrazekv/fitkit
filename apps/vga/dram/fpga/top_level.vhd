-- top_level.vhd: vga & sdram core
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
use work.sdram_controller_cfg.all;
use work.vga_controller_cfg.all;

architecture arch_beh of tlv_pc_ifc is

   signal spi_data_out   : std_logic_vector(7 downto 0);
   signal spi_we         : std_logic;

   signal sdram_data_out : std_logic_vector(7 downto 0);
   signal sdram_data_in  : std_logic_vector(7 downto 0);
   signal sdram_ra       : std_logic_vector(13 downto 0);

   signal CLK_div     : std_logic;
   signal CLK_div_reg : std_logic;

   signal ram_busy : std_logic;

   -- Pozadavky na barvu pro radek/sloupec
   signal req_row           : std_logic_vector(11 downto 0);
   signal req_col           : std_logic_vector(11 downto 0);

   signal spi_addr    : std_logic_vector(17 downto 0);
   signal sdram_addr  : std_logic_vector(22 downto 0);

   signal isettings : std_logic_vector(60 downto 0);

   signal sdram_vld : std_logic;

   signal req_col_mx : std_logic;
   signal req_row_mx : std_logic;
   signal req_col_reg: std_logic;
   signal req_row_reg: std_logic;

   signal s1 : std_logic;
   signal s2 : std_logic;
   signal ram_cmd_we : std_logic;

   signal di: std_logic_vector(7 downto 0);
   signal di_addr  : std_logic_vector(18 downto 0);

   -- Barvy pro VGA
   signal icolor            : std_logic_vector(8 downto 0) := (others => '0');

   signal cmd : sdram_func;

   -- Buffer
   signal buf2sdr_data      : std_logic_vector(35 downto 0);
   signal spi2buf_data      : std_logic_vector(35 downto 0);
   signal spi2buf_addr      : std_logic_vector(8 downto 0);
   signal buf2sdr_addr      : std_logic_vector(8 downto 0);
   signal buf_step          : std_logic;

   -- Sharing SDRAM signals
   signal req_draw          : std_logic;
   signal req_write         : std_logic;
   signal req_can_write     : std_logic;
   signal req_can_refresh   : std_logic;

   signal vga_hstate : vga_hfsm_state;
   signal vga_vstate : vga_vfsm_state;

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

         ADDR     : out std_logic_vector(ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
         DATA_IN  : in  std_logic_vector(DATA_WIDTH-1 downto 0);

         WRITE_EN : out std_logic;
         READ_EN  : out std_logic
      );
   end component;

   -- Data buffer SPI -> BUFFER -> shared SDRAM
   component RAMB16_S36_S36
      port (
         DOA   : out std_logic_vector(31 downto 0);
         DOB   : out std_logic_vector(31 downto 0);
         DOPA   : out std_logic_vector(3 downto 0);
         DOPB   : out std_logic_vector(3 downto 0);
         ADDRA : in std_logic_vector(8 downto 0);
         ADDRB : in std_logic_vector(8 downto 0);
         CLKA  : in std_logic;
         CLKB  : in std_logic;
         DIA   : in std_logic_vector(31 downto 0);
         DIB   : in std_logic_vector(31 downto 0);
         DIPA   : in std_logic_vector(3 downto 0);
         DIPB   : in std_logic_vector(3 downto 0);
         ENA   : in std_logic;
         ENB   : in std_logic;
         SSRA  : in std_logic;
         SSRB  : in std_logic;
         WEA   : in std_logic;
         WEB   : in std_logic
      );
   end component;

   -- Timing
   component BUFG
      port (
         I: in  std_logic;
         O: out std_logic
      );
   end component;

begin

   LEDF <= '0';

   -- Nastaveni grafickeho rezimu (640x480, 60 Hz refresh)
   SetMode(r640x480x60, isettings);

   -- SPI dekoder pro SDRAM
   spidecd: SPI_adc
      generic map (
         ADDR_WIDTH => 32,      -- sirka adresy 16 bitu
         DATA_WIDTH => 8,       -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 18,  -- sirka adresy na vystupu 18 bitu (512-9b x 480-9b)
         BASE_ADDR  => 16#00000000# -- adresovy prostor od 0x0000
      )
      port map (
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => spi_addr,
         DATA_OUT => spi_data_out,
         DATA_IN  => "00000000",
         WRITE_EN => spi_we,
         READ_EN  => open
      );

   -- SDRAM
   RA <= sdram_ra(13 downto 12) & '0' & sdram_ra(11 downto 0);
   sdram: entity sdram_raw_controller
      generic map (
         -- Generovani prikazu refresh radicem manu
         GEN_AUTO_REFRESH => false
      )
      port map (
         -- Adresa
         ADDR_ROW    => sdram_addr(20 downto 9),
         ADDR_COLUMN => sdram_addr(8 downto 0),
         BANK        => sdram_addr(22 downto 21),

         -- Pozadavek + jeho potvrzeni
         CMD      => cmd,
         CMD_WE   => ram_cmd_we,

         -- Hodiny, reset, ...
         CLK      => CLK,
         RST      => RESET,
         ENABLE   => '1',
         BUSY     => ram_busy,

         -- Data
         DATA_OUT    => sdram_data_out,
         DATA_VLD    => sdram_vld,
         DATA_IN     => sdram_data_in,

         -- SDRAM
         RAM_A       => sdram_ra,
         RAM_D       => RD,
         RAM_DQM     => RDQM,
         RAM_CS      => RCS,
         RAM_RAS     => RRAS,
         RAM_CAS     => RCAS,
         RAM_WE      => RWE,
         RAM_CLK     => RCLK,
         RAM_CKE     => RCKE
      );

   -- Data graphic buffer
   --   pointer A -> data <- pointer B
   --                  ^- 10b RESERVED  8b COLOR  9b YPOS  9b XPOS
   GRBUF: RAMB16_S36_S36
     port map (
         CLKA  => CLK,
         CLKB  => CLK,
         DOA   => open,
         DOB   => buf2sdr_data(31 downto 0),
         DOPA  => open,
         DOPB  => buf2sdr_data(35 downto 32),
         ADDRA => spi2buf_addr(8 downto 0),
         ADDRB => buf2sdr_addr(8 downto 0),
         DIA   => spi2buf_data(31 downto 0),
         DIB   => "00000000000000000000000000000000",
         DIPA  => spi2buf_data(35 downto 32),
         DIPB  => "0000",
         ENA   => '1',
         ENB   => '1',
         SSRA  => '0',
         SSRB  => '0',
         WEA   => '1',
         WEB   => '0');

   -- VGA radic
   vga: entity work.vga_controller(arch_vga_controller)
      generic map (REQ_DELAY => 4) -- Delay 4 tiky
      port map (
         CLK    => CLK_div,
         RST    => RESET,
         ENABLE => '1',
         MODE   => isettings,

         DATA_RED    => icolor(2 downto 0),
         DATA_GREEN  => icolor(5 downto 3),
         DATA_BLUE   => icolor(8 downto 6),
         ADDR_COLUMN => req_col,
         ADDR_ROW    => req_row,

         VGA_RED   => RED_V,
         VGA_BLUE  => BLUE_V,
         VGA_GREEN => GREEN_V,
         VGA_HSYNC => HSYNC_V,
         VGA_VSYNC => VSYNC_V,

         -- H/V Status
         STATUS_H  => vga_hstate,
         STATUS_V  => vga_vstate);

   -- Row/Col idle
   req_col_mx <= not req_col(9) when ((vga_hstate = hstRequestOnly) or (vga_hstate = hstDraw)) else '0';
   req_row_mx <= '1' when vga_vstate = vstDraw else '0';

   req_draw        <= req_col_mx and req_row_mx;
   req_can_write   <= '1' when (vga_hstate = hstSync) or (vga_hstate = hstWaitSync) else '0';
   req_can_refresh <= '1' when (vga_vstate = vstSync) or (vga_vstate = vstWaitSync) else '0';

   -- SDRAM address source selection
   sdram_addr <= "00000" & buf2sdr_data(17 downto 9) & buf2sdr_data(8 downto 0) when req_write = '1' else
                 "00000" & req_row(8 downto 0) & req_col(8 downto 0);

   -- Data in buffer reading
   sdram_data_in <= buf2sdr_data(25 downto 18);

   -- SDRAM read/write commands
   cmd <= fRead    when req_draw = '1' else
          fWrite   when req_write = '1' else
          fRefresh when req_can_refresh = '1' else
          fNop;

   -- VGA color
   icolor <= sdram_data_out & '0' when req_draw = '1' else (others => '0');

   spi2buf_data <= "0000000000" & spi_data_out & spi_addr;
   buf_step     <= '0' when spi2buf_addr = buf2sdr_addr else '1';
   req_write    <= req_can_write and buf_step and not ram_busy;

   ram_cmd_we <= '1'; -- (CLK_div and not ram_busy and req_draw) or req_write;

   buf_write: process(RESET, CLK)
   begin
      if RESET = '1' then
         spi2buf_addr   <= (others => '0');
      elsif CLK'event and CLK = '1' then
         if (spi_we = '1') then
            spi2buf_addr <= spi2buf_addr + 1;
         end if;
      end if;
   end process;

   buf_read: process(RESET, CLK)
   begin
      if RESET = '1' then
         buf2sdr_addr <= (others => '0');
      elsif CLK'event and CLK = '1' then
         if (req_write = '1') then
            buf2sdr_addr <= buf2sdr_addr + 1;
         end if;
      end if;
   end process;

   clk_d2: BUFG
      port map (
         I => CLK_div_reg,
         O => CLK_div
      );

   -- 50 Mhz / 2 => 25 MHz
   div_clk_2: process(CLK, RESET)
   begin
      if RESET = '1' then
         CLK_div_reg <= '0';
      elsif CLK'event and CLK = '1' then
         CLK_div_reg <= not CLK_div_reg;
      end if;
   end process;

end arch_beh;
