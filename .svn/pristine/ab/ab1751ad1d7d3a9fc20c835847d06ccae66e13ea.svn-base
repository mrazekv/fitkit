-- Pexeso - simple pure FPGA game
-- Author: Vojtech Mrazek (xmraze06)

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;

entity game_ctl is
   port (
      CLK : in std_logic;
      RESET : in std_logic;
      
      -- cursor
      CUR_X : in std_logic_vector(1 downto 0);
      CUR_Y : in std_logic_vector(1 downto 0);
      
      
      ENTER   : in std_logic;    --
      RESTART : in std_logic;
      
      RAM_GEN : out std_logic;    -- generovat novou sadu
      CNT_CLK : out std_logic;    -- poèítání poètu obrácení
      CNT_RST : out std_logic;    -- poèítání poètu obrácení - reset
      
      -- Port pro CTL
      RAM_ADDR_A : out std_logic_vector(3 downto 0);
      RAM_DATA_A : in std_logic_vector(3 downto 0);
      
      
      -- Port pro zapis
      RAM_ADDR_W : out std_logic_vector(3 downto 0);
      RAM_DATA_W : out std_logic_vector(3 downto 0);
      RAM_WE     : out std_logic  
      
   );
end game_ctl;

architecture behv of game_ctl is
   type t_state is (INIT, WAITFIRST, WAITSECOND, WAITTIME, HIDEFIRST, HIDESECOND);
   signal pstate, nstate : t_state; 
   signal card1, card2 : std_logic_vector(3 downto 0);
   signal card1data, card2data : std_logic_vector(2 downto 0);
   
   signal pairs : std_logic_vector(2 downto 0);
   
   signal clk_cntr : std_logic_vector(25 downto 0);
   signal clk_reset, clk_end : std_logic;
begin

   process (CLK, RESET)
   begin
      if CLK'event and CLK='1' then
         if RESET='1' or RESTART = '1' then
            pstate <= INIT;
         else
            pstate <= nstate;
         end if;
      end if;
   end process;
   
   timer : process (CLK)
   begin
      if clk_reset = '1' then
         clk_cntr <= (others => '0');
      elsif CLK'event and CLK='1' then
         clk_cntr <= clk_cntr + 1;           
      end if;
      clk_end <= clk_cntr(25); 
   end process;
   
   nstate_logic : process (CLK)
   begin
      RAM_GEN <= '0';
      RAM_WE <= '0';
      RAM_ADDR_A <= CUR_Y & CUR_X;
      RAM_ADDR_W <= CUR_Y & CUR_X;
      clk_reset <= '0';
      RAM_DATA_W <= "0" & RAM_DATA_A(2 downto 0);
      CNT_CLK <= '0';
      CNT_RST <= '0';
      
      case pstate is
         -- Povoleni inicializace RAM
         when INIT =>
            RAM_GEN <= '1';
            nstate <= WAITFIRST;   
            pairs <= "000";
            CNT_RST <= '1';
         
         -- Cekani na prvni kartu
         when WAITFIRST =>
            nstate <= WAITFIRST;
            if ENTER='1' then
               if RAM_DATA_A(3) = '1' then
                  -- otoceni karty
                  RAM_DATA_W <= "0" & RAM_DATA_A(2 downto 0);
                  RAM_WE <= '1';
                  
                  CNT_CLK <= '1';   -- zapocitej kartu
                  card1 <= CUR_Y & CUR_X; 
                  card1data <= RAM_DATA_A(2 downto 0);
                  nstate <= WAITSECOND;
               end if;
            end if;
         
         -- Cekani na druhou kartu
         when WAITSECOND =>
            nstate <= WAITSECOND;
            if ENTER='1' then
               if RAM_DATA_A(3) = '1' and card1 /= CUR_Y & CUR_X  then
                  RAM_DATA_W <= "0" & RAM_DATA_A(2 downto 0);
                  RAM_WE <= '1';
                  card2 <= CUR_Y & CUR_X; 
                  card2data <= RAM_DATA_A(2 downto 0);
                  if card1data = RAM_DATA_A(2 downto 0) then
                     nstate <= WAITFIRST;
                  else               
                     nstate <= WAITTIME;
                     clk_reset <= '1';
                  end if;
               end if;
            end if;
 
         -- Cekani na dobehnuti casovace
         when WAITTIME =>
            nstate <= WAITTIME;
            if clk_end = '1' then
               if card1data = card2data then
                  nstate <= WAITFIRST;
               else
                  nstate <= HIDEFIRST;
               end if;
            end if;
            
         -- Skryti prvni karty
         when HIDEFIRST =>
            nstate <= HIDESECOND;
            RAM_ADDR_W <= card1;
            RAM_DATA_W <= "1" & card1data;
            RAM_WE <= '1';
            
         -- Skryti druhe karty
         when HIDESECOND =>
            nstate <= WAITFIRST;
            RAM_ADDR_W <= card2;
            RAM_DATA_W <= "1" & card2data;
            RAM_WE <= '1';
            
            
         when others =>
            nstate <= INIT;
      
      end case;
      
   
   end process;
   
end;