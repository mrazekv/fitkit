-- lcd_fsm_cycle.vhd : LCD controller
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <xvasic11 AT stud.fit.vutbr.cz>
--            Tomas Martinek <martinto AT fit.vutbr.cz>
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
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

-- ------------------------------------------------------------------
--                      Entity Declaration
-- ------------------------------------------------------------------
entity lcd_fsm_cycle is
   port (
      RESET    : in std_logic;
      CLK      : in std_logic;

      -- input signals
      REQ      : in std_logic; 
      REG_RW   : in std_logic; 

      -- output signals
      FSM_LE   : out   std_logic;
      FSM_RW   : out   std_logic;
      FSM_DV   : out   std_logic;
      FSM_TS   : out   std_logic;
      FSM_BUSY : out   std_logic
);
end lcd_fsm_cycle;

-- ------------------------------------------------------------------
--                    Architecture Declaration
-- ------------------------------------------------------------------
architecture behavioral of lcd_fsm_cycle is
   type t_state is (IDLE,  T0, T1, T2, T3, T4, T5, T6, T7, T8, T9);
   signal present_state, next_state : t_state;

begin

-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= IDLE;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, REQ)
begin
   case (present_state) is
   when IDLE => 
      next_state <= IDLE;
      if (REQ='1') then
         next_state <= T0;
      end if;
   when T0 => next_state <= T1;
   when T1 => next_state <= T2;
   when T2 => next_state <= T3;
   when T3 => next_state <= T4;
   when T4 => next_state <= T5;
   when T5 => next_state <= T6;
   when T6 => next_state <= T7;
   when T7 => next_state <= T8;
   when T8 => next_state <= T9;
   when T9 => next_state <= IDLE;
   when others => 
      next_state <= IDLE;
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, REG_RW)
begin
   FSM_RW   <= '1';
   FSM_LE   <= '0';
   FSM_DV   <= '0';
   FSM_BUSY <= '1';
   FSM_TS   <= '1';

   case (present_state) is
   when IDLE => 
      FSM_BUSY <= '0';
   when T0 => 
      FSM_RW   <= REG_RW;
   when T1 => 
      FSM_RW   <= REG_RW;
   when T2 => 
      FSM_LE   <= '1'; 
      FSM_RW   <= REG_RW;
   when T3 => 
      FSM_LE   <= '1'; 
      FSM_TS   <= REG_RW;
      FSM_RW   <= REG_RW;
   when T4 => 
      FSM_LE   <= '1'; 
      FSM_TS   <= REG_RW;
      FSM_RW   <= REG_RW;
   when T5 => 
      FSM_LE   <= '1'; 
      FSM_TS   <= REG_RW;
      FSM_DV   <= REG_RW;
      FSM_RW   <= REG_RW;
   when T6 => 
      FSM_TS   <= REG_RW;
      FSM_RW   <= REG_RW;
   when T7 => 
      FSM_TS   <= REG_RW;
      FSM_RW   <= REG_RW;
   when T8 => 
   when T9 => 
   when others =>
   end case;
end process output_logic;

end behavioral;

