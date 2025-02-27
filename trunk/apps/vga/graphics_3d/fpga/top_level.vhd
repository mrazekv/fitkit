-- top_level.vhd: triangle rasterization demo
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

-- pragma translate_off 
use ieee.std_logic_textio.all;
use std.textio.all;

library unisim;
use unisim.vcomponents.all;
-- pragma translate_on 

architecture arch_beh of tlv_pc_ifc is

   -- Other signals
   signal debug : std_logic;

   -- SDRAM signals     
   signal sdram_addr     : std_logic_vector(22 downto 0); 
   signal sdram_data_out : std_logic_vector(7 downto 0);
   signal sdram_data_in  : std_logic_vector(7 downto 0);
   signal sdram_ra       : std_logic_vector(13 downto 0);
   signal sdram_cmd      : sdram_func;
   signal sdram_busy     : std_logic;
   signal sdram_we       : std_logic;
   
   -- Timing signals
   signal CLK_div       : std_logic;
   signal CLK_div_reg   : std_logic;
     
   -- VGA signals
   signal isettings   : std_logic_vector(60 downto 0);
   signal icolor      : std_logic_vector(8 downto 0) := (others => '0');
   signal req_row     : std_logic_vector(11 downto 0);
   signal req_col     : std_logic_vector(11 downto 0);
   signal req_col_mx  : std_logic;
   signal req_row_mx  : std_logic;
   signal vga_hstate  : vga_hfsm_state;
   signal vga_vstate  : vga_vfsm_state;
   signal vga_period  : std_logic;
      
   -- Acc to SDRAM FIFO
   signal buf2acc_reg       : std_logic_vector(53 downto 0);
   signal buf2sdr_data      : std_logic_vector(35 downto 0);
   signal acc2buf_data      : std_logic_vector(35 downto 0);
   signal row               : std_logic_vector(8 downto 0);
   signal col               : std_logic_vector(8 downto 0);
   signal acc2buf_we        : std_logic;
   signal accbuf_nfull      : std_logic;
   signal acc_en            : std_logic;
   signal pixel_req         : std_logic;
   signal grfifo_vld        : std_logic;
   signal grfifo_vld_reg    : std_logic;

   -- SPI to DATA FIFO
   signal spi_data          : std_logic_vector(63 downto 0);
   signal spi_temp          : std_logic_vector(27 downto 0);
   signal buf2acc_data      : std_logic_vector(35 downto 0);
   signal spi2buf_data      : std_logic_vector(35 downto 0);
   signal spi_we            : std_logic;
   signal spi2buf_we        : std_logic; 
   signal spi2buf_re        : std_logic; 
   signal spi2buf_we_reg    : std_logic; 
   signal spi2buf_we_reg2   : std_logic; 
   signal spi2buf_vld_reg   : std_logic; 
   signal spibuf_step       : std_logic;
   signal dtfifo_vld        : std_logic;
   signal dti               : std_logic; 
   
   -- SDRAM sharing signals
   signal write_time_mx   : std_logic;
   signal sdr_req         : std_logic_vector(3 downto 0);
   signal sdr_req_read    : std_logic;
   signal sdr_req_write   : std_logic;
   signal sdr_req_refresh : std_logic;
   signal spi_cls_reg     : std_logic;
   
   -- ACC signals
   signal acc_busy      : std_logic;
   signal buf2acc_set   : std_logic;
   signal vga_color     : std_logic_vector(7 downto 0);
   
   -- Clear SDRAM (screen)
   signal cls_reg       : std_logic;
   signal cls           : std_logic;

   -- Global line
   component BUFG 
      port (
         I: in  std_logic;
         O: out std_logic
      ); 
   end component;
   
-- pragma translate_off
   file log_file : text open write_mode is "fifo_out.txt";
-- pragma translate_on

   signal vx0 : std_logic_vector(8 downto 0);
   signal vy0 : std_logic_vector(8 downto 0);
   signal vx1 : std_logic_vector(8 downto 0);
   signal vy1 : std_logic_vector(8 downto 0);
   signal vx2 : std_logic_vector(8 downto 0);
   signal vy2 : std_logic_vector(8 downto 0);
   
begin

-- pragma translate_off
   textsave: process(CLK)
   variable line_out: Line;
   variable a: std_logic_vector(8 downto 0);
   begin
      if CLK'event and CLK = '1' then
         if grfifo_vld = '1' then
            a := buf2sdr_data(17 downto 9);
            write(line_out, conv_integer(a));

            write(line_out, string'(" "));
            a := buf2sdr_data(8 downto 0);
            write(line_out, conv_integer(a));

            write(line_out, string'(": "));
            write(line_out, conv_integer(sdram_data_in));
            writeline(log_file, line_out);
         end if;
      end if;
   end process;
-- pragma translate_on

   -----------------------------------------------------------------------------
   -- Debug LED & reset logic
   LEDF <= debug;

   clk_VGA: BUFG
      port map (
         I => CLK_div_reg,
         O => CLK_div
      );

   -- VGA need => 25 MHz  
   div_clk: process(CLK, RESET)
   begin
      if RESET = '1' then
         CLK_div_reg <= '0';
      elsif CLK'event and CLK = '1' then
         CLK_div_reg <= not CLK_div_reg;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- SPI control  
   spidecd: entity work.SPI_adc(basic)
      generic map (
         ADDR_WIDTH => 8,      -- sirka adresy 8 bitu
         DATA_WIDTH => 64,     -- sirka dat 64 bitu (C Y2 X2 Y1 X2 Y3 X3 -> 8b + 6x9b)
         ADDR_OUT_WIDTH => 1,  -- sirka adresy min 1 bit
         BASE_ADDR  => 16#00000000# -- adresovy prostor od 0x0000
      )
      port map (
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_OUT => spi_data,
         DATA_IN  => "0000000000000000000000000000000000000000000000000000000000000000",
         WRITE_EN => spi_we,
         READ_EN  => open
      );
   
   -----------------------------------------------------------------------------
   -- Main shared SDRAM memory control
   RA <= sdram_ra(13 downto 12) & '0' & sdram_ra(11 downto 0);
   sdram: entity work.sdram_raw_controller
      generic map (
         -- Generovani prikazu refresh radicem automaticky
         GEN_AUTO_REFRESH => false
      )
      port map (
         -- Adresa
         ADDR_ROW    => sdram_addr(20 downto 9),
         ADDR_COLUMN => sdram_addr(8 downto 0),
         BANK        => sdram_addr(22 downto 21),
   
         -- Pozadavek + jeho potvrzeni
         CMD      => sdram_cmd,
         CMD_WE   => sdram_we,

         -- Hodiny, reset, ...
         CLK      => CLK,
         RST      => RESET,
         ENABLE   => '1',
         BUSY     => sdram_busy,
   
         -- Data
         DATA_OUT    => sdram_data_out,
         DATA_VLD    => open,
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

   -----------------------------------------------------------------------------
   -- FIFOs

   -- Pixel graphic fifo
   -- 10b RESERVER  8b COLOR  9b YPOS  9b XPOS
   GRBUF: entity work.fifo36
     port map (
         CLK  => CLK,
         RST  => RESET,
         DATA_IN => acc2buf_data,  -- Data in
         DATA_WE => acc2buf_we,    -- Data write
         DATA_OUT => buf2sdr_data, -- Data out
         DATA_VLD => grfifo_vld,   -- Reqeusted data set 
         FIFO_NEXT => pixel_req,   -- Req read
         FIFO_FREE => accbuf_nfull -- Stop work
     );   

   -- Data struct fifo
   -- 9b Y2   9b X2   9b Y1   9b X1
   --         8b CLR  9b X3   9b Y3
   DTBUF: entity work.fifo36
     port map (
         CLK  => CLK,
         RST  => RESET,
         DATA_IN => spi2buf_data,   -- Data in
         DATA_WE => spibuf_step,    -- Data write
         DATA_OUT => buf2acc_data,  -- Data out
         DATA_VLD => dtfifo_vld,    -- Reqeusted data set 
         FIFO_NEXT => spi2buf_re,   -- Req read
         FIFO_FREE => open          -- Stop work
     );
   
   -- Pixel fifo control  
   grfifo_vld_p: process(CLK, RESET)
   begin
      if RESET = '1' then
         grfifo_vld_reg <= '0';
      elsif CLK'event and CLK = '1' then
         grfifo_vld_reg <= (grfifo_vld_reg or grfifo_vld) and sdram_busy; -- or spi_cls_reg or cls_reg);
      end if;
   end process;
   
   -- Data fifo logic
   spi2buf_we   <= spi_we and not spi2buf_we_reg;
   spi2buf_data <= spi_data(35 downto 0) when spi2buf_we_reg = '0'
              else "00000000" & spi_temp;
   spibuf_step  <= spi2buf_we or spi2buf_we_reg2;
   spi2buf_re <= (not spi2buf_we and not acc_busy and not spi2buf_vld_reg);

   -- Data fifo input control
   datbuf_write: process(RESET, CLK)
   begin
      if RESET = '1' then
         spi2buf_we_reg  <= '0';
         spi2buf_we_reg2 <= '0';
         spi_temp        <= (others => '0');
      elsif CLK'event and CLK = '1' then
         -- Set color buffer
         if (spi2buf_we = '1') then
            spi_temp <= spi_data(63 downto 36);
         end if;
         -- Write identify reg
         spi2buf_we_reg <= spi_we;
         spi2buf_we_reg2 <= spi2buf_we;
      end if;
   end process;
   
   -- Data buffer output control
   datbuf_read: process(RESET, CLK)
   begin
      if RESET = '1' then
         buf2acc_reg <= (others => '0');
         vga_color   <= (others => '0');
         buf2acc_set <= '0';
         dti         <= '0';
         spi_cls_reg <= '0';
         debug <= '0';
         spi2buf_vld_reg <= '0';
      elsif CLK'event and CLK = '1' then
         spi2buf_vld_reg <= dtfifo_vld;
         buf2acc_set <= dtfifo_vld and dti;
         dti <= dtfifo_vld and not dti;
         if (dtfifo_vld and not dti) = '1' then
            buf2acc_reg(35 downto 0)  <= buf2acc_data;
         end if;
         if (dtfifo_vld and dti) = '1' then
            buf2acc_reg(53 downto 36) <= buf2acc_data(17 downto 0);
            vga_color   <= buf2acc_data(27 downto 20);
         end if;
         -- Set clear screen
         spi_cls_reg <= (spi_cls_reg and not vga_period) or (buf2acc_data(19) and dtfifo_vld and dti);
         debug <= acc_en;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Main rasterizer component instance & logic
   acc2buf_data(35 downto 26) <= (others => '0');
   -- Always VGA/black color
   acc2buf_data(25 downto 18) <= vga_color;
   acc2buf_data(17 downto 9)  <= row(8 downto 0);
   acc2buf_data(8 downto 0)   <= col(8 downto 0);
   
   vx0 <= buf2acc_reg(8 downto 0);
   vy0 <= buf2acc_reg(17 downto 9);
   vx1 <= buf2acc_reg(26 downto 18);
   vy1 <= buf2acc_reg(35 downto 27);
   vx2 <= buf2acc_reg(44 downto 36);  
   vy2 <= buf2acc_reg(53 downto 45);
   
   acc_en <= accbuf_nfull; -- and cls_reg;-- and not spi_cls_reg;
   
   -- Rasterize komponent
   draw: entity work.rasterizer_triangle
      port map (
         -- Clock, reset, ...
         CLK     => CLK,
         RST     => RESET,
         ENABLE  => acc_en,
         BUSY    => acc_busy,

         -- Address/data
         ADDR_ROW    => row,
         ADDR_COLUMN => col,
         ADDR_VLD    => acc2buf_we,

         -- Command set
         VERTEX_X1  => vx0,
         VERTEX_Y1  => vy0,
         VERTEX_X2  => vx1,
         VERTEX_Y2  => vy1,
         VERTEX_X3  => vx2,
         VERTEX_Y3  => vy2,
         VERTEX_SET => buf2acc_set
         );
   
   -----------------------------------------------------------------------------
   -- Graphic output control
   SetMode(r640x480x60, isettings);

   -- VGA
   vga: entity work.vga_controller
      generic map (REQ_DELAY => 4) -- Delay 4
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

   -- VGA color
   icolor <= sdram_data_out & '0' when sdr_req_read = '1' else (others => '0');

   -----------------------------------------------------------------------------
   -- SDRAM sharing logic

   -- Row/Col idle (SDRAM read/write time)
   req_col_mx <= '1' when ((vga_hstate = hstRequestOnly) or (vga_hstate = hstDraw)) else '0'; 
   req_row_mx <= '1' when vga_vstate = vstDraw else '0';

   -- Write time block mx
   write_time_mx <= '1' when ((vga_hstate = hstWaitSync) or (vga_hstate = hstSync)) else '0';

   -- Sharing read/write/refresh
   vga_period      <= '1' when (vga_vstate = vstWaitPeriod) else '0';
   sdr_req_refresh <= '1' when ((vga_vstate = vstSync) or (vga_vstate = vstWaitSync)) else '0';
   sdr_req_read    <= req_col_mx and not req_col(9) and req_row_mx and cls_reg;
   sdr_req_write   <= grfifo_vld or grfifo_vld_reg;
   
   -- Req pixel from buffer
   pixel_req       <= req_row_mx and write_time_mx and not sdram_busy and cls_reg and not spi_cls_reg;
   
   -- SDR write
   sdram_we <= sdr_req_refresh or sdr_req_write or sdr_req_read or (not cls_reg);
   
   -- SDRAM address source selection
   sdram_addr(22 downto 18) <= "00000";
   sdram_addr(17 downto 0) <= buf2sdr_data(17 downto 0) when sdr_req_write = '1' else
                              req_row(8 downto 0) & req_col(8 downto 0);

   -- SDRAM data in (color)
   sdram_data_in <= buf2sdr_data(25 downto 18) when cls_reg = '1' else
                    (others => '0');

   -- SDRAM read/write shared commands   
   sdr_req <= sdr_req_read & sdr_req_write & sdr_req_refresh & cls_reg;
   with sdr_req select
      sdram_cmd <= fWrite   when "0000", -- Clear screen
                   fRead    when "1001", -- Read (VGA)
                   fWrite   when "0100", -- Clear screen
                   fWrite   when "0101", -- Write (new)
                   fRefresh when "0010", -- Refresh
                   fRefresh when "0011", -- Refresh
                   fNop when others;
   
   -----------------------------------------------------------------------------
   -- Clear screen (SDRAM) fiRESET
   -- logic
   cls <= (cls_reg or sdr_req_refresh) and (spi_cls_reg nand vga_period);
   -- register
   cls_i: process(RESET, CLK)
   begin
     if RESET = '1' then
        cls_reg <= '0';
     elsif CLK'event and CLK = '1' then
        cls_reg <= cls;
     end if;
   end process;

end arch_beh;
