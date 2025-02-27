-- video player
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

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

architecture arch_ide_test of tlv_ide_ifc is

   signal ide_bus        : std_logic_vector(25 downto 0) := (others => 'Z');
	 
	 component IDE_port
	    port (
	       IDE : inout std_logic_vector(25 downto 0) := (others => 'Z');
	       X   : inout std_logic_vector(45 downto 0) := (others => 'Z')
	    );
	 end component;

   for ide_p: IDE_port use entity work.IDE_port(small);

   component IDE_ctrl
      generic (
         CLK_PERIOD  : integer
      );
   port (
      -- Reset a synchronizace
      IDE     : inout std_logic_vector(25 downto 0) := (others => 'Z');
      CLK     : in std_logic; -- Hodiny radice (min. 4x vetsi, nez hodiny SPI)
      RST			:	in std_logic; -- RESET radice
      INTRQ	  :	out std_logic; -- preruseni

      -- Interni rozhrani
      DI      : out std_logic;   -- Serial data out (MOSI)
      DI_REQ  : in std_logic;   -- DI request
      DO      : in std_logic;   -- Serial data in (MISO)
      DO_VLD  : in std_logic;   -- DO valid
      CS      : in std_logic    -- internal SPI active
   );
   end component;

 begin

   ide_p: IDE_port
      port map(
         IDE  => ide_bus,
         X  => X(45 downto 0)
      );

   ide_c: IDE_ctrl
      generic map (
         CLK_PERIOD  => 20 --100 MHz - 10 ns 50 MHz = 20 ns
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,
	       RST			=> RESET,
	       INTRQ		=> open,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
         IDE  => ide_bus
      );

end arch_ide_test;
