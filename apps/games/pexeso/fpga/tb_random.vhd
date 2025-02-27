LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

ENTITY testbench IS
END testbench;

ARCHITECTURE behavior OF testbench IS 
   --Input and Output definitions.
   signal clk : std_logic := '0';
   signal random_num : std_logic_vector(3 downto 0);
   -- Clock period definitions
   constant clk_period : time := 1 ns;
BEGIN
        -- Instantiate the Unit Under Test (UUT)
   uut: entity work.random generic map (width => 4) PORT MAP (
          clk => clk,
          random_num => random_num
        );
   -- Clock process definitions
   clk_process :process
   begin
                clk <= '0';
                wait for clk_period/2;
                clk <= '1';
                wait for clk_period/2;
   end process;

END;