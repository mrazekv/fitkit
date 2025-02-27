library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

architecture Behavioral of tlv_gp_ifc is
	signal A : std_logic_vector(3 downto 0) := "0001";
	signal R : std_logic_vector(7 downto 0) := "01011100";
	signal cnt : std_logic_vector(23 downto 0) := (others => '0');  -- 1s - 20MHz / 20M ~ 24b
begin

	process (clk) is
		
	begin
		if rising_edge(clk) then
			cnt <= cnt + 1;
			if conv_integer(cnt) = 0 then
				A <= A + 1;
			end if;
		end if;
	
	end process;

	

    -- mapovani vystupu
    -- nemenit
    X(6) <= A(3);
    X(8) <= A(1);
    X(10) <= A(0);
    X(7) <= '0'; -- en_n
    X(9) <= A(2);

    X(16) <= R(1);
    X(18) <= R(0);
    X(20) <= R(7);
    X(22) <= R(2);
  
    X(17) <= R(4);
    X(19) <= R(3);
    X(21) <= R(6);
    X(23) <= R(5);
end Behavioral;

