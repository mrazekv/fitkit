-- serial_rx.vhd : Receiver
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

entity serial_receiver is
   generic (
      SPEED     : serial_speed   := s460800Bd;
      DATAWIDTH : serial_databit := 8;
      STOPBITS  : serial_stopbit := 1;
      PARITY    : serial_parity  := sParityEven
   );
   port (
      CLK      : in std_logic;
      RESET    : in std_logic;

      DATA_OUT : out std_logic_vector(DATAWIDTH-1 downto 0);
      DATA_VLD : out std_logic;

      BUSY     : out std_logic; 
      ERR      : out std_logic;
      ERR_RST  : in  std_logic;

      -- Serial interface
      RXD      : in  std_logic
   );
end serial_receiver;


architecture behavioral of serial_receiver is

   type FSMstate is (SIdle, SWaitStartBit, SDataBits, SCheckParity);
   signal pstate : FSMstate; -- actual state
   signal nstate : FSMstate; -- next state 

   signal rx_en      : std_logic;
   signal rxd_reg    : std_logic_vector(2 downto 0);   
   signal startbit   : std_logic;

   signal smplcnt_reg : std_logic_vector(3 downto 0) := (others => '0');
   signal smplcnt_en  : std_logic;
   signal smplcnt_cmp : std_logic;
   signal sample_en   : std_logic;
   signal sbit_reg    : std_logic_vector(4 downto 0) := (others => '1');   
   signal sbit_majority : std_logic;
   signal parity_reg  : std_logic;
   signal parity_rst  : std_logic;

   signal data_shreg : std_logic_vector(DATAWIDTH downto 0) := (others => '0');
   signal bitcnt_reg : std_logic_vector(3 downto 0);
   signal bitcnt_rst : std_logic;
   signal bitcnt_cmp : std_logic;

   signal err_reg : std_logic;
   signal err_en  : std_logic;

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

   component majority_five
      port (
         D : in  std_logic_vector(4 downto 0);
         Q : out std_logic
      );
   end component;

begin
   -- ===========================================================================================================
   -- Receiver   
   -- ===========================================================================================================
   DATA_OUT <= data_shreg(DATAWIDTH-1 downto 0);

   --Baudrate generator (8x faster than tx)
   rxclk_gen: serial_gen
      generic map (
         SPEED => SPEED,
         FREQMULT => 8
      )
      port map (
         CLK     => CLK,
         RESET   => RESET,
         EN      => '1',
         OUTPUT  => rx_en
      );

   -- rxd sample register
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         rxd_reg <= (others => '1');
      elsif (CLK'event) and (CLK = '1') then
         rxd_reg <= rxd_reg(1 downto 0) & RXD;
      end if;
   end process;

   -- start bit detector
   startbit <= rxd_reg(2) and (not rxd_reg(1)) and (not rxd_reg(0));

   -- Bit Sample counter
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         smplcnt_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (smplcnt_en='1') and (rx_en='1') then
            if (smplcnt_cmp='1') then
               smplcnt_reg <= (others => '0');
            else
               smplcnt_reg <= smplcnt_reg + 1;
            end if;
         end if;
      end if;
   end process;
   -- last sample
   smplcnt_cmp <= '1' when (conv_integer(smplcnt_reg) = 7) else '0';
   -- sample enabled
   sample_en <= '0' when (conv_integer(smplcnt_reg) = 0) or (conv_integer(smplcnt_reg) = 1) or
                         (conv_integer(smplcnt_reg) = 7) else '1';

   -- sample shift register (5 samples)
   process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (sample_en='1') and (rx_en='1') then
            sbit_reg <= sbit_reg(3 downto 0) & rxd_reg(2);
         end if;
      end if;
   end process;

   -- majority of five inputs
   sbitmajority: majority_five
      port map (
         D => sbit_reg,
         Q => sbit_majority
      );

   -- received data shift register
   process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (smplcnt_cmp='1') and (rx_en='1') then
            data_shreg <= sbit_majority & data_shreg(DATAWIDTH downto 1);
         end if;
      end if;
   end process;

   -- Parity register
   process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (parity_rst='1') then
            parity_reg <= '0';
         elsif (smplcnt_cmp='1') and (rx_en='1') then
            parity_reg <= parity_reg xor sbit_majority;
         end if;
      end if;
   end process;

   --Bit counter
   process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (bitcnt_rst='1') then
            bitcnt_reg <= (others => '0');   
         elsif (smplcnt_cmp='1') and (rx_en='1') then 
            bitcnt_reg <= bitcnt_reg + 1;
         end if;
      end if;
   end process;   

   --all bits received comparator
   bitcnt_cmp <= '1' when ((PARITY = sParityNone) and (conv_integer(bitcnt_reg) = DATAWIDTH-1)) or
                 ((PARITY /= sParityNone) and (conv_integer(bitcnt_reg) = DATAWIDTH)) else '0';

   --Error register
   error_reg: process (RESET, CLK)
   begin
      if (RESET = '1') then
         err_reg <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (err_en='1') then
            err_reg <= '1';
         elsif (ERR_RST='1') then
            err_reg <= '0';
         end if;
      end if;
   end process;

   ERR <= err_reg;

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
   nstate_logic: process(pstate, startbit, smplcnt_cmp, rx_en, sbit_majority, bitcnt_cmp, parity_reg)
   begin
      nstate <= SIdle;
      smplcnt_en <= '0';
      bitcnt_rst <= '0';
      DATA_VLD <= '0';
      BUSY <= '1';
      parity_rst <= '0';
      err_en <= '0';

      case pstate is
         when SIdle =>
            BUSY <= '0';
            if (startbit='1') then
               nstate <= SWaitStartBit;
            end if;

         -- Sample start bit
         when SWaitStartBit =>
            nstate <= SWaitStartBit;

            smplcnt_en <= '1';
            if (smplcnt_cmp='1') and (rx_en='1') then --first bit sampled
               if (sbit_majority='0') then
                  nstate <= SDataBits;
                  bitcnt_rst <= '1';
                  parity_rst <= '1';
               else 
                  nstate <= SIdle;
               end if;
            end if;

         -- Receive databits
         when SDataBits =>
            nstate <= SDataBits;
            smplcnt_en <= '1';

            if (smplcnt_cmp='1') and (rx_en='1') and (bitcnt_cmp='1') then
               if (PARITY = sParityNone) then
                  nstate <= SIdle;
                  -- Received valid bits
                  DATA_VLD <= '1';
               else
                  nstate <= SCheckParity;
               end if;
            end if;

         -- Parity check
         when SCheckParity =>
            nstate <= SCheckParity;

            smplcnt_en <= '1';
            if ((PARITY = sParityEven) and (parity_reg='0')) or ((PARITY = sParityOdd) and (parity_reg='1')) then
               nstate <= SIdle;
               -- Received valid bits
               DATA_VLD <= '1';
            else
               err_en <= '1';
               nstate <= SIdle;
            end if;

         when others => null;
      end case;
   end process;

end behavioral;

