-- testbench.vhd: Testbench of top level entity
-- Copyright (C) 2007 Brno University of Technology,
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

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_textio.all;
use ieee.numeric_std.all;
use std.textio.all;


-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity testbench is
end entity testbench;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of testbench is

signal reset : std_logic := '1';
signal smclk : std_logic := '1';

signal din : std_logic_vector(7 downto 0);
signal dout : std_logic_vector(7 downto 0);
signal dout_vld : std_logic;
signal we : std_logic;
signal busy : std_logic;
signal rxd : std_logic;
signal txd : std_logic;
signal shreg : std_logic_vector(10 downto 0);

begin

uut: entity work.serial_transceiver
port map(
   RESET    => reset,
   CLK      => smclk,

   DATA_OUT => dout,
   DATA_VLD => dout_vld,

   DATA_IN  => din,
   WRITE_EN => we,

   BUSY     => busy,
   ERR      => open,
   ERR_RST  => '0',

   RXD      => rxd,
   TXD      => txd,
   RTS      => '1',
   CTS      => open
);   

--Clock generator
clkgen : process
begin
   smclk<= '1';
   wait for 135 ns;
   smclk <= '0';
   wait for 135 ns;
end process;

--Reset
res : process
begin
    reset <= '1';
    wait for 1000 ns;
    reset <= '0';
    wait; 
end process;

process (reset, smclk)
begin
   if (reset = '1') then
      shreg <= (others => '1');
   elsif (smclk'event) and (smclk = '1') then
      shreg <= shreg(9 downto 0) & txd;
   end if;
end process;

rxd <= shreg(10);

tb : process
begin	
   din <= (others => '0');
   we <= '0';

   wait until reset='0';
   wait for 500 ns;

   -- 1. zapis
   wait until smclk='0';
   din <= "11001011";
   we <= '1'; 
   wait until smclk='1';
   wait until smclk='0';
   we <= '0';

   wait until busy='0';
   -- 2. zapis
   wait until smclk='0';
   din <= "01100011";
   we <= '1'; 
   wait until smclk='1';
   wait until smclk='0';
   we <= '0';
   -----------

   wait;
end process;

-- ----------------------------------------------------------------------------
end architecture behavioral;
