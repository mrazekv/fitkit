-- Pexeso - simple pure FPGA game
-- Author: Vojtech Mrazek (xmraze06)

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;

entity symbol_ram is
   generic (
      ADDRESS_SIZE : integer := 4;
      SYMBOL_SIZE  : integer := 4
   );
   port (
      CLK : in std_logic;
      RESET : in std_logic;
      GEN : in std_logic;
      
      -- Port pro CTL
      ADDR_A : in std_logic_vector(ADDRESS_SIZE - 1 downto 0);
      DATA_A : out std_logic_vector(SYMBOL_SIZE - 1 downto 0);
      
      
      -- Port pro zapis
      ADDR_W : in std_logic_vector(ADDRESS_SIZE - 1 downto 0);
      DATA_W : in std_logic_vector(SYMBOL_SIZE - 1 downto 0);
      WE     : in std_logic;  
      
      -- Port pro VGA
      ADDR_B : in std_logic_vector(ADDRESS_SIZE - 1 downto 0);
      DATA_B : out std_logic_vector(SYMBOL_SIZE - 1 downto 0) 
      
   );
end symbol_ram;

architecture behv of symbol_ram is
   type t_mem is array(2**ADDRESS_SIZE-1 downto 0) of std_logic_vector(SYMBOL_SIZE - 1 downto 0);
   signal symbols : t_mem := ("0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111","1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111");
   
   signal random : std_logic_vector(3 downto 0);

begin
   rand: entity work.random
      generic map (width => 4)
      port map (
         clk => CLK,
         random_num => random
      );
      
   write: process
      variable in_set : std_logic := '0';
      variable element : std_logic_vector(3 downto 0);
      variable addr : std_logic_vector(3 downto 0);
      
   begin
      if CLK'event and CLK='1' then
         if RESET = '1' or GEN='1' then
            for I in 0 to 15 loop
               symbols(I) <= "0000";
            end loop;
            
            if GEN='1' then
               in_set := '1';
               element := "0000";
               addr := random;
            end if;
            
         -- zkusim nastavit    
         elsif in_set='1' then
            if symbols(conv_integer(addr))(3) = '0' then
               symbols(conv_integer(addr)) <= "1" & element(2 downto 0);
               element := element + 1;
               addr := random;
               if element = "0000" then
                  in_set := '0';
               end if;
            else
               addr := addr + 1;
            end if;
            
         elsif WE='1' then
            symbols(conv_integer(ADDR_W)) <= DATA_W;
         end if;
      end if;
   end process;
   
   read: process
   begin
      if CLK'event and CLK='1' then
         DATA_A <= symbols(conv_integer(ADDR_A));
         DATA_B <= symbols(conv_integer(ADDR_B));
      end if;
   end process;
   
end;