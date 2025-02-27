-- fifo36.vhd: 36bit FIFO (by RAMB), length=512 
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
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
use IEEE.STD_LOGIC_1164.all;
use IEEE.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- pragma translate_off 
library unisim;
use unisim.vcomponents.all;
-- pragma translate_on 

entity fifo36 is
   port (
      CLK       : in std_logic;
      RST       : in std_logic;
      
      DATA_IN   : in std_logic_vector(35 downto 0);
      DATA_WE   : in std_logic;

      DATA_OUT  : out std_logic_vector(35 downto 0);
      DATA_VLD  : out std_logic;
      FIFO_NEXT : in std_logic;
      
      FIFO_FREE : out std_logic
   );
end entity fifo36;

architecture a_fifo of fifo36 is

   component RAMB16_S36_S36
      generic (
       WRITE_MODE_A : string := "WRITE_FIRST";
       WRITE_MODE_B : string := "WRITE_FIRST";
       INIT_A  : bit_vector  := X"000000000";
       SRVAL_A : bit_vector  := X"000000000";
       INIT_B  : bit_vector  := X"000000000";
       SRVAL_B : bit_vector  := X"000000000"
      );
      port (
         DOA   : out std_logic_vector(31 downto 0);
         DOB   : out std_logic_vector(31 downto 0);
         DOPA  : out std_logic_vector(3 downto 0);
         DOPB  : out std_logic_vector(3 downto 0);
         ADDRA : in std_logic_vector(8 downto 0);
         ADDRB : in std_logic_vector(8 downto 0);
         CLKA  : in std_logic;
         CLKB  : in std_logic;
         DIA   : in std_logic_vector(31 downto 0);
         DIB   : in std_logic_vector(31 downto 0);
         DIPA  : in std_logic_vector(3 downto 0);
         DIPB  : in std_logic_vector(3 downto 0);
         ENA   : in std_logic;
         ENB   : in std_logic;
         SSRA  : in std_logic;
         SSRB  : in std_logic;
         WEA   : in std_logic;
         WEB   : in std_logic
      );
   end component;

   signal addr_w      : std_logic_vector(8 downto 0);
   signal addr_r      : std_logic_vector(8 downto 0);
   signal addr_r_prev : std_logic_vector(8 downto 0);
   signal we       : std_logic;
   signal re       : std_logic;
   signal step     : std_logic;
   signal step_reg : std_logic;
   signal nfull    : std_logic;

begin

   fifo_i: RAMB16_S36_S36
     port map (
         CLKA  => CLK,
         CLKB  => CLK,
         DOA   => open,
         DOB   => DATA_OUT(31 downto 0),
         DOPA  => open,
         DOPB  => DATA_OUT(35 downto 32),
         ADDRA => addr_w(8 downto 0),
         ADDRB => addr_r(8 downto 0),
         DIA   => DATA_IN(31 downto 0),
         DIB   => "00000000000000000000000000000000",
         DIPA  => DATA_IN(35 downto 32),
         DIPB  => "0000",
         ENA   => we,
         ENB   => re,
         SSRA  => '0',
         SSRB  => '0',
         WEA   => we,
         WEB   => '0');
   
   step  <= '1' when addr_r /= addr_w else '0';
   nfull <= '1' when addr_r_prev /= addr_w else '0';
   we    <= DATA_WE and nfull;
   re    <= FIFO_NEXT and step and step_reg and (DATA_WE nand nfull);
   FIFO_FREE <= nfull;

   rw_proc: process(RST, CLK)
   begin
      if RST = '1' then
         addr_w <= (others => '0');
         addr_r <= (others => '0');
         addr_r_prev <= (others => '1');
         DATA_VLD <= '0';
         step_reg <= '0';
      elsif CLK'event and CLK = '1' then
         DATA_VLD <= re;
         step_reg <= step;
         if we = '1' then
            addr_w <= addr_w + 1;
         end if;
         if re = '1' then
            addr_r_prev <= addr_r;
            addr_r <= addr_r + 1;
         end if;
      end if;
   end process;

end architecture a_fifo;
