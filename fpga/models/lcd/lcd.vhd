-- lcd.vhd: LCD simulation model 
-- Author(s): Tomas Martinek <martinto at fit.vutbr.cz>
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
entity lcd is
port(
   LRS      : in    std_logic;
   LRW      : in    std_logic;
   LE       : in    std_logic;
   LD       : inout std_logic_vector(7 downto 0)
);
end entity lcd;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of lcd is

   type   t_ram         is array(0 to 255) of std_logic_vector(7 downto 0);
   type   t_table       is array(0 to 255) of character;

   signal cnt_addr      : std_logic_vector(6 downto 0) := (others => '0');
   signal ram           : t_ram := ((others => X"20"));
   signal display       : string(1 to 16) := (others => ' ');
   signal mx_data_out   : std_logic_vector(7 downto 0);
   signal busy_flag     : std_logic;

   -- Table of symbols - FIXME
   signal table         : t_table := (
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ','!','"','#','$','%','&',' ','(',')','*','+',',','-','.','/',
      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
      '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
      'P','Q','R','S','T','U','V','W','X','Y','Z','[',' ',']','^','_',
      ' ','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
      'p','q','r','s','t','u','v','w','x','y','z','{','|','}',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
      ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '
      );
begin

busy_flag <= '0';

-- data output multiplexor
mx_data_out <= busy_flag & cnt_addr when (LRS = '0') else
               ram(conv_integer(cnt_addr));

-- rasing edge process
process 
begin
   LD <= (others => 'Z');
   wait until (LE'event and LE='1');
   if (LRW = '1') then  -- read
      wait for 200 ns;
      LD <= mx_data_out;
      wait until (LE'event and LE='0');
      wait for 200 ns;
   end if;
end process;

-- falling edge process
process
begin
   wait until (LE'event and LE='0');
   if (LRW = '0') then 
      if (LRS = '0') then
         -- 0000 0001 Clear Display - - - - - - - - - - - - - - - -
         if (LD = "00000001") then
            report "LCD: Clear Display" severity note;
            ram <= (others => X"20");
         end if;
         -- 0000 001- Cursor Home - - - - - - - - - - - - - - - - -
         if (LD(7 downto 1) = "0000001") then
            report "LCD: Cursor Home" severity note;
         end if;
         -- 0000 01IS Entry Mode Set  - - - - - - - - - - - - - - -
         if (LD(7 downto 2) = "000001") then
            report "LCD: Entry Mode Set" severity note;
         end if;
         -- 0000 1DCB Display on/off Control  - - - - - - - - - - -
         if (LD(7 downto 3) = "00001") then
            report "LCD: Display on/off Control" severity note;
         end if;
         -- 0001 SR-- Display/Cursor Shift - - - - - - - - - - - -
         if (LD(7 downto 4) = "00001") then
            report "LCD: Display/Cursor shift" severity note;
         end if;
         -- 001D NF-- Set Function - - - - - - - - - - - - - - - -
         if (LD(7 downto 5) = "001") then
            report "LCD: Set Function" severity note;
         end if;
         -- 01aa aaaa CGRAM Address setting  - - - - - - - - - - -
         if (LD(7 downto 6) = "01") then
            report "LCD: CGRAM Address Setting" severity note;
         end if;
         -- 1aaa aaaa DGRAM Address setting  - - - - - - - - - - -
         if (LD(7) = '1') then
            report "LCD: DDRAM Address Setting" severity note;
            cnt_addr <= LD(6 downto 0);
         end if;
      else
         -- Write Data
         ram(conv_integer(cnt_addr)) <= LD;
         cnt_addr <= cnt_addr+1;
      end if;
   end if;
end process;

-- display process
process(ram, table)
begin
   for i in 0 to 7 loop
      display(i+1) <= table(conv_integer(ram(i)));
   end loop;
   for i in 0 to 7 loop
      display(i+8+1) <= table(conv_integer(ram(i+16#40#)));
   end loop;
end process;

end architecture behavioral;

