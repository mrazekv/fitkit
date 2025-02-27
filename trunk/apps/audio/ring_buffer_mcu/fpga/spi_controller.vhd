-- spi_controler.vhd: SPI controller
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

entity spi_ringbuffer is
  port (
    -- Control signals
    RESET    : in std_logic;
    CLK      : in std_logic;
    EN       : in std_logic;  -- enable
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
end entity;


architecture main of spi_ringbuffer is

  signal wr_cntr : std_logic_vector(8 downto 0);
  signal rd_cntr : std_logic_vector(8 downto 0);
  signal wr_cntr_en : std_logic;
  signal rd_cntr_en : std_logic;

  signal sck_reg : std_logic_vector(1 downto 0);
  signal sck_re : std_logic;
  signal sck_fe : std_logic;
  signal sample_ready : std_logic;
  signal di_reg : std_logic;

  signal bit_cntr : std_logic_vector(4 downto 0);
  signal bit_cntr_last : std_logic;
  signal data_shreg : std_logic_vector(31 downto 0);
  signal data_shreg_en : std_logic;
  signal data_shreg_ld : std_logic;
  signal bram_addr_sel : std_logic;
  signal overflow_reg : std_logic;

  type FSMstate is (SInit, SReadSample, SDataTrans, SWriteSample);
  signal pstate : FSMstate; -- actual state
  signal nstate : FSMstate; -- next state 
begin
   -- je alespon jeden vzorek v bufferu?
   sample_ready <= '1' when rd_cntr /= ADDR_WR else '0';

   OVERFLOW <= overflow_reg;

   -- hlidani preteceni
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         overflow_reg <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (wr_cntr = ADDR_RD) then
            overflow_reg <= '1';
         end if;
      end if;
   end process;
   
   sck_sample: process (RESET, CLK, SCK)
   begin
      if (RESET = '1') then
         sck_reg <= SCK & SCK;
      elsif (CLK'event) and (CLK = '1') then
         sck_reg <= sck_reg(0) & SCK;
         di_reg <= MOSI;
      end if;
   end process;

   --SCK rising edge
   sck_re <= '1' when (sck_reg(1) = '0') and (sck_reg(0) = '1') else '0';
   --SCK fallig edge
   sck_fe <= '1' when (sck_reg(1) = '1') and (sck_reg(0) = '0') else '0';

   bit_counter: process (RESET, CLK)
   begin
      if (RESET = '1') then
         bit_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (EN = '1') and (sck_re = '1') then
            bit_cntr <= bit_cntr + 1;
         end if;
      end if;
   end process;

   bit_cntr_last <= '1' when bit_cntr = "11111" else '0';

   misoreg: process (RESET, CLK)
   begin
      if (RESET = '1') then
         MISO <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (sck_fe = '1') then
            MISO <= data_shreg(31);
         end if;
      end if;
   end process;

   spi_shift_register: process (CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (data_shreg_ld = '1') then
            data_shreg <= BRAM_DOUT; 
         elsif (data_shreg_en = '1') and (sck_re = '1') then
            data_shreg <= data_shreg(30 downto 0) & di_reg;
         end if;
      end if;
   end process;

   BRAM_DIN <= data_shreg;

   cntrs: process (RESET, CLK)
   begin
      if (RESET = '1') then
         rd_cntr <= (others => '0');
         wr_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (rd_cntr_en = '1') then
            rd_cntr <= rd_cntr + 1;
         end if;
         if (wr_cntr_en = '1') then
            wr_cntr <= wr_cntr + 1;
         end if;
      end if;
   end process;    

   BRAM_ADDR <= rd_cntr when bram_addr_sel = '0' else wr_cntr;

   --Present State registr
   pstatereg: process(RESET, CLK)
   begin
      if (RESET='1') then
         pstate <= SInit;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;
   
   --Next State logic, Output logic
   nstate_output_logic: process(pstate, sample_ready, sck_fe, sck_re, bit_cntr_last, EN)
   begin
      bram_addr_sel <= '0';
      data_shreg_ld <= '0';
      data_shreg_en <= '0';
      wr_cntr_en <= '0';
      rd_cntr_en <= '0';
      BRAM_WE <= '0';
      BRAM_EN <= '0';
      READY <= '0';

      nstate <= SInit;
      case pstate is
         when SInit =>
             nstate <= SInit;
 
             if (sample_ready = '1') and (EN = '1') then
                nstate <= SReadSample;
                bram_addr_sel <= '0';
                BRAM_EN <= '1';
             end if;

         when SReadSample =>
              nstate <= SDataTrans;

              BRAM_EN <= '1';
              data_shreg_ld <= '1';
              rd_cntr_en <= '1';

         when SDataTrans =>
              nstate <= SDataTrans;
              
              READY <= '1';
              data_shreg_en <= '1';
              if (bit_cntr_last = '1') and (sck_re = '1') then
                 nstate <= SWriteSample;
              end if;

         when SWriteSample =>
              nstate <= SInit;

              -- Write received sample to BRAM, increment wr_counter
              BRAM_EN <= '1';
              BRAM_WE <= '1';
              bram_addr_sel <= '1';
              wr_cntr_en <= '1';

         when others => null;
      end case;
   end process;
   
end main;

