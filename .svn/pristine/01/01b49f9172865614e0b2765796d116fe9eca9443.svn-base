-- ps2half_ctrl.vhd: Simple PS/2 Controller (readonly version)
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
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

architecture half of PS2_controller is

   type    FSMstate is (SInit, SRcvData, SRcvParity, SRcvStop);
   signal  pstate    : FSMstate; -- actual state
   signal  nstate    : FSMstate; -- next state 
   
   signal  ps2clk_reg  : std_logic;
   signal  ps2clk_dreg : std_logic;
   signal  ps2data_reg : std_logic;
   signal  di_en       : std_logic; --read from PS/2 data
   
   signal bitcntr     : std_logic_vector(7 downto 0);
   signal bitcntr_en  : std_logic;
   signal bitcntr_rst : std_logic;
   signal bits_cmp8   : std_logic;
   
   signal datain_reg  : std_logic_vector(7 downto 0);
   signal datain_en   : std_logic;
   signal datain_rst  : std_logic;
   signal datain_parity : std_logic;

begin
   PS2_CLK   <= 'Z';
   PS2_DATA  <= 'Z';

   DATA_OUT  <= datain_reg;

   -- PS/2 CLK edge detector
   process (RST, CLK)
   begin
      if (RST = '1') then
         ps2clk_reg  <= '1';
         ps2clk_dreg <= '1';
         ps2data_reg <= '1';
      elsif (CLK'event) and (CLK = '1') then 
         ps2clk_reg  <= PS2_CLK;
         ps2clk_dreg <= ps2clk_reg;
         ps2data_reg <= PS2_DATA;
      end if;  
   end process;

   di_en <= ps2clk_dreg and (not ps2clk_reg);

   -- Bits counter
   bitscntr: process(RST, CLK)
   begin
      if (RST = '1') then 
         bitcntr <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if (bitcntr_rst = '1') then
            bitcntr <= (others => '0');
         elsif (bitcntr_en='1') then
            bitcntr <= bitcntr + 1;         
         end if;
      end if;
   end process; 

   -- Comparators
   bits_cmp8   <= '1' when (bitcntr = 16#08#) else '0'; 

   --DataIN shift register
   process (RST, CLK)
   begin
      if (RST = '1') then
         --datain_reg <= (others=>'0');
      elsif (CLK'event) and (CLK = '1') then
         if (datain_en='1') then
            datain_reg <= ps2data_reg & datain_reg(7 downto 1);
         end if;
      end if;
   end process;

   --DataIN odd parity
   process (RST, CLK)
   begin
      if (RST = '1') then
         datain_parity <= '1';
      elsif (CLK'event) and (CLK = '1') then
         if (datain_rst = '1') then
            datain_parity <= '1';
         elsif (datain_en='1') then
            datain_parity <= datain_parity xor ps2data_reg;
         end if;     
      end if;
   end process;

   --FSM present state
   process (RST, CLK)
   begin
      if (RST = '1') then
         pstate <= sinit;
      elsif (CLK'event) and (CLK = '1') then 
         pstate <= nstate;
      end if;  
   end process;

   --FSM next state logic
   process (pstate, di_en, ps2data_reg, datain_parity, bits_cmp8)
   begin
      nstate <= SInit;
      bitcntr_rst <= '0';
      bitcntr_en <= '0';
      datain_en <= '0';
      datain_rst <= '0';
      DATA_ERR <= '0';
      DATA_VLD <= '0';

      case pstate is
         when SInit =>
            if (di_en='1') and (ps2data_reg='0') then
               nstate <= SRcvData;
               bitcntr_rst <= '1';
               datain_rst <= '1';
            end if;

         --Receive data bits
         when SRcvData =>
            if (bits_cmp8='0') then
               nstate <= SRcvData;
               bitcntr_en <= di_en;
               datain_en <= di_en;
            else
               nstate <= SRcvParity;
            end if;

         -- Receive parity
         when SRcvParity =>
            if (di_en='1') then
               nstate <= SRcvStop;
               assert (ps2data_reg=datain_parity) report "PS/2 Data Parity Error" severity note; 
               if (ps2data_reg=datain_parity) then 
                  DATA_VLD <= '1';
               else
                  DATA_ERR <= '1';
               end if;
            else
               nstate <= SRcvParity;
            end if;

         -- Receive stop bit
         when SRcvStop =>
            if (di_en='0') then
               nstate <= SRcvStop;
            end if;

         when others => 
            null;
      end case;
   end process;
end half;

