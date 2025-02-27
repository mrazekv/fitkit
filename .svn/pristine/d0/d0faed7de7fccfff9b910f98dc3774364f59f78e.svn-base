-- lcd_fsm_main.vhd : LCD controller
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Tomas Martinek <martinto AT fit.vutbr.cz>
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
-- Functional description:
-- =======================
-- * when READ operation is required, the FSM waits until read cycle finish
--   and then goes to the IDLE state
-- * when WRITE operation is required then:
--       * FSM waits until write cycle finish
--       * FSM reads LCD Busy flag repeatly until flag goes low
--       * FSM waits several clock cycles before next read of write operation
--
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;
-- ------------------------------------------------------------------
--                      Entity Declaration
-- ------------------------------------------------------------------
entity lcd_fsm_main is
   port (
      RESET       : in  std_logic;
      CLK         : in  std_logic;

      -- input signals
      READ        : in  std_logic; 
      WRITE       : in  std_logic; 
      LCD_BUSY    : in  std_logic;
      FSMC_BUSY   : in  std_logic;

      -- output signals
      FSM_REQ     : out std_logic;
      FSM_DV      : out std_logic;
      FSM_RDBF    : out std_logic;
      FSM_BUSY    : out std_logic
);
end lcd_fsm_main;
-- ------------------------------------------------------------------
--                    Architecture Declaration
-- ------------------------------------------------------------------
architecture behavioral of lcd_fsm_main is
   type t_state is (IDLE, READ_OP, WRITE_OP, READ_BUSY, READ_WAIT);
   signal present_state, next_state : t_state;
   signal cnt_rdwait    : std_logic_vector(3 downto 0);
   signal cnt_rdwait_ce : std_logic;
   signal cnt_rdwait_of : std_logic;

begin

-- cnt_rdwait counter
process(RESET, CLK)
begin
   if (RESET = '1') then
      cnt_rdwait <= (others => '0');
   elsif (CLK'event AND CLK = '1') then
      if (cnt_rdwait_ce = '1') then
         cnt_rdwait <= cnt_rdwait + 1;
      end if;
   end if;
end process;

cnt_rdwait_of <= '1' when (cnt_rdwait = "1111") else '0';

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
next_state_logic : process(present_state, READ, WRITE, FSMC_BUSY, LCD_BUSY,
                           cnt_rdwait_of)
begin
   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when IDLE =>
      next_state <= IDLE;
      if (READ='1') then
         next_state <= READ_OP;
      elsif (WRITE='1') then
         next_state <= WRITE_OP;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_OP =>
      next_state <= READ_OP;
      if (FSMC_BUSY='0') then
         next_state <= IDLE;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_OP =>
      next_state <= WRITE_OP;
      if (FSMC_BUSY='0') then
         next_state <= READ_BUSY;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_BUSY =>
      next_state <= READ_BUSY;
      if (FSMC_BUSY='0' and LCD_BUSY='0') then
         next_state <= READ_WAIT;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_WAIT =>
      next_state <= READ_WAIT;
      if (cnt_rdwait_of='1') then
         next_state <= IDLE;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      next_state <= IDLE;
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, READ, WRITE, FSMC_BUSY, LCD_BUSY)
begin
   FSM_REQ  <= '0';   
   FSM_DV   <= '0';   
   FSM_RDBF <= '0';   
   FSM_BUSY <= '1';   
   cnt_rdwait_ce <= '0';

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when IDLE => 
      FSM_BUSY <= '0';   
      if (READ='1' or WRITE='1') then
         FSM_REQ <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_OP => 
      if (FSMC_BUSY='0') then
         FSM_DV <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_OP => 
      if (FSMC_BUSY='0') then
         FSM_RDBF <= '1';
         FSM_REQ  <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_BUSY => 
      if (FSMC_BUSY='0' and LCD_BUSY='1') then
         FSM_RDBF <= '1';
         FSM_REQ  <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when READ_WAIT=> 
      cnt_rdwait_ce <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
   end case;
end process output_logic;

end behavioral;

