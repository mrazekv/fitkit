library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ftdi_spi is
port(
   CLK    : in std_logic;
   RST    : in std_logic;

   -- FTDI SPI
   FTDI_SCK : in std_logic;
   FTDI_CS : in std_logic;
   FTDI_DO : in std_logic;
   FTDI_DI : out std_logic;

   -- DATA
   DLEN  : in std_logic_vector(7 downto 0);
   DIN   : in std_logic;
   DSHFT : out std_logic;

   DSHFTIN : out std_logic;
   DOUT :  out std_logic;

   DONE : out std_logic

);
end entity ftdi_spi;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture full of ftdi_spi is
  signal ftdisck : std_logic_vector(1 downto 0) := "00";
  signal ftdics  : std_logic := '0';

  signal bitcnt : std_logic_vector(7 downto 0) := (others => '0');
  signal sckfal : std_logic;
  signal sckris : std_logic;

begin

   FTDI_DI <= DIN;
   sckfal <= '1' when ((ftdisck(1) = '1') and (ftdisck(0) = '0') and (ftdics = '1')) else '0'; --falling edge
   sckris <= '1' when ((ftdisck(1) = '0') and (ftdisck(0) = '1') and (ftdics = '1')) else '0'; --raising edge

   process (RST, CLK)
   begin
      if (RST = '1') or (FTDI_CS='0') then
         DONE <= '0';
         DSHFT <= '0';
         DSHFTIN <= '0';
         DOUT <= '0';
         ftdisck(1 downto 0) <= "00";
         bitcnt <= (others => '0');
         ftdics <= '0';
      elsif (CLK'event) and (CLK = '1') then
         ftdisck(0) <= FTDI_SCK;
         ftdisck(1) <= ftdisck(0);
         ftdics     <= FTDI_CS;

         DONE <= '0';
         DSHFT <= '0';
         DSHFTIN <= '0';
         if (sckris = '1') then
            DSHFTIN <= '1';
            DOUT <= FTDI_DO;
         end if;

         if (sckfal = '1') then
            if (bitcnt = DLEN) then
               DONE <= '1';
               bitcnt <= (others => '0');
            else
               DSHFT <= '1';
               bitcnt <= bitcnt + 1;
            end if;
         end if;

      end if;
   end process;


end architecture;
