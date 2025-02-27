-- IMP project, Izidor Matusov, xmatus19, source: snake_top.vhd, changes cca 12%
-- Increased colors to 16, changed VRAM component
--
-- tetris_top.vhd : VGA - 64x64 block graphics with pallete of 16 colors
-- Copyright (C) 2008, 2010 Brno University of Technology,
--                          Faculty of Information Technology
-- Author(s): Izidor Matusov <xmatus19 AT stud.fit.vutbr.cz>
--            Zdenek Vasicek <vasicek AT fit.vutbr.cz>
--            Karel Slany    <slany AT fit.vutbr.cz>
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
use work.clkgen_cfg.all;

architecture arch_vga_mode of tlv_pc_ifc is

   signal irgb : std_logic_vector(8 downto 0);
   signal rrow : std_logic_vector(11 downto 0);
   signal rcol : std_logic_vector(11 downto 0);
   signal border: std_ulogic;

   -- Port A VIDEORAM
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
   signal vga_bram_addr    : std_logic_vector (11 downto 0); 
   signal vga_bram_dout    : std_logic_vector (3 downto 0); 
  
   signal vga_mode  : std_logic_vector(60 downto 0); -- default 640x480x60
   
   -- Keyboard is controlled by MCU
   signal spi_kbrd_data_out : std_logic_vector(15 downto 0);
   signal spi_kbrd_data_in  : std_logic_vector(15 downto 0);
   signal spi_kbrd_we       : std_logic;

   -- display
   signal dis_addr     : std_logic_vector(0 downto 0);
   signal dis_data_out : std_logic_vector(15 downto 0);
   signal dis_write_en : std_logic;

   -- SPI interface
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
   
begin
   -- Set graphical mode (640x480, 60 Hz refresh)
   setmode(r640x480x60, vga_mode);
    
   -- spi decoder (videoRAM)
   SPI_adc_ram: SPI_adc
      generic map(
         ADDR_WIDTH => 16,
         DATA_WIDTH => 8,
         ADDR_OUT_WIDTH => 12,
         BASE_ADDR  => 16#8000#, -- memory address range is 0x8000 - 0x9FFF 
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

   spi_vram_data_out(7 downto 4) <= (others => '0');

   blkram_vram0: RAMB16_S4_S4
      port map (
         DOA             => spi_vram_data_in(3 downto 0),
         ADDRA           => spi_vram_addr,
         CLKA            => CLK,
         DIA             => spi_vram_data_out(3 downto 0),
         ENA             => '1',
         SSRA            => '0',
         WEA             => spi_vram_write_en,

         DOB             => vga_bram_dout,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );
      
   -- spi decoder - keyboard
   SPI_adc_kbrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,
         DATA_WIDTH => 16,
         ADDR_OUT_WIDTH => 1,
         BASE_ADDR  => 16#02# 
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => open,
         DATA_IN  => spi_kbrd_data_in,
         WRITE_EN => open,
         READ_EN  => open
      );

   -- Keyboard controller
   kbrd_ctrl: entity work.keyboard_controller(arch_keyboard)
      port map (
         CLK => SMCLK,
         RST => RESET,

         DATA_OUT => spi_kbrd_data_in(15 downto 0),
         DATA_VLD => open,
         
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

   -- SPI decoder for display
   spidecd: SPI_adc
         generic map (
            ADDR_WIDTH => 8,
            DATA_WIDTH => 16,
            ADDR_OUT_WIDTH => 1,
            BASE_ADDR  => 16#00#
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

   -- LCD display controller
   lcdctrl: lcd_raw_controller
         port map (
            RST    =>  RESET,
            CLK    =>  CLK, -- 25MHz

            -- control singals
            DATA_IN  => dis_data_out,
            WRITE_EN => dis_write_en,

            --- display's signals
            DISPLAY_RS   => LRS,
            DISPLAY_DATA => LD,
            DISPLAY_RW   => LRW,
            DISPLAY_EN   => LE
         );
         
   -- VGA controller, delay 1 tact
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

   -- Convert request for VGA (row, column) into VideoRAM address
   vga_bram_addr <= rrow(8 downto 3) & rcol(8 downto 3);

  
   -- Choose color based on value returned from memory
   select_color: process(vga_bram_dout)
   begin
    if rrow(11 downto 9) /= "000" or rcol(11 downto 9) /= "000" or rcol(8 downto 0) = "000000000" then
        -- eliminate requests not within range 64x64 blocks
        irgb <= "000000000";
    else
        -- Decode color from palette
        case vga_bram_dout is
            -- Wall (white)
            when "0001" => irgb <= "111111111";

            -- Fallen blocks + drawings (gray)
            when "0010" => irgb <= "100100100";

            -- Random colors begin here
            when "0011" => irgb <= "111111000"; -- yellow
            when "0100" => irgb <= "000000111"; -- blue
            when "0101" => irgb <= "110100111"; -- light magenta
            when "0110" => irgb <= "000100010"; -- dark green
            when "0111" => irgb <= "100111000"; -- light green
            when "1000" => irgb <= "100111111"; -- light cyan
            when "1001" => irgb <= "000111000"; -- green
            when "1010" => irgb <= "000100111"; -- light blue
            when "1011" => irgb <= "111001000"; -- light red
            when "1100" => irgb <= "110100011"; -- light brown
            when "1101" => irgb <= "111000000"; -- red
            when "1110" => irgb <= "100001110"; -- magenta
            when "1111" => irgb <= "000011101"; -- cyan

            -- Otherwise black colors
            when others =>
               irgb <= "000000000";
        end case;
    end if; 
   end process;

end arch_vga_mode;
