-- keyboard_ctrl.vhd : Keyboard controller
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Tomas Martinek <martinto AT fit.vutbr.cz>
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

entity keyboard_controller_high is
   generic(
        READ_INTERVAL : integer := (2**14)-1
   );
   port(
      -- Hodiny a Reset
      CLK        : in std_logic;
      RST        : in std_logic;

      -- Stisknute klavesy
      DATA_OUT   : out std_logic_vector(15 downto 0);
      
      -- Signaly klavesnice 
      KB_KIN     : out std_logic_vector(3 downto 0);
      KB_KOUT    : in  std_logic_vector(3 downto 0)
   );
end keyboard_controller_high;

architecture behavioral of keyboard_controller_high is

   signal key_data_out : std_logic_vector(15 downto 0);
   signal key_data_vld : std_logic;

begin

-- keyboard controller instantion 
key_ctrl_u : entity work.keyboard_controller
generic map(
   READ_INTERVAL => READ_INTERVAL
)
port map(
   CLK        => CLK,
   RST        => RST,

   -- Stisknute klavesy
   DATA_OUT   => key_data_out,
   DATA_VLD   => key_data_vld,
      
   -- Signaly klavesnice 
   KB_KIN     => KB_KIN,
   KB_KOUT    => KB_KOUT
);

DATA_OUT(0)  <= key_data_vld and key_data_out(7);
DATA_OUT(1)  <= key_data_vld and key_data_out(0);
DATA_OUT(2)  <= key_data_vld and key_data_out(4);
DATA_OUT(3)  <= key_data_vld and key_data_out(8);
DATA_OUT(4)  <= key_data_vld and key_data_out(1);
DATA_OUT(5)  <= key_data_vld and key_data_out(5);
DATA_OUT(6)  <= key_data_vld and key_data_out(9);
DATA_OUT(7)  <= key_data_vld and key_data_out(2);
DATA_OUT(8)  <= key_data_vld and key_data_out(6);
DATA_OUT(9)  <= key_data_vld and key_data_out(10);
DATA_OUT(10) <= key_data_vld and key_data_out(12);
DATA_OUT(11) <= key_data_vld and key_data_out(13);
DATA_OUT(12) <= key_data_vld and key_data_out(14);
DATA_OUT(13) <= key_data_vld and key_data_out(15);
DATA_OUT(14) <= key_data_vld and key_data_out(3);
DATA_OUT(15) <= key_data_vld and key_data_out(11);

end behavioral;

