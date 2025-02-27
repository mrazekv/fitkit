-- lcd_ctrl_rw.vhd : LCD base controller
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
--

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

-- ------------------------------------------------------------------
--                      Entity Declaration
-- ------------------------------------------------------------------
entity lcd_ctrl_rw is
   port (
      RESET    : in std_logic;
      CLK      : in std_logic;

      -- interni rozhrani
      READ     : in std_logic; 
      WRITE    : in std_logic; 
      RS       : in std_logic; 
      DIN      : in  std_logic_vector(7 downto 0);
      DOUT     : out std_logic_vector(7 downto 0);
      DV       : out std_logic;
      BUSY     : out std_logic;

      --- rozhrani LCD displeje
      LRS      : out   std_logic;
      LRW      : out   std_logic;
      LE       : out   std_logic;
      LD       : inout std_logic_vector(7 downto 0)
);

end lcd_ctrl_rw;

-- ------------------------------------------------------------------
--                    Architecture Declaration
-- ------------------------------------------------------------------
architecture behavioral of lcd_ctrl_rw is

   signal reg_data_in      : std_logic_vector(7 downto 0);
   signal reg_data_in_we   : std_logic;
   signal reg_data_out     : std_logic_vector(7 downto 0);
   signal reg_data_out_we  : std_logic;
   signal reg_rs           : std_logic;
   signal reg_rs_we        : std_logic;
   signal reg_rw           : std_logic;
   signal reg_rw_we        : std_logic;

   signal fsm_cycle_dv     : std_logic;
   signal fsm_cycle_rw     : std_logic;
   signal fsm_cycle_ts     : std_logic;
   signal fsm_cycle_busy   : std_logic;

   signal fsm_main_rdbf    : std_logic;
   signal fsm_main_req     : std_logic;
   signal fsm_main_dv      : std_logic;
   signal fsm_main_busy    : std_logic;
   signal fsm_main_read    : std_logic;
   signal fsm_main_write   : std_logic;

begin
-- ------------------------------------------------------------------
--                           Registers
-- ------------------------------------------------------------------

-- reg_data_in register
reg_data_in_we <= WRITE and (not fsm_cycle_busy);
process(RESET, CLK)
begin
   if (RESET = '1') then
      reg_data_in <= (others => '0');
   elsif (CLK'event AND CLK = '1') then
      if (reg_data_in_we = '1') then
         reg_data_in <= DIN;
      end if;
   end if;
end process;

-- reg_data_out register
reg_data_out_we <= fsm_cycle_dv;
process(RESET, CLK)
begin
   if (RESET = '1') then
      reg_data_out <= (others => '0');
   elsif (CLK'event AND CLK = '1') then
      if (reg_data_out_we = '1') then
         reg_data_out <= LD;
      end if;
   end if;
end process;

-- reg_rs register
reg_rs_we <= (READ or WRITE or fsm_main_rdbf) and (not fsm_cycle_busy);
process(RESET, CLK)
begin
   if (RESET = '1') then
      reg_rs <= '0';
   elsif (CLK'event AND CLK = '1') then
      if (reg_rs_we = '1') then
         reg_rs <= RS and (not fsm_main_rdbf);
      end if;
   end if;
end process;

-- reg_rw register
reg_rw_we <= (READ or WRITE or fsm_main_rdbf) and (not fsm_cycle_busy);
process(RESET, CLK)
begin
   if (RESET = '1') then
      reg_rw <= '0';
   elsif (CLK'event AND CLK = '1') then
      if (reg_rw_we = '1') then
         reg_rw <= (READ and (not WRITE)) or fsm_main_rdbf;
      end if;
   end if;
end process;

-- Output mapping
LRS   <= reg_rs;
LRW   <= fsm_cycle_rw;
LD    <= (others => 'Z') when (fsm_cycle_ts = '1') else reg_data_in;

DOUT  <= reg_data_out;
BUSY  <= fsm_main_busy;
DV    <= fsm_main_dv;

-- ------------------------------------------------------------------
--            Finite State Machine - Read/Write Cycle
-- ------------------------------------------------------------------
fsm_main_read  <= READ  and (not fsm_main_busy);
fsm_main_write <= WRITE and (not fsm_main_busy);

FSM_MAIN : entity work.lcd_fsm_main
port map(
   RESET       => RESET,
   CLK         => CLK,

   -- input signals
   READ        => fsm_main_read,
   WRITE       => fsm_main_write,
   LCD_BUSY    => reg_data_out(7),
   FSMC_BUSY   => fsm_cycle_busy,

   -- output signals
   FSM_REQ     => fsm_main_req,
   FSM_DV      => fsm_main_dv,
   FSM_RDBF    => fsm_main_rdbf,
   FSM_BUSY    => fsm_main_busy
);

-- ------------------------------------------------------------------
--              Finite State Machine - Protocol Cycle
-- ------------------------------------------------------------------
FSM_CYCLE : entity work.lcd_fsm_cycle
port map(
   RESET    => RESET,
   CLK      => CLK,

   -- interni rozhrani
   REQ      => fsm_main_req,
   REG_RW   => reg_rw,

   --- rozhrani LCD displeje
   FSM_LE   => LE,
   FSM_DV   => fsm_cycle_dv,
   FSM_TS   => fsm_cycle_ts,
   FSM_RW   => fsm_cycle_rw,
   FSM_BUSY => fsm_cycle_busy
);

end behavioral;

