-- spi_ctrl.vhd : SPI Controller
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

-- SYNTH-ISE: slices=4, slicesFF=5, 4luts=7
entity SPI_ctrl is
   port (
      -- Reset a synchronizace
      RST     : in   std_logic;
      CLK     : in   std_logic;

      -- Rozhrani SPI (z master MCU)
      SPI_CLK : in   std_logic;
      SPI_CS  : in   std_logic;
      SPI_MOSI: in   std_logic;
      SPI_MISO: out  std_logic;

      -- Interni rozhrani
      DI      : in   std_logic;   -- Serial data in (MISO)
      DI_REQ  : out  std_logic;   -- DI request
      DO      : out  std_logic;   -- Serial data out (MOSI)
      DO_VLD  : out  std_logic;   -- DO valid
      CS      : out  std_logic    -- internal SPI active
   );
end SPI_ctrl;



architecture Arch_SPI_ctrl2 of SPI_ctrl is

type    FSMstate is (SInit, SRcv1, SRcv2, SRcv3, SActive, SNActive);
signal  pstate    : FSMstate; -- actual state
signal  nstate    : FSMstate; -- next state

signal  dovld     : std_logic;
signal  devsel    : std_logic;
signal  spi_clk_1 : std_logic;
signal  spi_clk_2 : std_logic;

begin

   -- FSM
   process (SPI_CS, CLK)
   begin
      if (SPI_CS = '1') then
         pstate <= SInit;
      elsif (CLK'event) and (CLK='1') then
         if (dovld = '1') then
            pstate <= nstate;
         end if;
      end if;
   end process;

   process (pstate, SPI_MOSI)
   begin
      nstate <= SInit;
      devsel <= '0';
      case pstate is
         when SInit =>
            nstate <= SNActive;
            if (SPI_MOSI = '0') then
               nstate <= SRcv1;
            end if;

         -- first bit received and was 1
         when SRcv1 =>
            nstate <= SNActive;
            if (SPI_MOSI = '0') then
               nstate <= SRcv2;
            end if;

        -- second bit received and was 1
        when SRcv2 =>
            nstate <= SNActive;
            if (SPI_MOSI = '0') then
               nstate <= SRcv3;
            end if;

        -- third bit received and was 1
        when SRcv3 =>
            nstate <= SNActive;
            if (SPI_MOSI = '1') then
               nstate <= SActive;
            end if;

        -- first four bits were 0001
        when SActive =>
            nstate <= SActive;
            devsel <= '1';

        when SNActive =>
            nstate <= SNActive;

     end case;
   end process;

   -- SPI_CLK edge detector
   process (RST, CLK)
   begin
      if (RST = '1') then
         spi_clk_1 <= '1'; --SPI_CLK inactive state is '1'
         spi_clk_2 <= '1';
      elsif (CLK'event) and (CLK = '1') then
         spi_clk_1 <= SPI_CLK;
         spi_clk_2 <= spi_clk_1;
      end if;
   end process;

   CS <= not SPI_CS when (devsel='1') else '0';

   -- signal DI must be valid during the whole SCK period,
   -- typically meet because SPI decoder contains data shift register
   SPI_MISO <= DI when (devsel='1') else 'Z';
   DO <= SPI_MOSI;

   dovld <= spi_clk_1 and (not spi_clk_2);   -- rising edge
   DI_REQ <= (not spi_clk_1) and  spi_clk_2;  -- falling edge
   DO_VLD <= dovld;

end Arch_SPI_ctrl2;

