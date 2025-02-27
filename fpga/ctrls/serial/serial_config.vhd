-- serial_config.vhd: Serial constants and types
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
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

package serial_cfg is

   subtype serial_databit is natural range 5 to 9;
   
   type serial_speed is (
      s1200Bd, s2400Bd, s4800Bd, s9600Bd, s19200Bd, s38400Bd, 
      s57600Bd, s115200Bd, s230400Bd, s460800Bd, s921600Bd
   ); 
   
   type serial_parity is (
      sParityNone, sParityEven, sParityOdd
   );
   
   subtype serial_stopbit is natural range 1 to 2;

   function clkdiv(constant speed : serial_speed; constant freqmult: positive) return integer;
   function log2(constant a : integer) return integer;

end serial_cfg;


package body serial_cfg is

   function clkdiv(constant speed : serial_speed; constant freqmult: positive) return integer is
      variable div : integer;
   begin
      div := 8 / freqmult;
      if speed = s921600Bd then return div; end if;
      div := div * 2;
      if speed = s460800Bd then return div; end if;
      div := div * 2;
      if speed = s230400Bd then return div; end if;
      div := div * 2;
      if speed = s115200Bd then return div; end if;
      div := div * 2;
      if speed = s57600Bd then return div; end if;
      div := integer(real(div) * 1.5);
      if speed = s38400Bd then return div; end if;
      div := div * 2;
      if speed = s19200Bd then return div; end if;
      div := div * 2;
      if speed = s9600Bd then return div; end if;
      div := div * 2;
      if speed = s4800Bd then return div; end if;
      div := div * 2;
      if speed = s2400Bd then return div; end if;
      div := div * 2;
      -- Default is 1200 Bd
      return div;
   end;

   function log2(constant a : integer) return integer is
      variable bits, b : integer;
   begin
      bits := 0;

      b := 1;
      while (b <= a) loop
        b := b * 2;
        bits := bits + 1;
      end loop;

      return bits;
   end function;
end serial_cfg;
