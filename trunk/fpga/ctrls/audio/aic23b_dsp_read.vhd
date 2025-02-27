-- top.vhd: Digital Audio Codec Receiving Interface in DSP Communication Mode
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Karel Slany <slany AT fit.vutbr.cz>
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

-- digital audio receiving unit
-- the audio codec has to be configured to operate in DSP master mode
entity aic23b_dsp_read is
  generic (
    DATA_WIDTH : integer := 16
  );
  port (
    -- Control signals 
    CLK : in std_logic;
    RST : in std_logic;

    -- DATA interface 
    LOUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
    ROUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
    DATA_VALID : out std_logic;

    -- CODEC interface
    ALRCOUT : in std_logic;
    ABCLK : in std_logic;
    ADOUT : in std_logic
  );
end entity aic23b_dsp_read;

architecture basic of aic23b_dsp_read is

  -- ABCLK rising edge detector and sampler
  signal abclk_re : std_logic;
  signal abclk_last : std_logic_vector(1 downto 0);
  signal alrcout_last : std_logic;
  signal adout_last : std_logic;
  
  -- sampled data from deserializer 
  signal data : std_logic_vector(DATA_WIDTH-1 downto 0);
  
  -- sample bit counter
  signal start_cntr : std_logic;
  signal bit_cntr : std_logic_vector(log2(DATA_WIDTH)-1 downto 0);
  signal bit_cntr_load : std_logic;
  
  -- output finite state machine
  type FSMstate is (SInit, SReadL, SWriteL, SReadR, SWriteR, SWriteRReadL);
  signal pstate : FSMstate; -- actual state
  signal nstate : FSMstate; -- next state
  signal lwrite : std_logic;
  signal rwrite : std_logic;
  
begin
  
  -- ABCLK rising edge detector and sampler
  abclk_re <= '1' when (abclk_last(1) = '0') and (abclk_last(0) = '1') else '0';
  abclk_detect_p : process (RST, CLK, ABCLK, ALRCOUT, ADOUT)
  begin
    if (RST = '1') then
      abclk_last <= ABCLK & ABCLK;
      alrcout_last <= ALRCOUT;
      adout_last <= ADOUT;
    elsif (CLK'event) and (CLK = '1') then
      abclk_last <= abclk_last(0) & ABCLK;
      alrcout_last <= ALRCOUT;
      adout_last <= ADOUT;
    end if;
  end process abclk_detect_p;
  
  -- deserializer
  deserializer_p : process (CLK)
  begin
    if (CLK'event) and (CLK='1') then
      if (abclk_re='1') then
        data <= data(DATA_WIDTH-2 downto 0) & adout_last;
      end if;
    end if;
  end process deserializer_p;
  
  -- bit counter process
  start_cntr <= '1' when (abclk_re = '1') and (alrcout_last = '1') else '0';
  bit_cntr_load <= '1' when (bit_cntr = conv_std_logic_vector(DATA_WIDTH-1, log2(DATA_WIDTH))) else '0';
  bit_cntr_p : process (RST, CLK)
  begin
    if (RST = '1') then
      bit_cntr <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      -- synchronous reset
      if (start_cntr = '1') then
        bit_cntr <= (others => '0');
      -- increase counter when sample ready
      -- abclk_re must be tested, signals may last several
      -- CLK but only one abclk_re
      elsif (abclk_re = '1') then
        if (bit_cntr_load = '1') then
          bit_cntr <= (others => '0');
        else
          bit_cntr <= bit_cntr + 1;
        end if;
      end if;
    end if;
  end process bit_cntr_p;
  
  -- present state process
  pstate_p : process (RST, CLK)
  begin
    if (RST = '1') then
      pstate <= SInit;
    elsif (CLK'event) and (CLK = '1') then
      pstate <= nstate;
    end if;
  end process pstate_p;
  
  -- next state logic, output driving
  nstate_p : process (pstate, start_cntr, abclk_re, bit_cntr_load)
  begin
    -- default values
    nstate <= SInit;
    lwrite <= '0';
    rwrite <= '0';

    case pstate is
      
      when Sinit =>
        if (start_cntr = '1') then
          nstate <= SReadL;
        end if;
        
      when SReadL =>
        if (start_cntr = '1') then
          nstate <= SReadL;
        -- abclk_re must be tested, signals may last several
        -- CLK but only one abclk_re
        elsif (abclk_re = '1') and (bit_cntr_load = '1') then
          nstate <= SWriteL;
        else
          nstate <= SReadL;
        end if;
        
      when SWriteL =>
        lwrite <= '1';
        if (start_cntr = '1') then
          nstate <= SReadL;
        else
          nstate <= SReadR;
        end if;
        
      when SReadR =>
        -- abclk_re must be tested, signals may last several
        -- CLK but only one abclk_re
        if (abclk_re = '1') and (bit_cntr_load = '1') then
          if (start_cntr = '1') then
            -- last sample arrived during next window anouncement
            nstate <= SWriteRReadL;
          else
            nstate <= SWriteR;
          end if;
        elsif (start_cntr = '1') then
          nstate <= SReadL;
        else
          nstate <= SReadR;
        end if;
        
      when SWriteR =>
        rwrite <= '1';
        if (start_cntr = '1') then
          nstate <= SReadL;
        else
          nstate <= SInit;
        end if;
        
      when SWriteRReadL =>
        rwrite <= '1';
        if (start_cntr = '1') then
          nstate <= SReadL;
        else
          nstate <= SReadL;
        end if;
        
      when others => null;

    end case;
  end process nstate_p;
  
  -- output registers
  output_p : process (RST, CLK)
  begin
    if (RST = '1') then
      LOUT <= (others => '0');
      ROUT <= (others => '0');
      DATA_VALID <= '0';
    elsif (CLK'event) and (CLK = '1') then
      DATA_VALID <= '0';
      if (lwrite = '1') then
        LOUT <= data;
      end if;
      if (rwrite = '1') then
        DATA_VALID <= '1';
        ROUT <= data;
      end if;
    end if;
  end process output_p;
  
end architecture basic;
