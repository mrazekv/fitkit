-- top.vhd: Digital Audio Codec Ring Buffer, DSP mode
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Karel Slany <slany AT fit.vutbr.cz> 
--            Zdenek Vasicek <vasicek AT fit.vutbr.cz> 
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

-- pragma translate_off
library unisim;
use unisim.vcomponents.all;
-- pragma translate_on

-- digital 16b audio transceiving unit
-- the audio codec has to be configured to operate in DSP master mode
entity aic23b_dsp_ringbuffer is
  port (
    -- Control signals
    RST : in std_logic;
    CLK : in std_logic;
    EN  : in std_logic;

    -- Ring Buffer interface
    BRAM_ADDR : in std_logic_vector(8 downto 0);
    BRAM_DOUT : out std_logic_vector(31 downto 0);
    BRAM_DIN  : in std_logic_vector(31 downto 0);
    BRAM_EN   : in std_logic;
    BRAM_WE   : in std_logic;

    ADDR_RD   : out std_logic_vector(8 downto 0);
    ADDR_WR   : out std_logic_vector(8 downto 0);
  
    -- CODEC digital interface
    ALRCOUT : in std_logic;
    ALRCIN  : in std_logic;
    ABCLK   : in std_logic;
    ADOUT   : in std_logic;
    ADIN    : out std_logic
  );
end entity aic23b_dsp_ringbuffer;

architecture basic of aic23b_dsp_ringbuffer is

  component aic23b_dsp_read
    generic (
      DATA_WIDTH : integer := 16
    );
    port (
      CLK        : in std_logic;
      RST        : in std_logic;
      ALRCOUT    : in std_logic;
      ABCLK      : in std_logic;
      ADOUT      : in std_logic;
      LOUT       : out std_logic_vector(DATA_WIDTH-1 downto 0);
      ROUT       : out std_logic_vector(DATA_WIDTH-1 downto 0);
      DATA_VALID : out std_logic
    );
  end component aic23b_dsp_read;
  
  component aic23b_dsp_write is
    generic (
      DATA_WIDTH : integer := 16
    );
    port (
      CLK     : in std_logic;
      RST     : in std_logic;
      ALRCIN  : in std_logic;
      ABCLK   : in std_logic;
      ADIN    : out std_logic;
      LIN     : in std_logic_vector(DATA_WIDTH-1 downto 0);
      RIN     : in std_logic_vector(DATA_WIDTH-1 downto 0);
      READY   : out std_logic;
      DATA_REQ: out std_logic
    );
  end component aic23b_dsp_write;
  
  component RAMB16_S36_S36
    port (
      DOA   : out std_logic_vector(31 downto 0); -- 32b output
      DOPA  : out std_logic_vector(3 downto 0);  -- 4b parity
      ADDRA : in std_logic_vector(8 downto 0);   -- address
      CLKA  : in std_ulogic;                     -- clock
      DIA   : in std_logic_vector(31 downto 0);  -- 32b input
      DIPA  : in std_logic_vector(3 downto 0);   -- 4b parity
      ENA   : in std_ulogic;                     -- enable read/write
      SSRA  : in std_ulogic;                     -- synchronous set/reset input
      WEA   : in std_logic;                      -- enable write input
      
      DOB   : out std_logic_vector(31 downto 0);
      DOPB  : out std_logic_vector(3 downto 0);
      ADDRB : in std_logic_vector(8 downto 0);
      CLKB  : in std_ulogic;
      DIB   : in std_logic_vector(31 downto 0);
      DIPB  : in std_logic_vector(3 downto 0);
      ENB   : in std_ulogic;
      SSRB  : in std_ulogic;
      WEB   : in std_logic
    );
  end component RAMB16_S36_S36;
  
  signal sample_in     : std_logic_vector(31 downto 0);
  signal sample_valid  : std_logic;
  signal sample_out    : std_logic_vector(31 downto 0);
  signal sample_req    : std_logic;
  signal bram_out      : std_logic_vector(31 downto 0);
  
  signal wr_cntr      : std_logic_vector(8 downto 0);
  signal rd_cntr      : std_logic_vector(8 downto 0);
  signal addr         : std_logic_vector(8 downto 0);
  signal serialout    : std_logic;

begin
  
  -- read unit (receiver)
  aud_rd : aic23b_dsp_read 
     generic map (
       DATA_WIDTH => 16
     )
     port map (
       CLK     => CLK,
       RST     => RST,
       ALRCOUT => ALRCOUT,
       ABCLK   => ABCLK,
       ADOUT   => ADOUT,
       LOUT    => sample_in(31 downto 16),
       ROUT    => sample_in(15 downto 0),
       DATA_VALID => sample_valid
     );
  
  -- write unit (transmitter)
  aud_wr : aic23b_dsp_write 
     generic map (
       DATA_WIDTH => 16
     )
     port map (
       CLK    => CLK,
       RST    => RST,
       READY  => open,
       ALRCIN => ALRCIN,
       ABCLK  => ABCLK,
       ADIN   => serialout,
       LIN    => sample_out(31 downto 16),
       RIN    => sample_out(15 downto 0),
       DATA_REQ  => sample_req
     );
  
  ADIN <= serialout and EN;

  -- ring buffer (512 x 32bit items)
  bram : RAMB16_S36_S36 
     port map (
       DOA   => bram_out,
       DOPA  => open,
       ADDRA => addr,
       CLKA  => CLK,
       DIA   => sample_in,
       DIPA  => (others => '0'),
       ENA   => '1',
       SSRA  => '0',
       WEA   => sample_valid,
         
       DOB   => BRAM_DOUT,
       DOPB  => open,
       ADDRB => BRAM_ADDR,
       CLKB  => CLK,
       DIB   => BRAM_DIN,
       DIPB  => (others => '0'),
       ENB   => BRAM_EN,
       SSRB  => '0',
       WEB   => BRAM_WE
     );

  -- write enable and increase write address counter  
  addr_wr_cntr : process (RST, CLK)
  begin
    if (RST = '1') then
      wr_cntr <= (others => '0');
    elsif (CLK'event) and (CLK = '1') then
      if (sample_valid = '1') then
        if (EN = '1') then
          wr_cntr <= wr_cntr + 1;
        end if;
      end if;
    end if;
  end process;
  
  ADDR_WR <= wr_cntr;

  -- multiplexed address input and read from bram
  addr <= wr_cntr when (sample_valid = '1') else rd_cntr;

  read_reg : process (CLK)
  begin
    if (CLK'event) and (CLK = '1') then
        -- when input data not written
      if (sample_req = '1') then
         sample_out <= bram_out;
      end if;
    end if;
  end process;
  
  -- read from BRAM and send data
  addr_rd_cntr : process (RST, CLK)
  begin
    if (RST = '1') then
       rd_cntr <= "000000001";
    elsif (CLK'event) and (CLK = '1') then
       if (sample_req = '1') then
         if (EN = '1') then
           rd_cntr <= rd_cntr + 1;
         end if;
       end if;
    end if;
  end process;
 
  ADDR_RD <= rd_cntr;

end architecture basic;
