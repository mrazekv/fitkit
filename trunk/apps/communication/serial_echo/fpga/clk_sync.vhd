-- clk_sync.vhd: valid synchronization accross clock domains
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
-- This software is provided ``as is'', and any express or implied
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

entity clk_sync is
   port ( 
      RESET     : in std_logic; 
      CLK_FAST  : in std_logic; 
      CLK_SLOW  : in std_logic; 

      FAST_IN  : in std_logic;
      SLOW_OUT : out std_logic
   ); 
end clk_sync;


architecture behavioral of clk_sync is

   signal val_rst : std_logic;
   signal val_set : std_logic;

begin

   process (RESET, CLK_SLOW)
   begin
      if (RESET='1') then
         val_rst <= '0';
         SLOW_OUT <= '0';
      elsif (CLK_SLOW='1') and (CLK_SLOW'event) then
         val_rst <= '0';
         SLOW_OUT <= '0';
         if (val_set = '1') then
            SLOW_OUT <= '1';
            val_rst <= '1';
         end if;
      end if;
   end process;

   process (RESET, CLK_FAST)
   begin
      if (RESET='1') then
          val_set <= '0';
      elsif (CLK_FAST='1') and (CLK_FAST'event) then
          if (FAST_IN = '1') then
             val_set <= '1';
          elsif (val_rst = '1') then
             val_set <= '0';
          end if;
      end if;
   end process;
   
end architecture;

