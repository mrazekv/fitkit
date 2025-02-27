-- serial_clkgen.vhd : Baudrate generator
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
use IEEE.std_logic_1164.ALL;
use IEEE.std_logic_ARITH.ALL;
use IEEE.std_logic_UNSIGNED.ALL;
use work.serial_cfg.ALL;

entity serial_gen is
   generic (
      SPEED     : serial_speed := s921600Bd; -- Serial speed
      FREQMULT  : positive := 1 -- divisor 8/FREQMULT is used for 921 600 Bd (SMCLK/8 = 7.3728MHz/8)
   );
   port (
      CLK         : in  std_logic;
      RESET       : in  std_logic;
      EN          : in  std_logic;

      OUTPUT      : out std_logic
   );
end serial_gen;

architecture behavioral of serial_gen is
   --maximal value of counter
   constant CNT_MAX  : integer := clkdiv(SPEED, FREQMULT);
   --minimal number of bits to achieve CNT_MAX
   constant CNT_BITS : integer := log2(CNT_MAX-1);

   signal cnt_reg : std_logic_vector(CNT_BITS-1 downto 0) := (others => '0');
   signal cnt_cmp : std_logic;
begin

   -- Counter
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         cnt_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (EN = '1') then 
            if (cnt_cmp = '1') then
               cnt_reg <= (others => '0');
            else
               cnt_reg <= cnt_reg + 1;
            end if;
         end if;
      end if;
   end process; 

   --Comparator
   cnt_cmp <= '1' when (CNT_MAX = 1) or (cnt_reg = conv_std_logic_vector(CNT_MAX-1, CNT_BITS)) else '0';

   --Output enable signal
   OUTPUT <= cnt_cmp and EN;
 
end behavioral;
