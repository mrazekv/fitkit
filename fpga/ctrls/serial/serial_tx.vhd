-- serial_tx.vhd : Transmitter
-- Copyright (C) 2007 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek (xvasic11 AT stud.fit.vutbr.cz)
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
use work.serial_cfg.all;

entity serial_transmitter is
   generic (
      SPEED     : serial_speed   := s460800Bd;
      DATAWIDTH : serial_databit := 8;
      STOPBITS  : serial_stopbit := 1;
      PARITY    : serial_parity  := sParityEven
   );
   port (
      -- Common interface
      CLK      : in std_logic;
      RESET    : in std_logic;

      DATA_IN  : in std_logic_vector(DATAWIDTH-1 downto 0);
      WRITE_EN : in std_logic;

      BUSY     : out std_logic; 

      -- Serial interface
      TXD      : out std_logic
   );
end serial_transmitter;


architecture behavioral of serial_transmitter is

   type FSMstate is (SIdle, SWaitTx, SStartbit, SDataBits, SParityBit, SStopBit1, SStopBit2, SStopBit3);
   signal pstate : FSMstate; -- actual state
   signal nstate : FSMstate; -- next state 

   signal tx_en      : std_logic;
   signal data_shreg : std_logic_vector(DATAWIDTH-1 downto 0);
   signal data_shen  : std_logic;
   signal bitcnt_reg : std_logic_vector(3 downto 0);
   signal bitcnt_en  : std_logic;
   signal bitcnt_rst : std_logic;
   signal bit_cmp    : std_logic;
   signal outmx_sel  : std_logic_vector(1 downto 0);
   signal parity_reg : std_logic;
   signal txbusy_reg : std_logic;
   signal txbusy_rst : std_logic;

   component serial_gen
      generic (
         SPEED     : serial_speed;
         FREQMULT  : positive
      );
      port(
         CLK    : in  std_logic;
         RESET  : in  std_logic;
         EN     : in  std_logic;
   
         OUTPUT : out std_logic
      );
   end component;
   
begin
   -- ===========================================================================================================
   -- Transmitter
   -- ===========================================================================================================
   TXD <= '1' when outmx_sel="00" else --idle, stop bit
          '0' when outmx_sel="01" else --start bit
          data_shreg(0) when outmx_sel="10" else --data bit
          parity_reg; --parity bit

   BUSY <= txbusy_reg;

   --Baudrate generator
   txclk_gen: serial_gen
      generic map (
         SPEED => SPEED,
         FREQMULT => 1
      )
      port map (
         CLK     => CLK,
         RESET   => RESET,
         EN      => '1',
         OUTPUT  => tx_en
      );

   -- Busy register
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         txbusy_reg <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (txbusy_rst='1') then
            txbusy_reg <= '0';
         elsif (WRITE_EN='1') and (txbusy_reg='0') then
            txbusy_reg <= '1';
         end if;
      end if;
   end process;

   -- Data shift register
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (WRITE_EN='1') and (txbusy_reg='0') then -- parallel load 
            data_shreg <= DATA_IN;

            if (PARITY = sParityEven) then
               parity_reg <= '0'; --even parity (suda parita)
            else
               parity_reg <= '1'; --odd parity (licha parita)
            end if;

         elsif (data_shen='1') and (tx_en='1') then -- shift enable
            data_shreg <= '0' & data_shreg(DATAWIDTH-1 downto 1);
            parity_reg <= parity_reg xor data_shreg(0);
         end if;
      end if;
   end process;

   --Bit counter
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (bitcnt_rst='1') then
            bitcnt_reg <= (others => '0');   
         elsif (bitcnt_en='1') and (tx_en='1') then 
            bitcnt_reg <= bitcnt_reg + 1;
         end if;
      end if;
   end process;   

   --bit comparator
   bit_cmp <= '1' when (conv_integer(bitcnt_reg) = DATAWIDTH-1) else '0';

   --Present State registr
   pstatereg: process(RESET, CLK)
   begin
      if (RESET='1') then
         pstate <= SIdle;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;
   
   --Next State logic, Output logic
   nstate_logic: process(pstate, txbusy_reg, tx_en, bit_cmp)
   begin
      data_shen <= '0';
      outmx_sel <= "00";
      bitcnt_en <= '0';
      bitcnt_rst <= '0';
      txbusy_rst <= '0';

      nstate <= SIdle;
      case pstate is
         when SIdle =>
            if (txbusy_reg='1') then
               nstate <= SWaitTx;
            end if;

         --Synchronization with baudrate generator
         when SWaitTx =>
            nstate <= SWaitTx;

            if (tx_en='1') then
               nstate <= SStartBit;

               bitcnt_rst <= '1';
            end if;

         --Start bit
         when SStartBit =>
            nstate <= SStartbit;

            outmx_sel <= "01";
            if (tx_en='1') then
               nstate <= SDataBits;
            end if;

         -- Data bits
         when SDataBits =>
            nstate <= SDataBits;

            data_shen <= '1';
            bitcnt_en <= '1';
            outmx_sel <= "10";
            if (tx_en='1') and (bit_cmp='1') then
               if (PARITY = sParityNone) then
                  if (STOPBITS=2) then -- two stop bits
                     nstate <= SStopBit1;
                  else -- one stop bit
                     nstate <= SStopBit2;
                  end if;
               else
                  nstate <= SParityBit;
               end if;
            end if;

         -- Parity bit
         when SParityBit =>
            nstate <= SParityBit;

            outmx_sel <= "11";
            if (tx_en='1') then
               if (STOPBITS=2) then -- two stop bits
                  nstate <= SStopBit1;
               else -- one stop bit
                  nstate <= SStopBit2;
               end if;
            end if;

         -- Stop bit
         when SStopBit1 =>
            nstate <= SStopBit1;

            outmx_sel <= "00";
            if (tx_en='1') then
               nstate <= SStopBit2;
            end if;

         -- Stop bit - busy reset
         when SStopBit2 =>
            nstate <= SStopBit3;

            outmx_sel <= "00";
            txbusy_rst <= '1';

         -- Stop bit
         when SStopBit3 =>
            nstate <= SStopBit3;

            outmx_sel <= "00";
            if (tx_en='1') then
               if (txbusy_reg='1') then
                  nstate <= SStartBit;

                  bitcnt_rst <= '1';
               else
                  nstate <= SIdle;
               end if;
            end if;

         when others => null;
      end case;
   end process;

end behavioral;

