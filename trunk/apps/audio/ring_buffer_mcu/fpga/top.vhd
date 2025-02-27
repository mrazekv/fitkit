-- top.vhd: Digital Audio Codec + SPI ring buffer
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

  signal bram_dout : std_logic_vector(31 downto 0);
  signal bram_din : std_logic_vector(31 downto 0);
  signal bram_addr : std_logic_vector(8 downto 0);
  signal bram_addr_sel : std_logic;
  signal bram_we : std_logic;
  signal bram_en : std_logic;

  signal addr_wr : std_logic_vector(8 downto 0);
  signal addr_rd : std_logic_vector(8 downto 0);

  signal spi_dout : std_logic_vector(7 downto 0);
  signal ctrl_reg : std_logic_vector(7 downto 0);
  signal spi_we : std_logic;

  signal susp_cntr : std_logic_vector(14 downto 0) := (others => '0');
  signal susp_cmp  : std_logic;
  signal dsp_en    : std_logic;
  
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

  component spi_ringbuffer is
    port (
      -- Control signals
      RESET    : in std_logic;
      CLK      : in std_logic;
      EN       : in std_logic;
      READY    : out std_logic; -- sample ready
      OVERFLOW : out std_logic; -- buffer overflow
  
      -- BRAM Ring Buffer interface
      BRAM_ADDR : out std_logic_vector(8 downto 0);
      BRAM_DOUT : in std_logic_vector(31 downto 0);
      BRAM_DIN  : out std_logic_vector(31 downto 0);
      BRAM_EN   : out std_logic;
      BRAM_WE   : out std_logic;
  
      ADDR_RD   : in std_logic_vector(8 downto 0);
      ADDR_WR   : in std_logic_vector(8 downto 0);
    
      -- SPI interface
      MOSI    : in std_logic;
      MISO    : out std_logic;
      SCK     : in std_logic
    );
  end component;

  component SPI_adc
     generic (
        ADDR_WIDTH : integer;
        DATA_WIDTH : integer;
        ADDR_OUT_WIDTH : integer;
        BASE_ADDR  : integer
     );
     port (
        CLK      : in  std_logic;

        CS       : in  std_logic;
        DO       : in  std_logic;
        DO_VLD   : in  std_logic;
        DI       : out std_logic;
        DI_REQ   : in  std_logic;

        ADDR     : out  std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
        DATA_OUT : out  std_logic_vector (DATA_WIDTH-1 downto 0);
        DATA_IN  : in   std_logic_vector (DATA_WIDTH-1 downto 0);

        WRITE_EN : out  std_logic;
        READ_EN  : out  std_logic
     );
  end component;
  
begin

  -- SPI decoder (access to the control register)
  spidec: SPI_adc
     generic map (
        ADDR_WIDTH => 8,      -- address width 8 bits
        DATA_WIDTH => 8,     -- data width 8 bits
        ADDR_OUT_WIDTH => 1,  -- unused
        BASE_ADDR  => 16#F0#  -- base address 0xF0
     )
     port map (
        CLK      => CLK,
        CS       => SPI_CS,

        DO       => SPI_DO,
        DO_VLD   => SPI_DO_VLD,
        DI       => SPI_DI,
        DI_REQ   => SPI_DI_REQ,

        ADDR     => open,
        DATA_OUT => spi_dout,
        DATA_IN  => ctrl_reg,
        WRITE_EN => spi_we,
        READ_EN  => open
     );  

  control_reg:process (RESET, CLK)
  begin
     if (RESET = '1') then
         ctrl_reg(6 downto 0) <= (others => '0');
     elsif (CLK'event) and (CLK = '1') then
         if (spi_we = '1') then
            ctrl_reg(6 downto 0) <= spi_dout(6 downto 0);
         end if;
     end if;
  end process;

  aud_wire : aic23b_dsp_ringbuffer 
    port map (
      CLK     => CLK,
      RST     => RESET,
      EN      => dsp_en,

      ALRCOUT => ALRCOUT,
      ALRCIN  => ALRCIN,
      ABCLK   => ABCLK,
      ADOUT   => ADOUT,
      ADIN    => ADIN,

      BRAM_ADDR => bram_addr,
      BRAM_DOUT => bram_dout,
      BRAM_DIN  => bram_din,
      BRAM_EN   => bram_en,
      BRAM_WE   => bram_we,
      ADDR_RD   => addr_rd,
      ADDR_WR   => addr_wr
    );

  spi_ctrl: spi_ringbuffer
    port map (
      CLK       => CLK,
      RESET     => RESET,
      EN        => ctrl_reg(1),
      READY     => P3M(7),
      OVERFLOW  => ctrl_reg(7),

      BRAM_ADDR => bram_addr,
      BRAM_DOUT => bram_dout,
      BRAM_DIN  => bram_din,
      BRAM_EN   => bram_en,
      BRAM_WE   => bram_we,
      ADDR_RD   => addr_rd,
      ADDR_WR   => addr_wr,
 
      MOSI => P3M(1),
      MISO => P3M(2),
      SCK  => P3M(3)
    );

   dsp_en <= ctrl_reg(0) and (not susp_cmp);

   -- Watchdog disables DSPs approx. 1ms after overflow has been detected
   suspend_counter:process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (spi_we = '1') then
            susp_cntr <= (others => '0'); 
         elsif (ctrl_reg(7) = '1') and (susp_cmp = '0') then
            susp_cntr <= susp_cntr + 1;
         end if;
      end if;
   end process;

   susp_cmp <= '1' when susp_cntr = "111111111111111" else '0';
   
   LEDF <= not susp_cmp;
end main;

