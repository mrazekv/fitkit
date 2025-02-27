-- Pexeso - simple pure FPGA game
-- Author: Vojtech Mrazek (xmraze06)

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;

entity bcd_cnt is
   port (
      CLK : in std_logic;
      RESET : in std_logic;
      
      DATA0 : out std_logic_vector(3 downto 0);
      DATA1 : out std_logic_vector(3 downto 0)
      
   );
end bcd_cnt;

architecture behv of bcd_cnt is
   signal sdata0,sdata1 : std_logic_vector(3 downto 0);
begin
   process (CLK,RESET)
   
   begin
      if RESET='1' then
         sdata0 <= (others => '0');
         sdata1 <= (others => '0');
      elsif CLK'event and CLK='1' then
         if sdata0 = 9 then
            sdata0 <= "0000";
            if sdata1 = 9 then
               sdata1 <= "0000";
            else
               sdata1 <= sdata1 + 1;
            end if;
         else
            sdata0 <= sdata0 + 1;
         end if;
      end if;
   
   end process;
   
   DATA0 <= sdata0;
   DATA1 <= sdata1;
   
end;