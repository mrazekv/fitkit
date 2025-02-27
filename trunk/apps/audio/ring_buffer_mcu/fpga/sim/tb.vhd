-- tb.vhd : FPGA top level testbench
-- Copyright (C) 2009 Brno University of Technology,
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

   signal smclk   : std_logic := '1';
   signal smclkd2 : std_logic := '1';
   signal codec_bclk : std_logic := '0';
   signal codec_lrc : std_logic := '0';
   signal codec_dout : std_logic := '0';
   signal codec_din : std_logic;
   signal codec_word : std_logic_vector(31 downto 0);
   signal spi_data : std_logic_vector(23 downto 0);

   signal ledf  : std_logic;
   signal p3m   : std_logic_vector(7 downto 0) := (others=>'Z');   
   signal afbus : std_logic_vector(11 downto 0) :=(others =>'Z');
   signal xbus  : std_logic_vector(45 downto 0) :=(others =>'Z');
   signal rdbus : std_logic_vector(7 downto 0) :=(others =>'Z');
   signal ldbus : std_logic_vector(7 downto 0) :=(others =>'Z');

   signal ispi_clk : std_logic:='1';
   signal ispi_cs  : std_logic:='1';
   signal ispi_di  : std_logic:='Z';    
   signal ispi_do  : std_logic:='1';

   signal aspi_clk : std_logic:='1';
   signal aspi_data : std_logic_vector(31 downto 0);
   signal aspi_idata : std_logic_vector(31 downto 0);
   signal sample_valid : std_logic;
   signal aspi_di  : std_logic:='Z';    
   signal aspi_do  : std_logic:='1';
   
-- converts a std_logic_vector into a hex string.
function hstr(slv: std_logic_vector) return string is
   variable hexlen: integer;
   variable longslv : std_logic_vector(67 downto 0) := (others => '0');
   variable hex : string(1 to 16);
   variable fourbit : std_logic_vector(3 downto 0);
begin
   hexlen := (slv'left+1)/4;
   if (slv'left+1) mod 4 /= 0 then
   hexlen := hexlen + 1;
   end if;
   longslv(slv'left downto 0) := slv;
   for i in (hexlen - 1) downto 0 loop
      fourbit := longslv(((i*4)+3) downto (i*4));
      case fourbit is
         when "0000" => hex(hexlen - i) := '0';
         when "0001" => hex(hexlen - i) := '1';
         when "0010" => hex(hexlen - i) := '2';
         when "0011" => hex(hexlen - i) := '3';
         when "0100" => hex(hexlen - i) := '4';
         when "0101" => hex(hexlen - i) := '5';
         when "0110" => hex(hexlen - i) := '6';
         when "0111" => hex(hexlen - i) := '7';
         when "1000" => hex(hexlen - i) := '8';
         when "1001" => hex(hexlen - i) := '9';
         when "1010" => hex(hexlen - i) := 'A';
         when "1011" => hex(hexlen - i) := 'B';
         when "1100" => hex(hexlen - i) := 'C';
         when "1101" => hex(hexlen - i) := 'D';
         when "1110" => hex(hexlen - i) := 'E';
         when "1111" => hex(hexlen - i) := 'F';
         when "ZZZZ" => hex(hexlen - i) := 'z';
         when "UUUU" => hex(hexlen - i) := 'u';
         when "XXXX" => hex(hexlen - i) := 'x';
         when others => hex(hexlen - i) := '?';
      end case;
   end loop;
   return hex(1 to hexlen);
end hstr; 
   
begin
   --Unit under test
   uut: entity work.fpga
      port map(
         SMCLK    => smclk,
         ACLK     => '0',
         FCLK     => '0',
         LEDF     => ledf,
   
         SPI_CLK  => ispi_clk,
         SPI_CS   => '1',
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

   -- Clock generator (SMCLK) 7.4MHz
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
      p3m(0) <= '1';
      wait for 1000 ns;
      p3m(0) <= '0';
      wait; 
   end process;

   xbus(1) <= codec_bclk;
   xbus(3) <= codec_lrc;
   xbus(2) <= 'Z';
   --codec_din <= xbus(2);
   xbus(5) <= codec_lrc;
   xbus(4) <= codec_dout;

   -- codec output (BCLK) 12MHz
   codec: process
   begin
    codec_bclk <= '0';
    codec_lrc <= '0';
    codec_dout <= '0';
    codec_word(31 downto 16) <= "1001000011110001"; --90F1
    codec_word(15 downto 0) <= (others => '0');
    wait for 1 ns;
    while (true) loop
      codec_bclk <= '0';
      codec_lrc <= '1';
      wait for 41.6 ns;
      codec_bclk <= '1';
      wait for 41.6 ns;
      codec_lrc <= '0';
      for i in 0 to 100 loop
         if (i < 32) then
            codec_dout <= codec_word(31-i);
         else
            codec_dout <= '0';
         end if;
         codec_bclk <= '0';
         wait for 41.6 ns;
         codec_bclk <= '1';
         wait for 41.6 ns;
      end loop;
      codec_word <= codec_word + 1;
    end loop;
   end process;


   p3m(3) <= aspi_clk;
   aspi_di <= p3m(2);
   p3m(1) <= aspi_do;
   sample_valid <= p3m(7);

   -- Audio SPI 7MHz
   audio_spi: process
      variable l : line; 
   begin
      aspi_do <= '0';
      aspi_clk <= '0';
      aspi_data(31 downto 16) <= "1110111000000001"; --EE01xxxx
      aspi_data(15 downto 0) <= (others => '0');
      wait until p3m(0) = '0'; --reset

      wait for 500 ns; --inicializace SPI portu
      aspi_clk <= '1';

      while (true) loop
         wait until sample_valid = '1';
         wait for 10 ns;
   
         l := NEW string'(""); 
         write(l,string'("SPI write:"));
         write(l, hstr(aspi_data));
         writeline(output,l);
         
         -- read/write 16+16 bits
         for i in 0 to 31 loop
             aspi_clk <= '0';

             aspi_do <= aspi_data(31-i);
             wait for 67.5 ns;
  
             aspi_clk <= '1';

             aspi_idata(31-i) <= aspi_di;
             wait for 67.5 ns;
         end loop;
  
         aspi_data <= aspi_data + 1;

         l := NEW string'(""); 
         write(l,string'("SPI read:"));
         write(l, hstr(aspi_idata));
         writeline(output,l);

      end loop;
      
   end process;


   --SPI, init
   mcu_spi: process
   begin
      ispi_cs <= '1';
      -- FPGA ID, WE, ADDRH, ADDRL, DATAH, DATAL
      spi_data <= "000100011111000000000011";
      wait until p3m(0) = '0'; --reset
      wait for 10 ns;
      wait until ispi_clk='0'; 

      ispi_cs <= '0';
      for i in 23 downto 0 loop
          ispi_do <= spi_data(i);
     	    wait until ispi_clk='1';
          wait until ispi_clk='0';  
      end loop;

      ispi_cs <= '1';
      wait;
   end process;      


end architecture behavioral;
