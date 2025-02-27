# Kostra projektu
Ta obsahuje základní architekturu GP (definované originálně v [SVN](../trunk/fpga/chips/architecture_gp/)). UCF soubor pak definuje propojení výstupů - pro použití je nejdůležitější práce s portem X. Tam je pak možné např rozblikat LED a podobně.

Potřebujete pracovat s ISE (otevřít proj.xise)

## Nainstalování ISE
Je možné Xilinx ISE nainstalovat do Linuxu nebo Windows WSL2 (pozor, je nutná vyšší verze 2).

- Stáhněte si ISE 14.7 pro Linux. JE nutná registrace. https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive-ise.html
- Vytvořte složku /opt/Xilinx a určete jí práva

```sh
mkdir /opt/Xilinx
chown $USER:USER /opt/Xilinx
```
- Rozbalte kdekoliv stažený archiv a spusťte ./xsetup
- Vyberte WebPack verzi a postupujte dle instalace

```sh
# načtení nastacení
. /opt/Xilinx/14.7/ISE_DS/settings64.sh
xlcm # správce licencí, nutné pouze poprvé
ise
```

Pokud by nebyla nalezena licence, na stránce https://www.xilinx.com/getlicense.html se přihlaste, vyberte ISE WebPAck a stáhněte ji (jako Xilinx.lic). V licenčním manažeru (xlcm) klikněte na Manage licences a importujte váš stažený soubor.

## Nahravání do FITkitu 
Poznámka: z ISE vytvoříte soubor .bin. Soubory hex jsou pro MCU, které nevyužíváte. Proto jsou už přiložené ve složce [upload](upload). Pro MCU musíte vytvořit soubor pro používané MCU msp430. Můžete využít multiplatformní nástroj pro nahrávání binárního kódu.

https://github.com/janchaloupka/fitkit-serial/releases

```
fitkit-serial-win-amd64.exe -hex1 testled_f1xx.hex -hex2 testked_f2xx.hex -bin testled.bin"
```
## Prezentace
Nekomentované části ukázek použivané při přednášce

```
.\fitkit-serial-win-amd64.exe -list
.\fitkit-serial-win-amd64.exe -term
```
### Rozsvícení diod

```vhdl
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

architecture Behavioral of tlv_gp_ifc is
	signal leds : std_logic_vector(7 downto 0) := "10000001";
begin
		
	ledc: for i in 0 to 7 generate
		X(8 + 2*i) <= leds(i);
	end generate;

end Behavioral;
```

a nahráním


```ps
cd C:\Users\Vojta\ivh
.\fitkit-serial-win-amd64.exe -hex1x testled_f1xx.hex -hex2x testled_f2xx.hex -bin fpga.bin -flash -term
```



### Proces s blikáním
Zkusíme rozblikat ledky
```vhdl
architecture Behavioral of tlv_gp_ifc is
	signal leds : std_logic_vector(7 downto 0) := "10000001";
	
	-- 25 MHz na 2 Hz
	subtype tcnt is integer range 0 to 25000000/2-1;
	signal cnt : tcnt := 0;
	signal en : std_logic;
begin

	process (CLK) is
	begin
		if rising_edge(CLK) then
			en <= '0';
			if cnt = tcnt'HIGH then
				cnt <= 0;
				en <= '1';
			else
				cnt <= cnt + 1;
			end if;
		end if;
	end process;
	
	process (CLK) is
	begin
		if rising_edge(CLK) then
			if en = '1' then
				leds <= not leds;
			end if;
		end if;
	end process;
		
	ledc: for i in 0 to 7 generate
		X(8 + 2*i) <= leds(i);
	end generate;

end Behavioral;
```




### Simulace
Je třeba vygenerovat simulační skript v ISE pro entitu  `tvl_gp_ifc`.

* nastavit
```vhdl
   constant CLK_period : time := 40 ns; -- 20 MHz  
   constant SMCLK_period : time := 1.356 us; -- 7.3728 MHz
```
* spustit `run 1s`

Závěr: reálný čas se nedá simulovat, pro simulaci bychom měli zkrátit časy

```vhdl
	-- 25 MHz na 2 Hz
	constant initfreq : integer := 25; -- simulace
	--constant initfreq : integer := 25000000; -- synteza
	
	subtype tcnt is integer range 0 to initfreq/2-1;
```

### Vytvoření samostatné entity
Samozřejmě nemusíme mít jen jednu entitu...

```vhdl
	eg: entity work.engen generic map (TICKS => initfreq / 4) port map (clk => clk, en => en);
```

```vhdl
-- engen.vhd
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;


entity engen is
  generic (
    TICKS : integer := 10E6
  );
    Port ( clk : in  STD_LOGIC;
           en : out  STD_LOGIC);
end engen;

architecture Behavioral of engen is
	subtype tcnt is integer range 0 to TICKS-1;
	signal cnt: tcnt  := 0;
begin

	citac : process( CLK )
	begin
		 if( rising_edge(CLK) ) then
			  EN <= '0';
			  if (cnt = tcnt'high) then
				  cnt <= 0;
				  EN <= '1';
			  else
				  cnt <= cnt + 1;
			  end if;
		 end if ;
	end process ; -- citac

end Behavioral;
```

### Světelný had
Ukázka pro blikání ledek na portu X - základní had běžící doleva
```vhdl
	eg: entity work.engen generic map (TICKS => initfreq / 2) port map (clk => clk, en => en);

	process (CLK) is
	begin
	   if rising_edge(CLK) then
	      if en = '1' then
		  	leds <= leds(6 dowtno 0) & leds(7);
		  end if;
		end if;
	end process;
```
### Čítač nahoru a dolů
Had běží doleva a doprava
```
	-- do signálů
	signal leds : std_logic_vector(7 downto 0) := "00000001";

	signal led_cnt : std_logic_vector(2 downto 0) := "000";
	signal led_dir : std_logic := '0';
	signal led_cnt_rst : std_logic := '0';
	signal led_cnt_last : std_logic;


	-- k procesům
	-- proces pro citani stavu
	process (CLK)
	begin
		if (CLK'event and CLK='1') then
			if (led_cnt_rst = '1') then
				led_cnt <= (others => '0');
			elsif (en = '1') then
				led_cnt <= led_cnt + 1;
			end if;
		end if;
	end process;
	led_cnt_last <= '1' when led_cnt = "110" else '0';

	-- vlastni rizeni stavu
	process (CLK)
	begin
		if (CLK'event and CLK='1') then
			led_cnt_rst <= '0';
			if (en = '1') then
				if (led_dir = '0') then --doleva
					leds <= leds(6 downto 0) & '0';
					if (led_cnt_last = '1') then
						led_dir <= '1';
						led_cnt_rst <= '1';
					end if;
				else --doprava
					leds <= '0' & leds(7 downto 1);
					if (led_cnt_last = '1') then
						led_dir <= '0';
						led_cnt_rst <= '1';
					end if;
				end if;
			end if;
	end if;
	end process; 
```
### Světelný had 3
Počítáme pouze do půlky, pak jdeme zpátky a pak zase posouváme nahoru

```vhdl
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

architecture Behavioral of tlv_gp_ifc is
	signal leds : std_logic_vector(7 downto 0) := "00000001";
	
	-- 25 MHz na 2 Hz
	--constant initfreq : integer := 25;
	constant initfreq : integer := 25000000;
	signal en : std_logic;
	
	signal led_val: std_logic_vector(2 downto 0) := "000";
	signal led_dir : std_logic := '0';
	
	type tstate is (s1, s2, s3, s4);
	signal pstate, nstate : tstate := s1;
	
begin

	eg: entity work.engen generic map (TICKS => initfreq / 4) port map (clk => clk, en => en);
	
	-- proces pro citani stavu
	process (CLK)
	begin
		if (CLK'event and CLK='1') then
			if (en = '1') then
				if led_dir = '0' then
					led_val <= led_val + 1;
				else
					led_val <= led_val - 1;
				end if;
			end if;
		end if;
	end process;

	process (led_val) is
	begin
		for i in leds'range loop
			if i = conv_integer(led_val) then
				leds(i) <= '1';
			else
				leds(i) <= '0';
			end if;
		end loop;
			
	end process;
	
	process (CLK) is
	begin

		if (CLK'event and CLK='1') then
			pstate  <= nstate;
		end if;
	end process; 

	process (pstate, led_val) is
	begin
		nstate <= s1;
		case pstate is
			when s1 =>
				nstate <= s1;
				if led_val = "011" then
					nstate <= s2;
				end if;
			when s2 =>
				nstate <= s2;
				if led_val = "000" then
					nstate <= s3;
				end if;
			when s3 =>
				nstate <= s3;
				if led_val = "111" then
					nstate <= s4;
				end if;
			when s4 =>
				nstate <= s4;
				if led_val = "000" then
					nstate <= s1;
				end if;

		end case;	
	end process;
	
	led_dir <= '0' when pstate = s1 or pstate = s3 else
					'1';
		
	ledc: for i in 0 to 7 generate
		X(8 + 2*i) <= leds(i);
	end generate;

end Behavioral;

```

### Nepravidelný vzor (ROM)
```vhdl
signal line_cnt : std_logic_vector(2 downto 0) := "000";
type trom is array (0 to 7) of std_logic_vector(7 downto 0);
signal rom : trom := 
       (X"3F", X"44", X"84", X"84", X"44", X"3F", X"00", X"00");

eg: entity work.engen generic map (TICKS => initfreq / 4) port map (clk => clk, en => en);

process (CLK)
begin
  if (CLK'event and CLK='1') then
     if (en = '1') then 
        line_cnt <= line_cnt + 1;
     end if;
  end if;
end process;

leds <= rom(conv_integer(line_cnt));
```



## Práce s QDevKit
https://youtu.be/0Q8TvkvfDHQ

https://www.fit.vut.cz/study/thesis-file/22463/22463.pdf 

