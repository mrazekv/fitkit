-- keyboard_ctrl.vhd : Keyboard controller
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
-- Description:
--
--                 KEYBOARD
--
--                 +---+---+---+---+
--  KB_KIN(0) ---->| 1 | 2 | 3 | A |
--                 +---+---+---+---+
--  KB_KIN(1) ---->| 4 | 5 | 6 | B |
--                 +---+---+---+---+
--  KB_KIN(2) ---->| 7 | 8 | 9 | C |
--                 +---+---+---+---+
--  KB_KIN(3) ---->| * | 0 | # | D |
--                 +---+---+---+---+
--                   |   |   |   |
--  KB_KOUT(0)  <----+   |   |   |
--  KB_KOUT(1)  <--------+   |   |
--  KB_KOUT(2)  <------------+   |
--  KB_KOUT(3)  <----------------+
--
-- meaning of DATA_OUT
--      DATA_OUT(0) <=> '1'
--      DATA_OUT(1) <=> '4'
--      DATA_OUT(2) <=> '7'
--      DATA_OUT(3) <=> '*'
--      DATA_OUT(4) <=> '2'
--      DATA_OUT(5) <=> '5'
--      DATA_OUT(6) <=> '8'
--      DATA_OUT(7) <=> '0'
--      DATA_OUT(8) <=> '3'
--      DATA_OUT(9) <=> '6'
--      DATA_OUT(10) <=> '9'
--      DATA_OUT(11) <=> '#'
--      DATA_OUT(12) <=> 'A'
--      DATA_OUT(13) <=> 'B'
--      DATA_OUT(14) <=> 'C'
--      DATA_OUT(15) <=> 'D'
--


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- SYNTH-ISE-8.2: slices=27, slicesFF=36, 4luts=53
entity keyboard_controller is
   generic(
      READ_INTERVAL : integer := (2**14)-1
   );
   port(
      -- Hodiny
      CLK        : in std_logic;
      -- Reset
      RST        : in std_logic;

      -- Stisknute klavesy
      DATA_OUT   : out std_logic_vector(15 downto 0);
      DATA_VLD   : out std_logic;
      
      -- Signaly klavesnice 
      KB_KIN     : out std_logic_vector(3 downto 0);
      KB_KOUT    : in  std_logic_vector(3 downto 0)
   );
end keyboard_controller;

architecture arch_keyboard of keyboard_controller is

   type   FSMstate is (SInit, SScan0, SScan1, SScan2, SScan3, SWE);

   signal pstate : FSMstate; -- actual state
   signal nstate : FSMstate; -- next state 

   signal data_reg : std_logic_vector(15 downto 0);
   signal data_mx  : std_logic_vector(3 downto 0);
   signal data_sel : std_logic_vector(1 downto 0);
   signal data_cmp : std_logic;

   signal cntr_reg : std_logic_vector(14 downto 0);
   signal cntr_zero: std_logic;

   signal kb_we  : std_logic_vector(3 downto 0);
   signal kbout  : std_logic_vector(3 downto 0);

   signal eq_reg : std_logic;
   signal eq_rst : std_logic;
   signal eq_en  : std_logic;

begin

   DATA_OUT <= data_reg;

   -- Interval counter
   intervalcounter: process (RST, CLK)
   begin
      if (RST = '1') then
         cntr_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (cntr_zero='1') then
            cntr_reg <= (others => '0');
         else
            cntr_reg <= cntr_reg + 1;
         end if;
      end if;
   end process;
   cntr_zero <= '1' when (cntr_reg=conv_std_logic_vector(READ_INTERVAL, cntr_reg'length)) 
                else '0';

   -- Activation of rows
   -- in order to avoid shorts due to missing diodes, we use 'Z'
   KB_KIN(0) <= '0' when kb_we(0) = '1' else 'Z'; 
   KB_KIN(1) <= '0' when kb_we(1) = '1' else 'Z'; 
   KB_KIN(2) <= '0' when kb_we(2) = '1' else 'Z'; 
   KB_KIN(3) <= '0' when kb_we(3) = '1' else 'Z'; 

   kbout <= not KB_KOUT;  

   -- Data registers
   dregs: process (RST, CLK)
   begin
      if (RST = '1') then
         data_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (kb_we(0) = '1') then
            data_reg(3 downto 0) <= kbout;
         end if;
         if (kb_we(1) = '1') then
            data_reg(7 downto 4) <= kbout;
         end if;
         if (kb_we(2) = '1') then
            data_reg(11 downto 8) <= kbout;
         end if;
         if (kb_we(3) = '1') then
            data_reg(15 downto 12) <= kbout;
         end if;
      end if;
   end process;

   data_mx <= data_reg(3 downto 0)  when data_sel="00" else
              data_reg(7 downto 4)  when data_sel="01" else
              data_reg(11 downto 8) when data_sel="10" else
              data_reg(15 downto 12);

   data_cmp <= '1' when (data_mx = kbout) else '0';

   -- Register that hold changes
   process (RST, CLK)
   begin
      if (RST = '1') then
         eq_reg <= '1';
      elsif (CLK'event) and (CLK = '1') then
         if (eq_rst='1') then
            eq_reg <= '1';
         elsif (eq_en='1') then
            eq_reg <= eq_reg and data_cmp;
         end if;
      end if;
   end process;
   
   --FSM actual state
   process (RST, CLK)
   begin
      if (RST = '1') then
         pstate <= SInit;
      elsif (CLK'event) and (CLK = '1') then
         pstate <= nstate;
      end if;
   end process;   

   --FSM next state logic
   process (pstate, cntr_zero, eq_reg)
   begin
      nstate <= SInit;
      kb_we <= "0000";
      data_sel <= "00";
      DATA_VLD <= '0';
      eq_rst <= '0';
      eq_en <= '0';

      case pstate is
         when SInit =>
            if (cntr_zero='1') then
               nstate <= SScan0;
               eq_rst <= '1';
            end if;

         --Scan
         when SScan0 =>
            nstate <= SScan1;
            data_sel <= "00";
            kb_we(0) <= '1';
            eq_en <= '1';

         when SScan1 =>
            nstate <= SScan2;
            data_sel <= "01";
            kb_we(1) <= '1';
            eq_en <= '1';

         when SScan2 =>
            nstate <= SScan3;
            data_sel <= "10";
            kb_we(2) <= '1';
            eq_en <= '1';

         when SScan3 =>
            nstate <= SWE;
            data_sel <= "11";
            kb_we(3) <= '1';
            eq_en <= '1';

         --Write enable
         when SWE =>
            DATA_VLD <= not eq_reg;

         when others =>
            null;
      end case;
   end process;

end arch_keyboard;
