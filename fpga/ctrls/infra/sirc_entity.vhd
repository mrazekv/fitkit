-- sirc_entity.vhd: Sony SIRC code  entity
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Michal Růžek <xruzek01 AT stud.fit.vutbr.cz>
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

entity ir_SIRC is
   port ( 
      -- Reset a synchronizace
      RST     : in   std_logic;
      CLK     : in   std_logic;

      -- V/V signaly z IR modulu
      IR_IN : in  std_logic;
      IR_OUT : out std_logic;
      
      -- Vstup do dekoderu SIRC
      DATA_IN : in std_logic_vector(23 downto 0);
      WRITE_12_EN : in std_logic;
      WRITE_15_EN : in std_logic;
      WRITE_20_EN : in std_logic;
      BUSY : out std_logic;
      
      -- Vystup z dekoderu SIRC
      DATA_OUT : out  std_logic_vector(23 downto 0); 
      DATA_12_VLD : out  std_logic ;
      DATA_15_VLD : out  std_logic;
      DATA_20_VLD : out  std_logic
   );
end ir_SIRC;
