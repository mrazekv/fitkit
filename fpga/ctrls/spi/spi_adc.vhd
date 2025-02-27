-- spi_adc.vhd : SPI Decoder
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

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.math_pack.all;

-- SYNTH-ISE: slices=28, slicesFF=36, 4luts=47
architecture basic of SPI_adc is

subtype tcntr is integer range 0 to max(ADDR_WIDTH, DATA_WIDTH);
signal  bitcntr : tcntr := 0;
signal  bitcntr_rst : std_logic;

signal  cmp_4bits : std_logic;
signal  cmp_baseaddr : std_logic;
signal  cmp_abits : std_logic;
signal  cmp_dbits : std_logic;
signal  cmp_bitszero : std_logic;

type    FSMstate is (SCmdRcv, SAddrRcv, SDataRcv, SNop);
signal  pstate    : FSMstate; -- actual state
signal  nstate    : FSMstate; -- next state

signal  shreg_cmd  : std_logic_vector(1 downto 0);
signal  cmdreg_en  : std_logic;
alias   cmd_we     : std_logic is shreg_cmd(0);
alias   cmd_re     : std_logic is shreg_cmd(1);
signal  shreg_addr : std_logic_vector(ADDR_WIDTH-1 downto 0);
signal  addrreg_en : std_logic;
signal  shreg_do   : std_logic_vector(DATA_WIDTH-1 downto 0);
signal  shreg_di   : std_logic_vector(DATA_WIDTH-1 downto 0);
signal  shreg_ld   : std_logic;
signal  doreg_en   : std_logic;
signal  rden       : std_logic;
signal  diouten    : std_logic;
signal  rden_reg   : std_logic_vector(max(DELAY,0) downto 0);

begin

   DATA_OUT <= shreg_do;
   ADDR <= shreg_addr(max(ADDR_OUT_WIDTH-1,0) DOWNTO 0);
   READ_EN <= rden;

   -- DI MX
   DI <= shreg_di(DATA_WIDTH-1) when (diouten='1') else 'Z';

   -- Bits counter
   bitecntr: process(CS, CLK)
   begin
      if (CS = '0') then
         bitcntr <= 0;
      elsif (CLK'event) and (CLK='1') then
         if (bitcntr_rst = '1') then
            bitcntr <= 0;
         elsif (DO_VLD='1') then
            bitcntr <= bitcntr + 1;
         end if;
      end if;
   end process;

   -- Comparators
   cmp_4bits <= '1' when (bitcntr = 4) else '0';
   cmp_abits <= '1' when (bitcntr = ADDR_WIDTH) else '0';
   cmp_dbits <= '1' when (bitcntr = DATA_WIDTH) else '0';
   cmp_bitszero <= '1' when (bitcntr = 0) else '0';

   cmp_baseaddr <= '1' when (shreg_addr(ADDR_WIDTH-1 downto ADDR_OUT_WIDTH) =
                             conv_std_logic_vector(BASE_ADDR,ADDR_WIDTH)(ADDR_WIDTH-1 downto ADDR_OUT_WIDTH))
                       else '0';

   -- ShReg Load delay
   shreg_ld <= rden_reg(DELAY);
   rden_reg(0) <= rden;
   process (CS, CLK)
   begin
      if (DELAY > 0) then
         if (CS = '0') then
            rden_reg(DELAY downto 1) <= (others=>'0');
         elsif (CLK'event) and (CLK='1') then
            rden_reg(DELAY downto 1) <= rden_reg(DELAY-1 downto 0);
         end if;
      end if;
   end process;

   -- Address register
   process (CS, CLK)
   begin
      if (CS = '0') then
         shreg_addr <= (others=>'0');
      elsif (CLK'event) and (CLK = '1') then
         if (DO_VLD='1') and (addrreg_en='1') then -- Shift enable
            shreg_addr <= shreg_addr(ADDR_WIDTH-2 downto 0) & DO;
         end if;
      end if;
   end process;

   -- Shift registers
   process (CS, CLK)
   begin
      if (CS = '0') then
         shreg_cmd <= (others=>'0');
         shreg_do  <= (others=>'0');
         shreg_di  <= (others=>'0');
      elsif (CLK'event) and (CLK='1') then

         if (DO_VLD='1') then -- shift enable
            if (cmdreg_en='1') then  -- CMD register
               shreg_cmd <= shreg_cmd(0) & DO;
            end if;
            if (doreg_en='1') then   -- DATA register
               shreg_do <= shreg_do(DATA_WIDTH-2 downto 0) & DO;
            end if;
         end if;

         if (shreg_ld='1') then  -- parallel load
            shreg_di <= DATA_IN;
         elsif (DI_REQ='1') then -- shift enable
            shreg_di <= shreg_di(DATA_WIDTH-2 downto 0)&'0';
         end if;
      end if;
   end process;

   -- FSM
   process (CS, CLK)
   begin
      if (CS = '0') then
         pstate <= SCmdRcv;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;

   -- FSM next state logic
   process (pstate, cmp_4bits, cmp_abits, cmp_dbits, cmp_bitszero, cmp_baseaddr, shreg_cmd, cmd_we, cmd_re, DI_REQ)
   begin
      bitcntr_rst <= '0';
      cmdreg_en <= '0';
      doreg_en <= '0';
      addrreg_en <= '0';
      WRITE_EN <= '0';
      rden <= '0';
      diouten <= '0';

      nstate <= SCmdRcv;
      case pstate is
         --CMD receive
         when SCmdRcv =>
            cmdreg_en <= '1';
            if (cmp_4bits = '1') then
               nstate <= SAddrRcv;
               bitcntr_rst <= '1';
            end if;

         --ADDR receive
         when SAddrRcv =>
            nstate <= SAddrRcv;
            addrreg_en <= '1';
            if (cmp_abits = '1') then
               if (cmp_baseaddr = '1') then
                  nstate <= SDataRcv;
               else
                  nstate <= SNop;
               end if;
               bitcntr_rst <= '1';
            end if;

         --DATA receive & transmitt
         when SDataRcv =>
            nstate <= SDataRcv;
            doreg_en <= '1';
            diouten <= '1';
            if (cmp_bitszero = '1') and (DI_REQ = '1') then
               rden <= cmd_re;
            end if;
            if (cmp_dbits = '1') then
               WRITE_EN <= cmd_we;
               bitcntr_rst <= '1';
            end if;

          --inactive device (different BASE_ADDR)
          when SNop =>
            nstate <= SNop;
            bitcntr_rst <= '1';

     end case;
   end process;


end basic;
