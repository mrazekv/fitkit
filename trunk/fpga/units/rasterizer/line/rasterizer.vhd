-- aclr_line.vhd: vga line rasterization
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

entity rasterizer_line is
   port (
      -- Clock, reset, ...
      CLK     : in std_logic;
      RST     : in std_logic;
      ENABLE  : in std_logic;
      BUSY    : out std_logic;
      
      -- Address/data
      ADDR_ROW    : out std_logic_vector(8 downto 0);    
      ADDR_COLUMN : out std_logic_vector(8 downto 0);    
      ADDR_VLD    : out std_logic; -- Output data valid

      -- Command set
      LINE_X1  : in std_logic_vector(8 downto 0);
      LINE_Y1  : in std_logic_vector(8 downto 0);
      LINE_X2  : in std_logic_vector(8 downto 0);
      LINE_Y2  : in std_logic_vector(8 downto 0);
      LINE_SET : in std_logic
      );
end rasterizer_line; 

architecture arch_rasterizer_line of rasterizer_line is

   type accState is (saIdle, saPreCalc, saPreCalc2, saAlgStep1, saAlgStep2, saAlgStep3);
   
   signal cstate : accState;
   signal nstate : accState;
   
   signal iset  : std_logic;
   signal ibusy : std_logic;

   signal dx : std_logic_vector(8 downto 0);
   signal dy : std_logic_vector(8 downto 0);
   signal x1_reg : std_logic_vector(8 downto 0);
   signal x2_reg : std_logic_vector(8 downto 0);
   signal y1_reg : std_logic_vector(8 downto 0);
   signal y2_reg : std_logic_vector(8 downto 0);
   signal x1_i : std_logic_vector(8 downto 0);
   signal x2_i : std_logic_vector(8 downto 0);
   signal y1_i : std_logic_vector(8 downto 0);
   signal y2_i : std_logic_vector(8 downto 0);
   
   signal x1_j : std_logic_vector(8 downto 0);
   signal x2_j : std_logic_vector(8 downto 0);
   signal y1_j : std_logic_vector(8 downto 0);
   signal y2_j : std_logic_vector(8 downto 0);

   signal as1_a : std_logic_vector(8 downto 0);
   signal as1_b : std_logic_vector(8 downto 0);
   signal as1_y : std_logic_vector(8 downto 0);
   signal as1_e : std_logic;
   signal as2_a : std_logic_vector(8 downto 0);
   signal as2_b : std_logic_vector(8 downto 0);
   signal as2_y : std_logic_vector(8 downto 0);
   signal as2_e : std_logic;
   
   signal cs_pc     : std_logic;
   signal cs_pc2    : std_logic;
   signal cs_as1    : std_logic;
   signal cs_as2    : std_logic;
   signal cs_as3    : std_logic;
   signal steep     : std_logic;
   signal steep_reg : std_logic;
   signal x_mx      : std_logic;
   signal err_mx    : std_logic;
   signal ystep     : std_logic;
   
   signal cx : std_logic_vector(8 downto 0);
   signal cy : std_logic_vector(8 downto 0);
   signal cy_i    : std_logic_vector(8 downto 0);
   signal err_reg : std_logic_vector(9 downto 0);
   signal err     : std_logic_vector(9 downto 0);
   signal err_a   : std_logic_vector(9 downto 0);
   signal err_s   : std_logic_vector(9 downto 0);
   
begin

   -----------------------------------------------------------------------------
   -- Absolute sub

   asub1: entity work.abs_sub
      port map (
         A => as1_a,
         B => as1_b,
         Y => as1_y,
         E => as1_e
      );
      
   asub2: entity work.abs_sub
      port map (
         A => as2_a,
         B => as2_b,
         Y => as2_y,
         E => as2_e
      );

   -----------------------------------------------------------------------------
   -- External logic (busy, set, ...)
   iset <= LINE_SET and not ibusy;
   BUSY <= ibusy;
   
   -----------------------------------------------------------------------------
   -- Output signals
   ADDR_COLUMN <= cy when steep_reg = '1' else cx;
   ADDR_ROW    <= cx when steep_reg = '1' else cy;
  
   -----------------------------------------------------------------------------
   -- Internal logic
   steep <= '1' when as2_y > as1_y else '0';
  
   -- [X,Y] <-> [Y,X]
   x1_i <= LINE_Y1 when steep = '1' else LINE_X1;
   x2_i <= LINE_Y2 when steep = '1' else LINE_X2;
   y1_i <= LINE_X1 when steep = '1' else LINE_Y1;
   y2_i <= LINE_X2 when steep = '1' else LINE_Y2;
   
   -- [X1,Y1] <-> [X2,Y2]
   x1_j <= x2_reg when as1_e = '1' else x1_reg;
   x2_j <= x1_reg when as1_e = '1' else x2_reg;
   y1_j <= y2_reg when as1_e = '1' else y1_reg;
   y2_j <= y1_reg when as1_e = '1' else y2_reg;
  
   -- Next Y value
   cy_i <= cy - '1' when ystep = '1' else cy + '1';
   
   -- Last X value (X = X2)
   x_mx <= '1' when cx = x2_reg else '0';
  
   -- Signed 2*ERR (10bit) > unsigned DX (9bit)
   err_mx <= not err_reg(9) when (err_reg(8 downto 0) & '0') >= ('0' & dx) else '0';
   
   -- Add/sub ERR
   err_a <= err_reg + dy;
   err_s <= err_reg - dx;
   
   -- Next ERR value
   err <= err_a   when cs_as1 = '1' else
          err_reg when cs_as2 = '1' else
          err_s   when cs_as3 = '1' else
          (others => '0');

   -----------------------------------------------------------------------------
   -- Main register values
   main: process(RST, CLK)
   begin
      if RST = '1' then
      elsif CLK'event and CLK = '1' then
         if ENABLE = '1' then
            if iset = '1' then
               x1_reg <= x1_i;
               x2_reg <= x2_i;
               y1_reg <= y1_i;
               y2_reg <= y2_i;
               steep_reg <= steep;
            end if;
            
            if cs_pc = '1' then
               x1_reg <= x1_j;
               x2_reg <= x2_j;
               y1_reg <= y1_j;
               y2_reg <= y2_j;
            end if;

            if cs_pc2 = '1' then
               dx <= as1_y;
               dy <= as2_y;
               ystep <= as2_e;
               cy <= y1_reg;
               cx <= x1_reg;
            end if;
            
            if cs_as1 = '1' then
               cx <= cx + 1;
            end if;
            
            if cs_as3 = '1' then
               cy <= cy_i;
            end if;
         end if;
      end if;
   end process;

   err_i: process(RST, CLK)
   begin
      if RST = '1' then
         err_reg <= (others => '0');
      elsif CLK'event and CLK = '1' then
         if ENABLE = '1' then
            err_reg <= err;
         end if;
      end if;
   end process;

   FSM_CTRL: process(RST, CLK)
   begin
      if RST = '1' then
         cstate <= saIdle;
      elsif CLK'event and CLK = '1' then
         if ENABLE = '1' then
            cstate <= nstate;
         end if;
      end if;
   end process;
   
   FSM: process(cstate, LINE_X1, LINE_X2, LINE_Y1, LINE_Y2, LINE_SET, x1_reg, x2_reg, y1_reg, y2_reg, x_mx, err_mx)
   begin
      ADDR_VLD <= '0';
      nstate <= saIdle;
      ibusy  <= '1';
      as1_a <= LINE_X1;
      as1_b <= LINE_X2;
      as2_a <= LINE_Y1;
      as2_b <= LINE_Y2;
      cs_pc  <= '0';
      cs_pc2 <= '0';
      cs_as1 <= '0';
      cs_as2 <= '0';
      cs_as3 <= '0';
      
      case cstate is
         when saIdle =>
            ibusy    <= '0';
            if LINE_SET = '1' then
               nstate <= saPreCalc;
            end if;
         
         when saPreCalc =>
            as1_a  <= x1_reg;
            as1_b  <= x2_reg;
            cs_pc  <= '1';
            nstate <= saPreCalc2;
            
         when saPreCalc2 =>
            as1_a  <= x1_reg;
            as1_b  <= x2_reg;
            as2_a  <= y1_reg;
            as2_b  <= y2_reg;
            cs_pc2 <= '1';
            nstate <= saAlgStep1;
            
         when saAlgStep1 =>
            cs_as1 <= '1';
            ADDR_VLD <= '1';
            if x_mx = '1' then
               nstate <= saIdle;
            else
               nstate <= saAlgStep2;
            end if;

         when saAlgStep2 =>
            cs_as2 <= '1';
            if err_mx = '1' then
               nstate <= saAlgStep3;
            else
               nstate <= saAlgStep1;
            end if;

         when saAlgStep3 =>
            cs_as3 <= '1';
            nstate <= saAlgStep1;
      end case;
   end process;
     
end arch_rasterizer_line;
