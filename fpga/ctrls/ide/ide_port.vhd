-- ide_port.vhd : IDE Port
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Stanislav Sigmund <xsigmu02 AT stud.fit.vutbr.cz>
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

entity IDE_port is
   port (
      IDE : inout std_logic_vector(25 downto 0) := (others => 'Z');
      X   : inout std_logic_vector(45 downto 0) := (others => 'Z')
   );
end IDE_port;

architecture basic of IDE_port is
begin
idec: for i in 0 to 7 generate
 X(16-2*i) <= IDE(i);
 X(3+2*i) <= IDE(i+8);
end generate;
X(0) <= IDE(16); -- RESET
X(22) <= IDE(17); -- IOW
X(24) <= IDE(18); -- IOR

X(37) <= IDE(23);X(36) <= IDE(22);
X(35) <= IDE(21);X(32) <= IDE(20);
X(34) <= IDE(19); -- CS1 CS0 DA2 DA1 DA0

X(20) <= IDE(24);X(28) <= IDE(25); -- DMARQ DMAACK

X(1) <= '0';X(18) <= '0';X(21) <= '0';X(23) <= '0';
X(25) <= '0';X(29) <= '0';X(39) <= '0';

-- nezapojene vyvody
X(30) <= 'Z'; -- INTQ
-- DASP- DIAG- IOCS16- CSEL
X(38) <= 'Z';X(33) <= 'Z';X(31) <= 'Z';X(27) <= 'Z';
-- IORDY keypin_20, nic nic
X(26) <= 'Z';X(19) <= 'Z';X(40) <= 'Z';X(41) <= 'Z';
-- nic nic nic nic
X(42) <= 'Z';X(43) <= 'Z';X(44) <= 'Z';X(45) <= 'Z';
end basic;

architecture small of IDE_port is
begin
idec: for i in 0 to 7 generate
 X(14-2*i) <= IDE(i);
 X(1+2*i) <= IDE(i+8);
end generate;
X(16) <= IDE(16); -- RESET
X(20) <= IDE(17); -- IOW
X(22) <= IDE(18); -- IOR

X(17) <= IDE(23);X(19) <= IDE(22);
X(21) <= IDE(21);X(24) <= IDE(20);
X(23) <= IDE(19); -- CS1 CS0 DA2 DA1 DA0

IDE(24) <= X(18);X(26) <= IDE(25); -- DMARQ DMAACK

-- nezapojene vyvody
X(25) <= 'Z'; -- volne misto
inz: for i in 27 to 45 generate
 X(i) <=  'Z';
end generate;
end small;

architecture rotate of IDE_port is
begin
idec: for i in 0 to 7 generate
 X(23+2*i) <= IDE(i);
 X(36-2*i) <= IDE(i+8);
end generate;
X(39) <= IDE(16); -- RESET
X(17) <= IDE(17); -- IOW
X(15) <= IDE(18); -- IOR

X(2) <= IDE(23);X(3) <= IDE(22);
X(4) <= IDE(21);X(7) <= IDE(20);
X(5) <= IDE(19); -- CS1 CS0 DA2 DA1 DA0

X(19) <= IDE(24);X(11) <= IDE(25); -- DMARQ DMAACK

X(38) <= '0';X(21) <= '0';X(18) <= '0';X(16) <= '0';
X(14) <= '0';X(10) <= '0';-- X(0) <= '0';

-- nezapojene vyvody
X(9) <= 'Z'; -- INTQ
-- DASP- DIAG- IOCS16- CSEL
X(1) <= 'Z';X(6) <= 'Z';X(8) <= 'Z';X(12) <= 'Z';
-- IORDY keypin_20, nic nic
X(13) <= 'Z';X(20) <= 'Z';X(40) <= 'Z';X(41) <= 'Z';
-- nic nic nic nic
X(42) <= 'Z';X(43) <= 'Z';X(44) <= 'Z';X(45) <= 'Z';
end rotate;
