-- lcd_raw.vhd : Simple LCD wrapper
-- Copyright (C) 2009 Brno University of Technology,
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
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;

entity lcd_raw_controller is
   port (
      RST      : in std_logic;
      CLK      : in std_logic;

      -- interni rozhrani
      DATA_IN  : in std_logic_vector(15 downto 0);
      WRITE_EN : in std_logic;

      --- rozhrani LCD displeje
      DISPLAY_RS   : out   std_logic;
      DISPLAY_DATA : inout std_logic_vector(7 downto 0);
      DISPLAY_RW   : out   std_logic;
      DISPLAY_EN   : out   std_logic
  );
end entity;

architecture base of lcd_raw_controller is
   signal data_reg : std_logic_vector(9 downto 0);
begin
   DISPLAY_RW <= '0';
   DISPLAY_EN <= data_reg(9);
   DISPLAY_RS <= data_reg(8);
   DISPLAY_DATA <= data_reg(7 downto 0);

   -- data register
   process(CLK, RST)
   begin
      if (RST = '1') then
         data_reg <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (WRITE_EN='1') then
            data_reg <= DATA_IN(9 downto 0);
         end if;
      end if;
   end process;
end architecture;

