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

   procedure write_data(
      signal spi_cs : out std_logic;
      signal spi_do : out std_logic; 
      constant addr : natural;
      constant data : natural
   ) is
   variable vaddr : std_logic_vector(15 downto 0);
   variable vdata : std_logic_vector(7 downto 0);
   begin
      wait until ispi_clk='0'; 
      spi_cs<='0';
	 
      xsend_data_4(spi_do,"0001"); --FPGA id 
      xsend_data_4(spi_do,"0001"); --WE

      vaddr := conv_std_logic_vector(addr + 16#8000#, 16);
      for i in 15 downto 0 loop
          spi_do <= vaddr(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;

      vdata := conv_std_logic_vector(data, 8);
      for i in 7 downto 0 loop
          spi_do <= vdata(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;

      spi_cs <= '1';
   end; 

   procedure write_len(
      signal spi_cs : out std_logic;
      signal spi_do : out std_logic;
      constant datalen : natural
   ) is
   variable vdatalen : std_logic_vector(15 downto 0);
   begin
      wait until ispi_clk='0'; 
      spi_cs<='0';

      xsend_data_4(spi_do,"0001"); --FPGA id 
      xsend_data_4(spi_do,"0001"); --WE

      xsend_data_4(spi_do,"1100"); --ADDR 0xC0
      xsend_data_4(spi_do,"0000"); 

      vdatalen := conv_std_logic_vector(datalen, 16);
      for i in 15 downto 0 loop
          spi_do <= vdatalen(i);
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
      wait for 135 ns;
      smclk <= '0';
      wait for 135 ns;
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
    wait for 5 us;

--   write_data(ispi_cs,ispi_do, 16#0000#, 16#11#); 
--    write_data(ispi_cs,ispi_do, 16#0001#, 16#22#); 
--    write_data(ispi_cs,ispi_do, 16#0002#, 16#33#); 
--    write_data(ispi_cs,ispi_do, 16#0003#, 16#44#); 
--    write_data(ispi_cs,ispi_do, 16#0004#, 16#55#); 
--    write_data(ispi_cs,ispi_do, 16#0005#, 16#66#); 
--
--    write_len(ispi_cs,ispi_do, 6); 
--    wait;

    --ZAPIS ETHERNETOVEHO RAMCE DO BRAM POMOCI SPI
    write_data(ispi_cs,ispi_do, 16#0000#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0001#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0002#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0003#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0004#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0005#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0006#, 16#55#);
    write_data(ispi_cs,ispi_do, 16#0007#, 16#D5#);
    
    write_data(ispi_cs,ispi_do, 16#0008#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0009#, 16#10#);
    write_data(ispi_cs,ispi_do, 16#000A#, 16#A4#);
    write_data(ispi_cs,ispi_do, 16#000B#, 16#7B#);
    write_data(ispi_cs,ispi_do, 16#000C#, 16#EA#);
    write_data(ispi_cs,ispi_do, 16#000D#, 16#80#);
    write_data(ispi_cs,ispi_do, 16#000E#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#000F#, 16#12#);
    write_data(ispi_cs,ispi_do, 16#0010#, 16#34#);
    write_data(ispi_cs,ispi_do, 16#0011#, 16#56#);
    write_data(ispi_cs,ispi_do, 16#0012#, 16#78#);
    write_data(ispi_cs,ispi_do, 16#0013#, 16#90#);
    write_data(ispi_cs,ispi_do, 16#0014#, 16#08#);
    write_data(ispi_cs,ispi_do, 16#0015#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0016#, 16#45#);
    write_data(ispi_cs,ispi_do, 16#0017#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0018#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0019#, 16#2E#);
    write_data(ispi_cs,ispi_do, 16#001A#, 16#B3#);
    write_data(ispi_cs,ispi_do, 16#001B#, 16#FE#);
    write_data(ispi_cs,ispi_do, 16#001C#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#001D#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#001E#, 16#80#);
    write_data(ispi_cs,ispi_do, 16#001F#, 16#11#);
    write_data(ispi_cs,ispi_do, 16#0020#, 16#05#);
    write_data(ispi_cs,ispi_do, 16#0021#, 16#40#);
    write_data(ispi_cs,ispi_do, 16#0022#, 16#C0#);
    write_data(ispi_cs,ispi_do, 16#0023#, 16#A8#);
    write_data(ispi_cs,ispi_do, 16#0024#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0025#, 16#2C#);
    write_data(ispi_cs,ispi_do, 16#0026#, 16#C0#);
    write_data(ispi_cs,ispi_do, 16#0027#, 16#A8#);
    write_data(ispi_cs,ispi_do, 16#0028#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0029#, 16#04#);
    write_data(ispi_cs,ispi_do, 16#002A#, 16#04#);
    write_data(ispi_cs,ispi_do, 16#002B#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#002C#, 16#04#);
    write_data(ispi_cs,ispi_do, 16#002D#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#002E#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#002F#, 16#1A#);
    write_data(ispi_cs,ispi_do, 16#0030#, 16#2D#);
    write_data(ispi_cs,ispi_do, 16#0031#, 16#E8#);
    write_data(ispi_cs,ispi_do, 16#0032#, 16#00#);
    write_data(ispi_cs,ispi_do, 16#0033#, 16#01#);
    write_data(ispi_cs,ispi_do, 16#0034#, 16#02#);
    write_data(ispi_cs,ispi_do, 16#0035#, 16#03#);
    write_data(ispi_cs,ispi_do, 16#0036#, 16#04#);
    write_data(ispi_cs,ispi_do, 16#0037#, 16#05#);
    write_data(ispi_cs,ispi_do, 16#0038#, 16#06#);
    write_data(ispi_cs,ispi_do, 16#0039#, 16#07#);
    write_data(ispi_cs,ispi_do, 16#003A#, 16#08#);
    write_data(ispi_cs,ispi_do, 16#003B#, 16#09#);
    write_data(ispi_cs,ispi_do, 16#003C#, 16#0A#);
    write_data(ispi_cs,ispi_do, 16#003D#, 16#0B#);
    write_data(ispi_cs,ispi_do, 16#003E#, 16#0C#);
    write_data(ispi_cs,ispi_do, 16#003F#, 16#0D#);
    write_data(ispi_cs,ispi_do, 16#0040#, 16#0E#);
    write_data(ispi_cs,ispi_do, 16#0041#, 16#0F#);
    write_data(ispi_cs,ispi_do, 16#0042#, 16#10#);
    write_data(ispi_cs,ispi_do, 16#0043#, 16#11#);
    
    -- CRC write_data(ispi_cs,ispi_do, 16#0044#, 16#B3#);
    -- CRC write_data(ispi_cs,ispi_do, 16#0045#, 16#31#);
    -- CRC write_data(ispi_cs,ispi_do, 16#0046#, 16#88#);
    -- CRC write_data(ispi_cs,ispi_do, 16#0047#, 16#1B#);
    
    --ZAPIS DELKY PAKETU DO REGISTRU POMOCI SPI    
    write_len(ispi_cs,ispi_do, 72-4);
    
    wait;
end process;

end architecture behavioral;
