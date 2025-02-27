-- keyboard.vhd: Keyboard simulation file
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
entity keyboard is
port(
   KEY  : in  character;
   KIN  : in  std_logic_vector(3 downto 0);
   KOUT : out std_logic_vector(3 downto 0)
);
end entity keyboard;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of keyboard is

signal matrix : std_logic_vector(15 downto 0);

begin

process (KEY)
begin
   case KEY is
      when '1' => matrix <= "1111" & "1111" & "1111" & "1110";
      when '4' => matrix <= "1111" & "1111" & "1111" & "1101";
      when '7' => matrix <= "1111" & "1111" & "1111" & "1011";
      when '*' => matrix <= "1111" & "1111" & "1111" & "0111";

      when '2' => matrix <= "1111" & "1111" & "1110" & "1111";
      when '5' => matrix <= "1111" & "1111" & "1101" & "1111";
      when '8' => matrix <= "1111" & "1111" & "1011" & "1111";
      when '0' => matrix <= "1111" & "1111" & "0111" & "1111";

      when '3' => matrix <= "1111" & "1110" & "1111" & "1111";
      when '6' => matrix <= "1111" & "1101" & "1111" & "1111";
      when '9' => matrix <= "1111" & "1011" & "1111" & "1111";
      when '#' => matrix <= "1111" & "0111" & "1111" & "1111";

      when 'A' => matrix <= "1110" & "1111" & "1111" & "1111";
      when 'B' => matrix <= "1101" & "1111" & "1111" & "1111";
      when 'C' => matrix <= "1011" & "1111" & "1111" & "1111";
      when 'D' => matrix <= "0111" & "1111" & "1111" & "1111";
      when others => matrix <= (others => '1');
   end case;
end process;

with KIN select
KOUT <= matrix( 3 downto  0) when "ZZZ0",
        matrix( 7 downto  4) when "ZZ0Z",
        matrix(11 downto  8) when "Z0ZZ",
        matrix(15 downto 12) when "0ZZZ",
        "1111"               when others;

end architecture behavioral;

