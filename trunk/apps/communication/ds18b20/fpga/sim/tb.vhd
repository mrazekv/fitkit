-- tb.vhd : FPGA top level testbench
-- Copyright (C) 2007 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek
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

   signal smclk : std_logic := '1';
   signal smclkd2  : std_logic := '1';

   signal ledf  : std_logic;
   signal p3m   : std_logic_vector(7 downto 0) := (others=>'Z');   
   signal afbus : std_logic_vector(11 downto 0) :=(others =>'Z');
   signal xbus  : std_logic_vector(45 downto 0) :=(others =>'Z');
   signal rdbus : std_logic_vector(7 downto 0) :=(others =>'Z');
   signal ldbus : std_logic_vector(7 downto 0) :=(others =>'Z');

   signal ispi_clk : std_logic:='1';
   signal ispi_cs  : std_logic:='1';
   signal ispi_di  : std_logic:='0';    
   signal ispi_do  : std_logic:='0';
   
   procedure xsend_data_4(
      signal spi_do : out std_logic; 
      constant data : std_logic_vector(3 downto 0)
   ) is
   begin
      for i in 3 downto 0 loop
          spi_do <= data(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;
   end;

   procedure send_data_16(
      signal spi_cs : out std_logic;
      signal spi_do : out std_logic; 
      constant data : std_logic_vector(15 downto 0)
   ) is
   begin
      wait until ispi_clk='0'; 
      spi_cs<='0';
	 
      xsend_data_4(spi_do,"0001"); --FPGA id 
      xsend_data_4(spi_do,"0001"); --WE

      xsend_data_4(spi_do,"1111"); --ADDR
      xsend_data_4(spi_do,"0000"); 

      for i in 15 downto 0 loop
          spi_do <= data(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;

      spi_cs <= '1';
   end; 

   procedure read_data_16(
      signal spi_cs : out std_logic;
      signal spi_do : out std_logic
   ) is
   begin
      wait until ispi_clk='0'; 
      spi_cs<='0';
	 
      xsend_data_4(spi_do,"0001"); --FPGA id 
      xsend_data_4(spi_do,"0010"); --RE

      xsend_data_4(spi_do,"1010"); --ADDR
      xsend_data_4(spi_do,"0000"); 

      for i in 15 downto 0 loop
          --spi_do <= data(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;

      spi_cs <= '1';
   end; 

begin

   uut: entity work.fpga
      port map(
         SMCLK    => smclk,
         ACLK     => '0',
         FCLK     => '0',
         LEDF     => ledf,
   
         SPI_CLK  => ispi_clk,
         SPI_CS   => '0',
         SPI_FPGA_CS => ispi_cs,
         SPI_DI   => ispi_di,
         SPI_DO   => ispi_do,
   
         KIN      => open,
         KOUT     => (others => 'Z'),
   
         LE       => open,
         LRW      => open,
         LRS      => open,
         LD       => ldbus,

         RA       => open,
         RD       => rdbus,
         RDQM     => open,
         RCS      => open,
         RRAS     => open,
         RCAS     => open,
         RWE      => open,
         RCKE     => open,
         RCLK     => open,

         P3M      => p3m,
         AFBUS    => afbus,
   	   X        => xbus
   );

   -- Clock generator 7.4MHz
   clkgen : process
   begin
      smclk<= '1';
      wait for 67.5 ns;
      smclk <= '0';
      wait for 67.5 ns;
   end process;

   -- SPI clock generator (SMCLK/4)
   spiclkgen: process(smclk)
   begin
      if (smclk'event) and (smclk='1')then
         smclkd2 <= not smclkd2;
         if (smclkd2 = '1') then
            ispi_clk <= not ispi_clk;
         end if;
      end if;
   end process;

   -- Reset generator
   resetgen: process
   begin
      p3m(0)<= '1';
      wait for 1000 ns;
      p3m(0) <= '0';
      wait; 
   end process;


-- Test bench circuit
write:process
  begin
    ispi_cs<='1';
    wait until p3m(0)='0';
    wait for 100 ns;

    send_data_16(ispi_cs,ispi_do,"00000000" & "00000000"); -- reset

    wait for 1000 us;
    send_data_16(ispi_cs,ispi_do,"00000001" & "11001100"); -- write cmd CC

    wait for 1000 us;
    send_data_16(ispi_cs,ispi_do,"00000001" & "10110100"); -- write cmd B4

    wait for 1000 us;
    send_data_16(ispi_cs,ispi_do,"00000010" & "00000000"); -- read cmd


    wait for 1000 us;
    read_data_16(ispi_cs,ispi_do); -- read
    wait;
end process;

end architecture behavioral;
