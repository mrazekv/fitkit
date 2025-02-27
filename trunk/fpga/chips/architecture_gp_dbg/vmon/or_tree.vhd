library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use work.math_pack.all; 

entity or_tree is
generic(
   INPUTS : integer
);
port(
   DIN   : in std_logic_vector(INPUTS-1 downto 0);
   DOUT  : out std_logic
);
end entity or_tree;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture full of or_tree is

   constant LEVELS  : integer := log2(INPUTS);
   constant WIDTH   : integer := 2**LEVELS;

   type t_int is array (0 to LEVELS) of std_logic_vector(WIDTH-1 downto 0);
   signal tree : t_int;

begin
   tree(0)(INPUTS-1 downto 0) <= DIN;
   z: if (WIDTH > INPUTS) generate
      tree(0)(WIDTH-1 downto INPUTS) <= (others => '0');
   end generate;

   TREE_SUM : for i in 1 to LEVELS generate
      TREE_LEVEL : for j in 0 to (WIDTH/(2**i))-1 generate
         tree(i)(j*(2**i)) <= tree(i-1)(j*(2**i)) or tree(i-1)(j*(2**i) + (2**(i-1)));
      end generate;
   end generate;

   DOUT <= tree(LEVELS)(0);

end architecture;
