-- rasterizer.vhd: vga triangle rasterization
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

--------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_signed.all;

-- pragma translate_off
library unisim;
use unisim.vcomponents.all;
-- pragma translate_on

entity rasterizer_triangle is
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
      VERTEX_X1  : in std_logic_vector(8 downto 0);
      VERTEX_Y1  : in std_logic_vector(8 downto 0);
      VERTEX_X2  : in std_logic_vector(8 downto 0);
      VERTEX_Y2  : in std_logic_vector(8 downto 0);
      VERTEX_X3  : in std_logic_vector(8 downto 0);
      VERTEX_Y3  : in std_logic_vector(8 downto 0);
      VERTEX_SET : in std_logic
      );
end rasterizer_triangle;

architecture arch_rasterizer_triangle of rasterizer_triangle is

   type accState is (saIdle,
                     -- Coefficients calculation
                     saSetCoef0, saSetCoef1, saSetCoef2, saSetCoef3, saSetCoef4, saSetCoef5, saSetCoef6, saSetCoef7, saSetCoef8,
                     -- Coefficients correction
                     saCrr0, saCrr1, saCrr2, saCrr3, saCrr4, saCrr5, saCrr6, saCrr7, saCrr8,
                     saCrr2_i1, saCrr2_i2, saCrr2_i3,
                     saCrr5_i1, saCrr5_i2, saCrr5_i3,
                     saCrr8_i1, saCrr8_i2, saCrr8_i3,
                     -- Point test
                     saTest0, saTest1, saTest2);

   signal cstate : accState;
   signal nstate : accState;

   signal iset  : std_logic;
   signal ibusy : std_logic;

   signal vx0 : std_logic_vector(8 downto 0);
   signal vy0 : std_logic_vector(8 downto 0);
   signal vx1 : std_logic_vector(8 downto 0);
   signal vy1 : std_logic_vector(8 downto 0);
   signal vx2 : std_logic_vector(8 downto 0);
   signal vy2 : std_logic_vector(8 downto 0);

   signal x_cnt   : std_logic_vector(8 downto 0);
   signal y_cnt   : std_logic_vector(8 downto 0);
   signal x_mx    : std_logic;
   signal y_mx    : std_logic;

   --- Math signals
   signal mult1_a : std_logic_vector(9 downto 0);
   signal mult1_b : std_logic_vector(9 downto 0);
   signal mult1_y : std_logic_vector(35 downto 0);
   signal mult2_a : std_logic_vector(9 downto 0);
   signal mult2_b : std_logic_vector(9 downto 0);
   signal mult2_y : std_logic_vector(35 downto 0);

   signal sa1_a   : std_logic_vector(9 downto 0);
   signal sa1_b   : std_logic_vector(9 downto 0);
   signal sa1_y   : std_logic_vector(9 downto 0);
   signal sa3_y   : std_logic_vector(18 downto 0);
   signal sa4_a   : std_logic_vector(21 downto 0);
   signal sa4_b   : std_logic_vector(21 downto 0);
   signal sa4_y   : std_logic_vector(21 downto 0);
   signal sa5_a   : std_logic_vector(21 downto 0);
   signal sa5_b   : std_logic_vector(21 downto 0);
   signal sa5_y   : std_logic_vector(21 downto 0);
   signal sa6_a   : std_logic_vector(21 downto 0);
   signal sa6_b   : std_logic_vector(21 downto 0);
   signal sa6_y   : std_logic_vector(21 downto 0);

   signal not_a : std_logic_vector(18 downto 0);
   signal not_y : std_logic_vector(18 downto 0);

   signal a1_reg : std_logic_vector(9 downto 0);
   signal a2_reg : std_logic_vector(9 downto 0);
   signal a3_reg : std_logic_vector(9 downto 0);
   signal b1_reg : std_logic_vector(9 downto 0);
   signal b2_reg : std_logic_vector(9 downto 0);
   signal b3_reg : std_logic_vector(9 downto 0);
   signal c1_reg : std_logic_vector(18 downto 0);
   signal c2_reg : std_logic_vector(18 downto 0);
   signal c3_reg : std_logic_vector(18 downto 0);
   signal acc_reg1 : std_logic_vector(21 downto 0);
   signal acc_reg2 : std_logic_vector(21 downto 0);
   signal acc_reg3 : std_logic_vector(21 downto 0);
   signal bck_reg1 : std_logic_vector(21 downto 0);
   signal bck_reg2 : std_logic_vector(21 downto 0);
   signal bck_reg3 : std_logic_vector(21 downto 0);

   signal cs_saSetCoef0 : std_logic;
   signal cs_saSetCoef1 : std_logic;
   signal cs_saSetCoef2 : std_logic;
   signal cs_saSetCoef3 : std_logic;
   signal cs_saSetCoef4 : std_logic;
   signal cs_saSetCoef5 : std_logic;
   signal cs_saSetCoef6 : std_logic;
   signal cs_saSetCoef7 : std_logic;
   signal cs_saSetCoef8 : std_logic;
   signal cs_saCrr1 : std_logic;
   signal cs_saCrr4 : std_logic;
   signal cs_saCrr7 : std_logic;
   signal cs_saTest0 : std_logic;
   signal cs_saTest1 : std_logic;
   signal cs_saTest2 : std_logic;
   signal cs_saCrr2_i1 : std_logic;
   signal cs_saCrr2_i2 : std_logic;
   signal cs_saCrr2_i3 : std_logic;
   signal cs_saCrr5_i1 : std_logic;
   signal cs_saCrr5_i2 : std_logic;
   signal cs_saCrr5_i3 : std_logic;
   signal cs_saCrr8_i1 : std_logic;
   signal cs_saCrr8_i2 : std_logic;
   signal cs_saCrr8_i3 : std_logic;

   signal mlt1_i : std_logic_vector(17 downto 0);
   signal mlt2_i : std_logic_vector(17 downto 0);
   signal mlt1_j : std_logic_vector(17 downto 0);
   signal mlt2_j : std_logic_vector(17 downto 0);

   component MULT18X18S
      port (P  : out std_logic_vector (35 downto 0);
            A  : in std_logic_vector (17 downto 0);
            B  : in std_logic_vector (17 downto 0);
            C  : in STD_ULOGIC;
            CE : in STD_ULOGIC;
            R  : in STD_ULOGIC);
   end component;

begin

   -----------------------------------------------------------------------------
   -- External logic (busy, set, ...)
   iset <= VERTEX_SET and not ibusy;
   BUSY <= ibusy;

   -----------------------------------------------------------------------------
   -- Output signals
   ADDR_VLD    <= cs_saTest1 and not (acc_reg1(acc_reg1'high) or acc_reg2(acc_reg2'high) or acc_reg3(acc_reg3'high));
   ADDR_COLUMN <= x_cnt;
   ADDR_ROW    <= y_cnt;

   -----------------------------------------------------------------------------
   -- Math
   --
   -- MULT1  s10*s10  IWs10    OWs20     shared-6
   -- MULT2  s10*s10  IWs10    OWs20     shared-6
   -- 2xSUB  s10-s10  IWs10    OWs10     shared-3/3
   -- 1xSUB  s19-s19  IWs19    OWs19     shared-3
   -- 3xADD  s22+s22  IWs22    OWs22     shared-8/2/2

   mlt1_i <= mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a(mult1_a'high) & mult1_a;
   mlt1_j <= mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b(mult1_b'high) & mult1_b;
   mlt2_i <= mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a(mult2_a'high) & mult2_a;
   mlt2_j <= mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b(mult2_b'high) & mult2_b;

   mult1: MULT18X18S
      port map (
         P  => mult1_y,
         A  => mlt1_i,
         B  => mlt1_j,
         C  => CLK,
         CE => '1',
         R  => RST
      );

   mult2: MULT18X18S
      port map (
         P  => mult2_y,
         A  => mlt2_i,
         B  => mlt2_j,
         C  => CLK,
         CE => '1',
         R  => RST
      );

   -- ADDERS
   sa1_y <= sa1_a - sa1_b;
   sa3_y <= (mult1_y(35) & mult1_y(17 downto 0)) - (mult2_y(35) & mult2_y(17 downto 0));
   sa4_y <= sa4_a + sa4_b;
   sa5_y <= sa5_a + sa5_b;
   sa6_y <= sa6_a + sa6_b;
   not_y <= "0000000000000000000" - not_a;

   -----------------------------------------------------------------------------
   -- Counter limits
   x_mx <= '1' when x_cnt = "111111111" else '0';
   y_mx <= '1' when y_cnt = "111111111" else '0';
   
   -----------------------------------------------------------------------------
   -- Main register values
   main: process(RST, CLK)
   begin
      if RST = '1' then
         x_cnt  <= (others => '0');
         y_cnt  <= (others => '0');
         a1_reg <= (others => '0');
         a2_reg <= (others => '0');
         a3_reg <= (others => '0');
         b1_reg <= (others => '0');
         b2_reg <= (others => '0');
         b3_reg <= (others => '0');
         c1_reg <= (others => '0');
         c2_reg <= (others => '0');
         c3_reg <= (others => '0');
         vx0 <= (others => '0');
         vy0 <= (others => '0');
         vx1 <= (others => '0');
         vy1 <= (others => '0');
         vx2 <= (others => '0');
         vy2 <= (others => '0');
      elsif CLK'event and CLK = '1' then
         if ENABLE = '1' then
            if iset = '1' then
               vx0    <= VERTEX_X1;
               vy0    <= VERTEX_Y1;
               vx1    <= VERTEX_X2;
               vy1    <= VERTEX_Y2;
               vx2    <= VERTEX_X3;
               vy2    <= VERTEX_Y3;
            end if;

            -- Coefficients
            if cs_saSetCoef0 = '1' then
               a1_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef1 = '1' then
               b1_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef2 = '1' then
               c1_reg  <= sa3_y(sa3_y'high) & sa3_y(17 downto 0);
            end if;
            if cs_saSetCoef3 = '1' then
               a2_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef4 = '1' then
               b2_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef5 = '1' then
               c2_reg  <= sa3_y(sa3_y'high) & sa3_y(17 downto 0);
            end if;
            if cs_saSetCoef6 = '1' then
               a3_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef7 = '1' then
               b3_reg  <= sa1_y(sa1_y'high) & sa1_y(8 downto 0);
            end if;
            if cs_saSetCoef8 = '1' then
               c3_reg  <= sa3_y(sa3_y'high) & sa3_y(17 downto 0);
            end if;

            -- Corrections
            if (cs_saCrr1 or cs_saCrr4 or cs_saCrr7) = '1' then
               acc_reg1 <= sa4_y;
            end if;
            if cs_saCrr2_i1 = '1' then
               a1_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr2_i2 = '1' then
               b1_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr2_i3 = '1' then
               c1_reg <= not_y;
            end if;

            if cs_saCrr5_i1 = '1' then
               a2_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr5_i2 = '1' then
               b2_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr5_i3 = '1' then
               c2_reg <= not_y;
            end if;

            if cs_saCrr8_i1 = '1' then
               a3_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr8_i2 = '1' then
               b3_reg <= not_y(9 downto 0);
            end if;
            if cs_saCrr8_i3 = '1' then
               c3_reg <= not_y;
            end if;

            -- Test
            if cs_saTest0 = '1' then
               x_cnt <= (others => '0');
               y_cnt <= (others => '0');
               acc_reg1 <= c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg;
               acc_reg2 <= c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg;
               acc_reg3 <= c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg;
               bck_reg1 <= c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg;
               bck_reg2 <= c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg;
               bck_reg3 <= c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg;
            end if;
            if (cs_saTest1 or cs_saTest2) = '1' then
               acc_reg1 <= sa4_y;
               acc_reg2 <= sa5_y;
               acc_reg3 <= sa6_y;
            end if;
            if cs_saTest2 = '1' then
               bck_reg1 <= sa4_y;
               bck_reg2 <= sa5_y;
               bck_reg3 <= sa6_y;
            end if;
            -- Limit counter
            if cs_saTest1 = '1' then
               x_cnt <= x_cnt + 1;
            end if;
            if (x_mx and cs_saTest1) = '1' then
               y_cnt <= y_cnt + 1;
            end if;
         end if;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Muxes
   process(cstate, vy0, vy1, vy2, vx0, vx1, vx2)
   begin
      case cstate is
         when saSetCoef0 => sa1_a <= '0' & vy0;
         when saSetCoef1 => sa1_a <= '0' & vx1;
         when saSetCoef3 => sa1_a <= '0' & vy1;
         when saSetCoef4 => sa1_a <= '0' & vx2;
         when saSetCoef6 => sa1_a <= '0' & vy2;
         when others     => sa1_a <= '0' & vx0;
      end case;
   end process;
   
   process(cstate, vy0, vy1, vy2, vx0, vx1, vx2)
   begin
      case cstate is
         when saSetCoef0 => sa1_b <= '0' & vy1;
         when saSetCoef1 => sa1_b <= '0' & vx0;
         when saSetCoef3 => sa1_b <= '0' & vy2;
         when saSetCoef4 => sa1_b <= '0' & vx1;
         when saSetCoef6 => sa1_b <= '0' & vy0;
         when others     => sa1_b <= '0' & vx2;
      end case;
   end process;

   process(cstate, vx0, vx1, vx2)
   begin
      case cstate is
         when saSetCoef1 => mult1_a <= '0' & vx0;
         when saSetCoef4 => mult1_a <= '0' & vx1;
         when saSetCoef7 => mult1_a <= '0' & vx2;
         when saCrr0     => mult1_a <= '0' & vx2;
         when saCrr3     => mult1_a <= '0' & vx0;
         when others     => mult1_a <= '0' & vx1;
      end case;
   end process;
         
   process(cstate, vy0, vy1, vy2, a1_reg, a2_reg, a3_reg)
   begin
      case cstate is
         when saSetCoef1 => mult1_b <= '0' & vy1;
         when saSetCoef4 => mult1_b <= '0' & vy2;
         when saSetCoef7 => mult1_b <= '0' & vy0;
         when saCrr0     => mult1_b <= a1_reg;
         when saCrr3     => mult1_b <= a2_reg;
         when others     => mult1_b <= a3_reg;
      end case;
   end process;

   process(cstate, vx0, vx1, vx2, b1_reg, b2_reg, b3_reg)
   begin
      case cstate is
         when saSetCoef1 => mult2_a <= '0' & vx1;
         when saSetCoef4 => mult2_a <= '0' & vx2;
         when saSetCoef7 => mult2_a <= '0' & vx0;
         when saCrr1     => mult2_a <= b1_reg;
         when saCrr4     => mult2_a <= b2_reg;
         when others     => mult2_a <= b3_reg;
      end case;
   end process;

   process(cstate, vy0, vy1, vy2)
   begin
      case cstate is
         when saSetCoef1 => mult2_b <= '0' & vy0;
         when saSetCoef4 => mult2_b <= '0' & vy1;
         when saSetCoef7 => mult2_b <= '0' & vy2;
         when saCrr1     => mult2_b <= '0' & vy2;
         when saCrr4     => mult2_b <= '0' & vy0;
         when others     => mult2_b <= '0' & vy1;
      end case;
   end process;

   
   process(cstate, mult1_y, mult2_y, a1_reg, b1_reg)
   begin
      case cstate is 
         when saCrr1  => sa4_a <= mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(18 downto 0);
         when saCrr2  => sa4_a <= mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(18 downto 0);
         when saCrr4  => sa4_a <= mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(18 downto 0);
         when saCrr5  => sa4_a <= mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(18 downto 0);
         when saCrr7  => sa4_a <= mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(mult1_y'high) & mult1_y(18 downto 0);
         when saCrr8  => sa4_a <= mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(mult2_y'high) & mult2_y(18 downto 0);
         when saTest1 => sa4_a <= a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg;
         when others  => sa4_a <= b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg;
      end case;         
   end process;
   
   process(cstate, c1_reg, c2_reg, c3_reg, acc_reg1, bck_reg1)
   begin
      case cstate is
         when saCrr1  => sa4_b <= c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg(c1_reg'high) & c1_reg;
         when saCrr2  => sa4_b <= acc_reg1;
         when saCrr4  => sa4_b <= c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg(c2_reg'high) & c2_reg;
         when saCrr5  => sa4_b <= acc_reg1;
         when saCrr7  => sa4_b <= c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg(c3_reg'high) & c3_reg;
         when saCrr8  => sa4_b <= acc_reg1;
         when saTest1 => sa4_b <= acc_reg1;
         when others  => sa4_b <= bck_reg1;
      end case;
   end process;
   
   process(cstate, a2_reg, b2_reg)
   begin
      case cstate is
         when saTest1 => sa5_a <= a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg;
         when others  => sa5_a <= b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg;
      end case;
   end process;
   
   process(cstate, acc_reg2, bck_reg2)
   begin
      case cstate is
         when saTest1 => sa5_b <= acc_reg2;
         when others  => sa5_b <= bck_reg2;
      end case;
   end process;
   
   process(cstate, a3_reg, b3_reg)
   begin
      case cstate is
         when saTest1 => sa6_a <= a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg;
         when others  => sa6_a <= b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg;
      end case;
   end process;

   process(cstate, acc_reg3, bck_reg3)
   begin
      case cstate is
         when saTest1 => sa6_b <= acc_reg3;
         when others  => sa6_b <= bck_reg3;
      end case;
   end process;

   process(cstate, a1_reg, a2_reg, a3_reg, b1_reg, b2_reg, b3_reg, c1_reg, c2_reg, c3_reg)
   begin
      case cstate is
         when saCrr2_i1 => not_a <= a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg(a1_reg'high) & a1_reg;
         when saCrr2_i2 => not_a <= b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg(b1_reg'high) & b1_reg;
         when saCrr2_i3 => not_a <= c1_reg;
         when saCrr5_i1 => not_a <= a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg(a2_reg'high) & a2_reg;
         when saCrr5_i2 => not_a <= b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg(b2_reg'high) & b2_reg;
         when saCrr5_i3 => not_a <= c2_reg;
         when saCrr8_i1 => not_a <= a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg(a3_reg'high) & a3_reg;
         when saCrr8_i2 => not_a <= b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg(b3_reg'high) & b3_reg;
         when others    => not_a <= c3_reg;
      end case;
   end process;
   
   -----------------------------------------------------------------------------
   -- FSM
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

   FSM_NSTATE: process(cstate, VERTEX_SET, x_mx, y_mx, sa4_y)
   begin
      nstate   <= saIdle;
      case cstate is
         when saIdle =>
            if VERTEX_SET = '1' then
               nstate <= saSetCoef0;
            end if;
         when saSetCoef0 => nstate <= saSetCoef1;
         when saSetCoef1 => nstate <= saSetCoef2;
         when saSetCoef2 => nstate <= saSetCoef3;
         when saSetCoef3 => nstate <= saSetCoef4;
         when saSetCoef4 => nstate <= saSetCoef5;
         when saSetCoef5 => nstate <= saSetCoef6;
         when saSetCoef6 => nstate <= saSetCoef7;
         when saSetCoef7 => nstate <= saSetCoef8;       
         when saSetCoef8 => nstate <= saCrr0;

         when saCrr0 =>     nstate <= saCrr1;
         when saCrr1 =>     nstate <= saCrr2;
         when saCrr2 =>
            if sa4_y(sa4_y'high) = '1' then
               nstate <= saCrr2_i1;
            else
               nstate <= saCrr3;
            end if;
         when saCrr2_i1 =>  nstate <= saCrr2_i2;
         when saCrr2_i2 =>  nstate <= saCrr2_i3;
         when saCrr2_i3 =>  nstate <= saCrr3;

         when saCrr3 =>     nstate <= saCrr4;
         when saCrr4 =>     nstate <= saCrr5;
         when saCrr5 =>
            if sa4_y(sa4_y'high) = '1' then
               nstate <= saCrr5_i1;
            else
               nstate <= saCrr6;
            end if;
         when saCrr5_i1 =>  nstate <= saCrr5_i2;
         when saCrr5_i2 =>  nstate <= saCrr5_i3;
         when saCrr5_i3 =>  nstate <= saCrr6;

         when saCrr6 =>     nstate <= saCrr7;
         when saCrr7 =>     nstate <= saCrr8;
         when saCrr8 =>
            if sa4_y(sa4_y'high) = '1' then
               nstate <= saCrr8_i1;
            else
               nstate <= saTest0;
            end if;
         when saCrr8_i1 =>  nstate <= saCrr8_i2;
         when saCrr8_i2 =>  nstate <= saCrr8_i3;
         when saCrr8_i3 =>  nstate <= saTest0;

         -- Begin
         when saTest0 =>    nstate <= saTest1;

         when saTest1 =>
            if (x_mx and y_mx) = '1' then
               nstate <= saIdle;
            elsif x_mx = '1' then
               nstate <= saTest2;
            else
               nstate <= saTest1;
            end if;

         when saTest2 =>    nstate <= saTest1;

      end case;
   end process;

   FSM_SIGS: process(cstate)
   begin
      ibusy    <= '1';
      cs_saSetCoef0 <= '0';
      cs_saSetCoef1 <= '0';
      cs_saSetCoef2 <= '0';
      cs_saSetCoef3 <= '0';
      cs_saSetCoef4 <= '0';
      cs_saSetCoef5 <= '0';
      cs_saSetCoef6 <= '0';
      cs_saSetCoef7 <= '0';
      cs_saSetCoef8 <= '0';
      cs_saCrr1 <= '0';
      cs_saCrr4 <= '0';
      cs_saCrr7 <= '0';
      cs_saCrr2_i1 <= '0';
      cs_saCrr2_i2 <= '0';
      cs_saCrr2_i3 <= '0';
      cs_saCrr5_i1 <= '0';
      cs_saCrr5_i2 <= '0';
      cs_saCrr5_i3 <= '0';
      cs_saCrr8_i1 <= '0';
      cs_saCrr8_i2 <= '0';
      cs_saCrr8_i3 <= '0';
      cs_saTest0 <= '0';
      cs_saTest1 <= '0';
      cs_saTest2 <= '0';
         
      case cstate is
         when saIdle     => ibusy <= '0';
         when saSetCoef0 => cs_saSetCoef0 <= '1';
         when saSetCoef1 => cs_saSetCoef1 <= '1';
         when saSetCoef2 => cs_saSetCoef2 <= '1';
         when saSetCoef3 => cs_saSetCoef3 <= '1';
         when saSetCoef4 => cs_saSetCoef4 <= '1';
         when saSetCoef5 => cs_saSetCoef5 <= '1';
         when saSetCoef6 => cs_saSetCoef6 <= '1';
         when saSetCoef7 => cs_saSetCoef7 <= '1';
         when saSetCoef8 => cs_saSetCoef8 <= '1';
         when saCrr1     => cs_saCrr1 <= '1';
         when saCrr2_i1  => cs_saCrr2_i1 <= '1';
         when saCrr2_i2  => cs_saCrr2_i2 <= '1';
         when saCrr2_i3  => cs_saCrr2_i3 <= '1';
         when saCrr5_i1  => cs_saCrr5_i1 <= '1';
         when saCrr5_i2  => cs_saCrr5_i2 <= '1';
         when saCrr5_i3  => cs_saCrr5_i3 <= '1';
         when saCrr8_i1  => cs_saCrr8_i1 <= '1';
         when saCrr8_i2  => cs_saCrr8_i2 <= '1';
         when saCrr8_i3  => cs_saCrr8_i3 <= '1';
         when saCrr4     => cs_saCrr4 <= '1';
         when saCrr7     => cs_saCrr7 <= '1';
         when saTest0    => cs_saTest0 <= '1';
         when saTest1    => cs_saTest1 <= '1';
         when saTest2    => cs_saTest2 <= '1';
         when others     => null;
      end case;
   end process;

end arch_rasterizer_triangle;
