-- onewire.vhd: 1-wire
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.math_pack.all;

entity onewire is
   generic (
      CLK_DIV : integer := 250 -- 50MHz CLK (5*50), 40MHz CLK (5*40), 20 MHZ CLK (5*20)
   );
   port ( 
      CLK      : in std_logic; 
      RESET    : in std_logic; 


      CMD      : in std_logic_vector(1 downto 0);
      CMD_WE   : in std_logic;

      DATA_IN  : in std_logic_vector(7 downto 0);
      DATA_OUT : out std_logic_vector(7 downto 0);
      PRESENCE : out std_logic;
   
      BUSY     : out std_logic;
      DQ       : inout std_logic
   ); 
end onewire;


architecture beh of onewire is
   signal dqclk : std_logic;
   signal clk_cntr : std_logic_vector(log2(CLK_DIV)-1 downto 0);

   signal cmd_reg : std_logic_vector(1 downto 0);
   signal din_shreg : std_logic_vector(7 downto 0);
   signal din_shen : std_logic;
   signal dout_shreg : std_logic_vector(7 downto 0);
   signal dout_shen : std_logic;
   signal presence_reg : std_logic;
   signal presence_we : std_logic;

   signal bit_cntr : std_logic_vector(2 downto 0);
   signal bitcntr_cmp : std_logic; 
   signal bitcntr_zero : std_logic;

   type FSMstate is (SInit, SCLKWait, SReset1, SReset2, SReset3, SWrite1, SWrite2, SWrite3, SRead1, SRead2a,SRead2, SRead3);
   signal pstate : FSMstate; -- actual state
   signal nstate : FSMstate; -- next state 

   signal cntr : std_logic_vector(6 downto 0);
   signal cntr_rst : std_logic;
   signal cntr_en : std_logic;
   signal cmp_100 : std_logic;
   signal cmp_14 : std_logic;

   signal dq_reg1 : std_logic;
   signal dq_reg2 : std_logic;
begin
   -- CLOCK divider (200 kHz signal)
   -- ----------------------------------------------------------
   dqclk <= '1' when conv_integer(clk_cntr) = CLK_DIV else '0';

   freqdv: process (RESET, CLK)
   begin
      if (RESET = '1') then
         clk_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (dqclk = '1') then
            clk_cntr <= (others => '0');
         else
            clk_cntr <= clk_cntr + 1;
         end if;
      end if;
   end process;

   -- Cycle counter
   -- ----------------------------------------------------------
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (cntr_rst = '1') then
            cntr <= (others => '0');
         elsif (cntr_en = '1') and (dqclk = '1') then
            cntr <= cntr + 1;
         end if;
      end if;
   end process;
   cmp_100 <= '1' when conv_integer(cntr) = 100 else '0';
   cmp_14  <= '1' when conv_integer(cntr) = 14  else '0';

   -- Data registers
   -- ----------------------------------------------------------
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (DQ='0') then 
            dq_reg1 <= '0';
         else
            dq_reg1 <= '1';
         end if;
         dq_reg2 <= dq_reg1;
      end if;
   end process;

   -- CMD register
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         cmd_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (CMD_WE='1') then
            cmd_reg <= cmd;
         end if;
      end if;
   end process;

   -- DATA (out) shift register
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         dout_shreg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (dout_shen = '1') then
            dout_shreg <= dq_reg2 & dout_shreg(7 downto 1);
         end if;
      end if;
   end process;

   -- DATA (in) shift register
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         din_shreg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (CMD_WE='1') then
            din_shreg <= DATA_IN;
         elsif (din_shen = '1') then
            din_shreg <= '0' & din_shreg(7 downto 1);
         end if;
      end if;
   end process;

   -- bit counter
   bitcntr_cmp  <= '1' when conv_integer(bit_cntr) = 7 else '0';
   bitcntr_zero <= '1' when conv_integer(bit_cntr) = 0 else '0';

   process (RESET, CLK)
   begin
      if (RESET = '1') then
         bit_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (din_shen = '1') or (dout_shen='1') then
            if (bitcntr_cmp = '1') then
               bit_cntr <= (others => '0');
            else
               bit_cntr <= bit_cntr + 1;
            end if;
         end if;
      end if;
   end process;

   process (RESET, CLK)
   begin
      if (RESET = '1') then
         presence_reg <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (presence_we = '1') then
            presence_reg <= not dq_reg2;
         end if;
      end if;
   end process;

   PRESENCE <= presence_reg;

   DATA_OUT <= dout_shreg;
 
   -- FSM
   -- ----------------------------------------------------------
   --Present State registr
   pstatereg: process(RESET, CLK)
   begin
      if (RESET='1') then
         pstate <= SInit;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;
   
   --Next State logic
   nstate_logic: process(pstate, CMD_WE, cmd_reg, dqclk, cmp_100, cmp_14, bitcntr_zero, din_shreg)
   begin
      cntr_rst <= '0';
      cntr_en <= '0';
      presence_we <= '0';
      din_shen <= '0';
      dout_shen <= '0';
      BUSY <= '1';
      DQ <= 'Z';

      nstate <= SInit;
      case pstate is
         when SInit =>
            BUSY <= '0';

            nstate <= SInit;
            if (CMD_WE='1') then
               nstate <= SCLKWait;
            end if; 

         when SCLKWait => 
            cntr_rst <= '1';

            nstate <= SCLKWait;
            if (dqclk = '1') then
               if (cmd_reg = "00") then
                  nstate <= SReset1; 
               elsif (cmd_reg = "01") then
                  nstate <= SWrite1; 
               elsif (cmd_reg = "10") then
                  nstate <= SRead1; 
               else 
                  nstate <= SInit;
               end if;
            end if;
         ----------------- Reset ------------------
         when SReset1 =>
            DQ <= '0';
            cntr_en <= '1';

            nstate <= SReset1;
            if (cmp_100 = '1') then
               cntr_rst <= '1';
               nstate <= SReset2;
            end if;

         when SReset2 =>
            cntr_en <= '1';

            nstate <= SReset2;
            if (cmp_14 = '1') then
               presence_we <= '1';
               nstate <= SReset3;
            end if;

         when SReset3 =>
            cntr_en <= '1';

            nstate <= SReset3;
            if (cmp_100 = '1') then
               nstate <= SInit;
            end if;

         ----------------- Write ------------------
         when SWrite1 =>
            cntr_en <= '1';
            DQ <= '0';

            nstate <= SWrite1;
            if (dqclk = '1') then
               nstate <= SWrite2;
            end if;

         when SWrite2 =>
            cntr_en <= '1';
            DQ <= din_shreg(0); --Data, ktera se maji zapsat

            nstate <= SWrite2;
            if (cmp_14 = '1') then
               nstate <= SWrite3;
               din_shen <= '1';
            end if;

         when SWrite3 =>
            cntr_rst <= '1';

            nstate <= SWrite3;
            if (dqclk = '1') then
               if (bitcntr_zero = '1') then
                  nstate <= SInit;
               else
                  nstate <= SWrite1;
               end if;
            end if;

         ----------------- Read ------------------
         when SRead1 =>
            cntr_en <= '1';
            DQ <= '0';

            nstate <= SRead1;
            if (dqclk = '1') then
               nstate <= SRead2;
            end if;

         when SRead2 =>
            cntr_en <= '1';

            nstate <= SRead2;
            if (dqclk = '1') then
               nstate <= SRead2a;
            end if;

         when SRead2a =>
            --vzorkovani DQ
            dout_shen <= '1';
            cntr_en <= '1';
            nstate <= SRead3;

         when SRead3 =>
            cntr_en <= '1';

            nstate <= SRead3;
            if (cmp_14 = '1') then
               cntr_rst <= '1';
               if (bitcntr_zero = '1') then
                  nstate <= SInit;
               else
                  nstate <= SRead1;
               end if;
            end if;

         when others => null;
      end case;
   end process;
   
end beh;

