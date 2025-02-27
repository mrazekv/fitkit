-- ps2full_ctrl.vhd: PS/2 Controller
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

architecture full of PS2_controller is

   type    FSMstate is (SInit, SRcvData, SRcvParity, SRcvStop,
                        SStart0, SStart1, SStart2, SSndData, 
                        SSndParity, SSndStop, SRcvAck);
                        
   signal  pstate      : FSMstate; -- actual state
   signal  nstate      : FSMstate; -- next state 
   
   signal  ps2clk_reg  : std_logic;
   signal  ps2clk_dreg : std_logic;
   signal  ps2data_reg : std_logic;
   signal  di_en       : std_logic; --read from PS/2 data
   signal  do_en       : std_logic; --write to PS/2 data
   
   signal cntr_reg     : std_logic_vector(8 downto 0);
   signal cntr_en      : std_logic;
   signal cntr_rst     : std_logic;
   signal cntr_cmp8    : std_logic;
   signal cntr_cmp511  : std_logic;
   
   signal data_reg     : std_logic_vector(7 downto 0);
   signal data_en      : std_logic;
   
   signal parity_reg   : std_logic;
   signal parity_en    : std_logic;
   signal parity_rst   : std_logic;
   signal parity_in    : std_logic;
   signal parity_sel   : std_logic;
   
   signal clk_we       : std_logic;
   signal data_sel     : std_logic_vector(1 downto 0);
   signal busy         : std_logic;

begin

   DATA_OUT  <= data_reg;

   -- PS/2 interface
   PS2_CLK   <= '0' when clk_we='1' else 'Z' ;
   
   PS2_DATA  <= data_reg(0) when data_sel="00" else
                parity_reg  when data_sel="01" else
                '0'         when data_sel="10" else
                'Z';

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

   di_en <= ps2clk_dreg and (not ps2clk_reg); --Falling edge
   do_en <= (not ps2clk_dreg) and ps2clk_reg; --Rising edge

   -- Bits counter
   bitscntr: process(RST, CLK)
   begin
      if (RST = '1') then 
         cntr_reg <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if (cntr_rst = '1') then
            cntr_reg <= (others => '0');
         elsif (cntr_en='1') then
            cntr_reg <= cntr_reg + 1;         
         end if;
      end if;
   end process; 

   -- Comparators
   cntr_cmp8   <= '1' when (cntr_reg = 16#008#) else '0'; 
   cntr_cmp511 <= '1' when (cntr_reg = 16#1ff#) else '0'; 

   --Data IN/OUT shift register
   process (RST, CLK)
   begin
      if (RST = '1') then
         --datain_reg <= (others=>'0');
      elsif (CLK'event) and (CLK = '1') then
         if (WRITE_EN='1') and (busy='0') then --load
            data_reg <= DATA_IN;
         elsif (data_en='1') then
            data_reg <= ps2data_reg & data_reg(7 downto 1);
         end if;
      end if;
   end process;

   --Data ODD parity calculator
   process (RST, CLK)
   begin
      if (RST = '1') then
         parity_reg <= '1';
      elsif (CLK'event) and (CLK = '1') then
         if (parity_rst = '1') then
            parity_reg <= '1';
         elsif (parity_en='1') then
            parity_reg <= parity_reg xor parity_in;
         end if;     
      end if;
   end process;

   parity_in <= ps2data_reg when parity_sel='0' else data_reg(0); --data bit mx

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
   process (pstate, di_en, do_en, ps2data_reg, parity_reg, cntr_cmp8, cntr_cmp511, WRITE_EN)
   begin
      nstate <= SInit;
      cntr_rst <= '0';
      cntr_en <= '0';
      parity_rst <= '0';
      parity_sel <= '0';
      parity_en <= '0';
      data_en <= '0';
      data_sel <= "11";
      clk_we <= '0';
      DATA_ERR <= '0';
      DATA_VLD <= '0';
      busy <= '1';

      case pstate is
         when SInit =>
            busy <= '0';
            if (di_en='1') and (ps2data_reg='0') then
               nstate <= SRcvData;
               cntr_rst <= '1';
               parity_rst <= '1';
            end if;
            if (WRITE_EN='1') then
               cntr_rst <= '1';
               nstate <= SStart0;
            end if;

         --====================================================================
         --Receiver
         --====================================================================
         -- Receive data bits
         when SRcvData =>
            if (cntr_cmp8='0') then
               nstate <= SRcvData;
               cntr_en <= di_en;
               data_en <= di_en;
               parity_en <= di_en;
            else
               nstate <= SRcvParity;
            end if;

         --Receive parity bit
         when SRcvParity =>
            if (di_en='1') then
               nstate <= SRcvStop;

               assert (ps2data_reg=parity_reg) report "PS/2 Data Parity Error" severity note; 

               if (ps2data_reg=parity_reg) then 
                  DATA_VLD <= '1';
               else
                  DATA_ERR <= '1';
               end if;
            else
               nstate <= SRcvParity;
            end if;

         --Receive stop bit
         when SRcvStop =>
            if (di_en='0') then
               nstate <= SRcvStop;
            end if;

         --====================================================================
         -- Transmitter
         --====================================================================
         when SStart0 =>
            clk_we <= '1';
            if (cntr_cmp511='1') then
               nstate <= SStart1;
               cntr_rst <= '1';
            else
               nstate <= SStart0;
               cntr_en <= '1';
            end if;

         --Init data transfer
         when SStart1 =>
            clk_we <= '1';
            data_sel <= "10";
            if (cntr_cmp511='1') then
               nstate <= SStart2;
            else
               nstate <= SStart1;
               cntr_en <= '1';
            end if;

         --Release CLK
         when SStart2 =>
            data_sel <= "10";
            if (di_en='1') then
               nstate <= SSndData;
               cntr_rst <= '1';
               parity_rst <= '1';
            else
               nstate <= SStart2;
            end if;

         --Transmit data bits
         when SSndData =>
            data_sel <= "00";
            if (cntr_cmp8='0') then
               nstate <= SSndData;
               parity_sel <= '1';
               parity_en <= di_en;
               data_en <= di_en;
               cntr_en <= di_en;
            else
               nstate <= SSndParity;
            end if;

         --Transmit parity bit
         when SSndParity =>
            data_sel <= "01";
            if (di_en='1') then
               nstate <= SSndStop;
            else
               nstate <= SSndParity;
            end if;

         --Transmit stop bit            
         when SSndStop =>
            data_sel <= "11";
            if (di_en='1') then
               nstate <= SRcvAck;
            else
               nstate <= SSndStop;
            end if;

         --Receive acknowledge
         when SRcvAck =>
            if (do_en='1') then

               assert (ps2data_reg='0') report "PS/2 ACK Error" severity note;

               if (ps2data_reg='1') then
                 DATA_ERR <= '1';
               end if;
            else
               nstate <= SRcvAck;
            end if;

         when others => 
            null;
      end case;
   end process;
end full;

