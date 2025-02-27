-- top.vhd: Digital Audio Codec FPGA Wire Connection
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Karel Slany, slany@fit.vutbr.cz
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

architecture main of tlv_bare_ifc is

  component aic23b_dsp_ringbuffer is
    port (
      -- Control signals
      RST     : in std_logic;
      CLK     : in std_logic;
      EN      : in std_logic;
  
      -- Ring Buffer interface
      BRAM_ADDR : in std_logic_vector(8 downto 0);
      BRAM_DOUT : out std_logic_vector(31 downto 0);
      BRAM_DIN  : in std_logic_vector(31 downto 0);
      BRAM_EN   : in std_logic;
      BRAM_WE   : in std_logic;
  
      ADDR_RD : out std_logic_vector(8 downto 0);
      ADDR_WR : out std_logic_vector(8 downto 0);
    
      -- CODEC digital interface
      ALRCOUT : in std_logic;
      ALRCIN  : in std_logic;
      ABCLK   : in std_logic;
      ADOUT   : in std_logic;
      ADIN    : out std_logic
    );
  end component;

begin

  P3M(3 downto 1) <= (others => '0');

  aud_wire : aic23b_dsp_ringbuffer 
    port map (
      CLK     => CLK,
      RST     => RESET,
      EN      => '1',
      ALRCOUT => ALRCOUT,
      ALRCIN  => ALRCIN,
      ABCLK   => ABCLK,
      ADOUT   => ADOUT,
      ADIN    => ADIN,
      ADDR_RD => open,
      ADDR_WR => open,
      BRAM_ADDR => (others => '0'),
      BRAM_DOUT => open,
      BRAM_DIN  => (others => '0'),
      BRAM_EN   => '0',
      BRAM_WE   => '0'
    );

end main;

