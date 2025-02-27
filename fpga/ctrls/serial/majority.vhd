-- majority.vhd : Majority of five inputs
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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- pragma translate_off
library unisim;
use unisim.vcomponents.ALL;
-- pragma translate_on

entity majority_five is
   port (
      D : in  std_logic_vector(4 downto 0);
      Q : out std_logic
   );
end majority_five;


architecture struct of majority_five is

   component ROM32x1
      generic (
         INIT : bit_vector := X"FEE8E880"
      );
      port (
         O  : out STD_ULOGIC;
         A0 : in STD_ULOGIC;
         A1 : in STD_ULOGIC;
         A2 : in STD_ULOGIC;
         A3 : in STD_ULOGIC;
         A4 : in STD_ULOGIC
      );
   end component;

   attribute INIT : bit_vector;
   attribute INIT of bit_majority : label is X"FEE8E880";

begin
   -- Truth Table 
   --  I0 I1 I2 I3 I4   O
   --  0  0  0  0  0    0    (bit 0) 
   --  0  0  0  0  1    0
   --  0  0  0  1  0    0
   --  0  0  0  1  1    0
   --  0  0  1  0  0    0
   --  0  0  1  0  1    0
   --  0  0  1  1  0    0 
   --  0  0  1  1  1    1    (bit 7)
   --  ==================
   --  0  1  0  0  0    0    (bit 0) 
   --  0  1  0  0  1    0
   --  0  1  0  1  0    0
   --  0  1  0  1  1    1
   --  0  1  1  0  0    0
   --  0  1  1  0  1    1
   --  0  1  1  1  0    1 
   --  0  1  1  1  1    1    (bit 7)
   --  ==================
   --  1  0  0  0  0    0    (bit 0) 
   --  1  0  0  0  1    0
   --  1  0  0  1  0    0
   --  1  0  0  1  1    1
   --  1  0  1  0  0    0
   --  1  0  1  0  1    1
   --  1  0  1  1  0    1 
   --  1  0  1  1  1    1    (bit 7)
   --  ==================
   --  1  1  0  0  0    0    (bit 0) 
   --  1  1  0  0  1    1
   --  1  1  0  1  0    1
   --  1  1  0  1  1    1
   --  1  1  1  0  0    1
   --  1  1  1  0  1    1
   --  1  1  1  1  0    1 
   --  1  1  1  1  1    1    (bit 7)
   
   bit_majority : ROM32X1
      generic map (
         INIT => X"FEE8E880"
      )
      port map (
         O =>  Q,
         A0 => D(0),
         A1 => D(1),
         A2 => D(2),
         A3 => D(3),
         A4 => D(4)
      );

end struct;

