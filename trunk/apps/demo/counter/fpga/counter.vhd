-- counter_ncs.vhd : Generic counter
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity counter is
   generic (
      DATA_WIDTH : integer := 3
   );
   port( 
      CLK      : in std_logic; -- synchronizace
      RST      : in std_logic; -- asynchronnni reset
      EN       : in std_logic; -- povoleni citani

      DATA_OUT  : out STD_LOGIC_VECTOR (DATA_WIDTH-1 downto 0) -- vystup
   ); 
end counter;


architecture arch_counter of counter is

signal cnt : std_logic_vector(DATA_WIDTH-1 downto 0);

begin
   DATA_OUT <= cnt;
   
   -- Counter
   process(CLK, RST) 
   begin 
      if (RST = '1') then
         cnt <= (others => '0');
      elsif (CLK'event) and (CLK='1') then 
         if (EN = '1') then
            cnt <= cnt + 1;
         end if;
      end if;  
   end process;

end arch_counter;

