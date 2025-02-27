-- Random number generator
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity random is
   generic ( width : integer :=  4 ); 
   port (
      clk : in std_logic;
      random_num : out std_logic_vector (width-1 downto 0)   --output vector            
   );
end random;

architecture Behavioral of random is
   begin
      process(clk)
         variable rand_temp : std_logic_vector(width+5 downto 0) := (5 => '1',others => '0');
         variable temp : std_logic := '0';
      begin
         if(rising_edge(clk)) then
            temp := rand_temp(width+5) xor rand_temp(width+4);
            rand_temp(width+5 downto 1) := rand_temp(width+4 downto 0);
            rand_temp(0) := temp;
         end if;
         -- Posun o 1 bit, jinak by tam nebyla nikdy 0000
         random_num <= rand_temp(width+5 downto 5);
      end process;
end;