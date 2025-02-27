-- pexeso_top.vhd : Pexeso - simple FPGA game
-- Copyright (C) 2011 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Vojtech Mrazek <xmraze06 AT fit.vutbr.cz>
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

architecture behav of tlv_pc_ifc is

   -- VGA signals
   signal vga_mode   : std_logic_vector(60 downto 0); -- default 640x480x60
   signal vga_row    : std_logic_vector(11 downto 0);
   signal vga_col    : std_logic_vector(11 downto 0);
   
   signal vga_rgb    : std_logic_vector(8 downto 0);
   
   signal sym_color  : std_logic_vector(3 downto 0);
   
   signal sym_rd_addr : std_logic_vector(3 downto 0);
   signal sym_rd_data : std_logic_vector(3 downto 0);
   signal sym_wr_addr : std_logic_vector(3 downto 0);
   signal sym_wr_data : std_logic_vector(3 downto 0);
   signal sym_we      : std_logic;
   signal sym_gen     : std_logic;
   
   signal sym_vga_sym : std_logic_vector(3 downto 0);
   signal sym_vga_addr : std_logic_vector(3 downto 0);
   
   -- cursor signals
   signal cur_x   : std_logic_vector(1 downto 0) := "00";
   signal cur_y   : std_logic_vector(1 downto 0) := "00"; 
   
   signal kbrd_data_out : std_logic_vector(15 downto 0);
   signal kbrd_data_vld : std_logic;
   
   signal ctl_enter, ctl_reset : std_logic;
   
   -- character decoder
   signal char_symbol : std_logic_vector(3 downto 0) := "1010";
   signal char_data : std_logic;
   
   -- bcd counter
   signal bcd_clk, bcd_reset : std_logic;
   signal bcd_data0, bcd_data1 : std_logic_vector(3 downto 0);
begin
   -- ----------------------------------------------------------
   -- ------------- GLOBAL MODULES -----------------------------
   -- ----------------------------------------------------------
   
   -- VGA controller, delay 1 tact
   vga: entity work.vga_controller(arch_vga_controller) 
      generic map (REQ_DELAY => 1)
      port map (
         CLK    => CLK, 
         RST    => RESET,
         ENABLE => '1',
         MODE   => vga_mode,

         DATA_RED    => vga_rgb(8 downto 6) ,
         DATA_GREEN  => vga_rgb(5 downto 3),
         DATA_BLUE   => vga_rgb(2 downto 0),
         ADDR_COLUMN => vga_col,
         ADDR_ROW    => vga_row,

         VGA_RED   => RED_V,
         VGA_BLUE  => BLUE_V,
         VGA_GREEN => GREEN_V,
         VGA_HSYNC => HSYNC_V,
         VGA_VSYNC => VSYNC_V
      );
   -- Set graphical mode (640x480, 60 Hz refresh)
   setmode(r640x480x60, vga_mode);
   
   -- char 2 vga decoder
   chardec : entity work.char_rom
      port map (
         ADDRESS => char_symbol,
         ROW => vga_row(3 downto 0),
         COLUMN => vga_col(2 downto 0),
         DATA => char_data
      );
   
   sym: entity work.symbol_decoder
      port map (
         CLK => CLK,
         SYMBOL => sym_vga_sym, 
      
         ROW => vga_row(5 downto 1),
         COL => vga_col(5 downto 1),
      
         COLOR => sym_color
      );
      
   symram: entity work.symbol_ram 
      generic map (
         ADDRESS_SIZE => 4,
         SYMBOL_SIZE  => 4
      )
      port map (
         CLK => CLK,
         RESET => RESET,
         GEN => sym_gen,
         
         -- Port pro CTL
         ADDR_A => sym_rd_addr,
         DATA_A => sym_rd_data,
         
         
         -- Port pro zapis
         ADDR_W => sym_wr_addr,
         DATA_W => sym_wr_data,
         WE     => sym_we, 
         
         -- Port pro VGA
         ADDR_B => sym_vga_addr,
         DATA_B => sym_vga_sym
         
      );
      
   -- Keyboard controller
   kbrd_ctrl: entity work.keyboard_controller(arch_keyboard)
      port map (
         CLK => SMCLK,
         RST => RESET,

         DATA_OUT => kbrd_data_out(15 downto 0),
         DATA_VLD => kbrd_data_vld,
         
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );
      
   -- BCD counter
   bcd_cnt: entity work.bcd_cnt
      port map (
         CLK => bcd_clk,
         RESET => bcd_reset,
         
         DATA0 => bcd_data0,
         DATA1 => bcd_data1
      );
      
   -- FSM for game
   game_fsm : entity work.game_ctl
      port map (
         CLK => CLK,
         RESET => RESET,
         
         -- cursor
         CUR_X => cur_x,
         CUR_Y => cur_y,
         
         ENTER   => ctl_enter,
         RESTART => ctl_reset,
         
         RAM_GEN => sym_gen, 
         CNT_CLK => bcd_clk,
         CNT_RST => bcd_reset,
         
         -- Port pro CTL
         RAM_ADDR_A => sym_rd_addr,
         RAM_DATA_A => sym_rd_data,
         
         
         -- Port pro zapis
         RAM_ADDR_W => sym_wr_addr,
         RAM_DATA_W => sym_wr_data,
         RAM_WE     => sym_we
         
      );
   
   -- cursor controller, move to CLK
   cursor: process
      variable in_access : std_logic := '0';
   begin
      if CLK'event and CLK='1' then
         ctl_enter <= '0';
         ctl_reset <= '0';
                  
         if in_access='0' then
            if kbrd_data_vld='1' then 
               in_access:='1';
               if kbrd_data_out(9)='1' and cur_x /= "11" then  -- key 6
                  cur_x <= cur_x+1;
               elsif kbrd_data_out(1)='1' and cur_x /= "00" then  -- key 4
                  cur_x <= cur_x-1;
               elsif kbrd_data_out(4)='1' and cur_y /= "00" then  -- key 2
                  cur_y <= cur_y-1;
               elsif kbrd_data_out(6)='1' and cur_y /= "11" then  -- key 8
                  cur_y <= cur_y+1;
               elsif kbrd_data_out(5)='1' then     -- key 5
                  ctl_enter <= '1';                
               
               elsif kbrd_data_out(12)='1' then    -- key A
                  ctl_reset <= '1';
                   
                          
               end if;
            end if;
         else
            if kbrd_data_vld='0' then 
               in_access:='0';
            end if;
         end if;
      end if;
      
   end process;
   

   
   setgraph : process (vga_row, vga_col)
      variable in_row : std_logic := '0';
      variable in_col : std_logic := '0';
      variable in_nmr_row, in_sym_1, in_sym_2 : std_logic;
      
      variable col_id : std_logic_vector(3 downto 0);
   begin
      if CLK'event and CLK='1' then
         vga_rgb <= "000000000";    -- default color
         if in_sym_1 = '1' then
            char_symbol <= bcd_data1;
         else
            char_symbol <= bcd_data0;
         end if;
         
         if vga_row=0 or vga_row=386 then
            in_row :='0';
         elsif vga_row=128 then
            in_row :='1';
         end if;
         
         if vga_col=0 or vga_col=450 then
            in_col :='0';
         elsif vga_col=192 then
            in_col :='1';
         end if;
         
         if vga_col=0 or vga_col=447 then
            in_sym_1 := '0';
            in_sym_2 := '0';
         elsif vga_col=431 then
            in_sym_1 := '1';
         elsif vga_col=439 then
            in_sym_1 := '0';
            in_sym_2 := '1';
         end if;
         
         if vga_row=0 or vga_row=416 then
            in_nmr_row:='0';
         elsif vga_row=400 then
            in_nmr_row:='1';
         end if;
         
         col_id := vga_col(9 downto 6) - 3;
         sym_vga_addr <= not vga_row(7) & vga_row(6) & col_id(1 downto 0);
         
         
         if in_row = '1' and in_col = '1' then 
            if vga_row(5 downto 1)="00000" or vga_col(5 downto 1)="00000" then
                  vga_rgb <= "100100100";
            else
               -- GRID
               if  not vga_row(7) & vga_row(6) & col_id(1 downto 0) =cur_y & cur_x and vga_row(1)='1' and vga_col(1)='1' then
                  vga_rgb <= "111000000";
               else
                  -- CARD
            		case sym_color is
            			when "0000" => vga_rgb <= "000000000"; -- black
            		   when "0001" => vga_rgb <= "000000111"; -- blue
                     when "0010" => vga_rgb <= "100100100"; -- gray
                     when "0011" => vga_rgb <= "111111000"; -- yellow
                     when "0100" => vga_rgb <= "111000000"; -- red
                     when "0101" => vga_rgb <= "110100111"; -- light magenta
                     when "0110" => vga_rgb <= "000100010"; -- dark green
                     when "0111" => vga_rgb <= "100111000"; -- light green
                     when "1000" => vga_rgb <= "100001110"; -- magenta
                     when "1010" => vga_rgb <= "000100111"; -- light blue
            			when others => vga_rgb <= "111111111"; -- white
            		end case;
         		end if;
         	end if;
         elsif in_nmr_row='1' and (in_sym_1 = '1' or in_sym_2 = '1') then
            if char_data = '1' then
               vga_rgb <= "111111111"; -- white
            end if;
         end if; 
		end if;
   end process;
   
   
   X(13) <= '1';
   X(15) <= '0';
   
   
end behav;
