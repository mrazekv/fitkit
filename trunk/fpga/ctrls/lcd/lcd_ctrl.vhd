-- lcd_ctrl.vhd : LCD base controller
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

-- SYNTH-ISE: slices=7, slicesFF=13, 4luts=6
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

entity lcd_controller is
   port (
      RST      : in std_logic;
      CLK      : in std_logic;

      -- interni rozhrani
      RS       : in std_logic;
      DATA_IN  : in std_logic_vector (7 downto 0);
      WRITE_EN : in std_logic; -- zapis, okamzik planosti RS, DATA_IN (pokud radic prave zpracovava, je zapis zahozen)
      --DATA_OUT  : in std_logic_vector (7 downto 0);
      --READ_EN  : in std_logic;

      --- rozhrani LCD displeje
      DISPLAY_RS   : out   std_logic;
      DISPLAY_DATA : inout std_logic_vector(7 downto 0);
      DISPLAY_RW   : out   std_logic;
      DISPLAY_EN   : out   std_logic
);

end lcd_controller;

architecture base of lcd_controller is

   type FSMState is (idle,  w0, w1, w2, w3, w4, w5, w6, w7);

   signal pstate : FSMState; -- FSM present state
   signal nstate : FSMState; -- FSM next state
   signal data_reg : std_logic_vector(7 downto 0);
   signal datareg_en : std_logic;
   signal rs_reg   : std_logic;
   signal data_sel : std_logic;

begin
   DISPLAY_RS <= rs_reg;
   DISPLAY_DATA <= data_reg when (data_sel='1') else (others => 'Z');

   -- data register
   process(CLK, RST)
   begin
      if (RST = '1') then
         data_reg <= (others => '0');
         rs_reg <= '0';
      elsif (CLK='1') and (CLK'event) then
         if (WRITE_EN='1') and (datareg_en='1') then
            data_reg <= DATA_IN;
            rs_reg <= RS;
         end if;
      end if;
   end process;

   -- FSM present state
   process(CLK, RST)
   begin
      if (RST = '1') then
         pstate <= idle;
      elsif (CLK='1') and (CLK'event) then
         pstate <= nstate;
      end if;
   end process;

   -- FSM next state logic, output logic
   process(pstate, WRITE_EN)
   begin
      nstate <= idle;
      DISPLAY_RW <= '1';
      DISPLAY_EN <= '0';
      data_sel <= '0';
      datareg_en <= '0';

      case pstate is
         when idle =>
            datareg_en <= '1';
            if (WRITE_EN = '1') then
               nstate <= w0;
            end if;

         when w0 =>
            nstate <= w1;
            DISPLAY_RW <= '0';

         when w1 =>
            nstate <= w2;
            DISPLAY_RW <= '0';

         when w2 =>
            nstate <= w3;
            DISPLAY_RW <= '0';
            DISPLAY_EN <= '1';

         when w3 =>
            nstate <= w4;
            DISPLAY_RW <= '0';
            DISPLAY_EN <= '1';
            data_sel <= '1';

         when w4 =>
            nstate <= w5;
            DISPLAY_RW <= '0';
            DISPLAY_EN <= '1';
            data_sel <= '1';

         when w5 =>
            nstate <= w6;
            DISPLAY_RW <= '0';
            DISPLAY_EN <= '1';
            data_sel <= '1';

         when w6 =>
            nstate <= w7;
            DISPLAY_RW <= '0';
            data_sel <= '1';

         when w7 =>
            data_sel <= '1';
            DISPLAY_RW <= '0';

      end case;
   end process;

end base;

