library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity data_watch2 is
   generic (
      DATA_WIDTH : integer := 4
   );
   port ( 
      CLK     : in std_logic;
      DIN     : in std_logic_vector(DATA_WIDTH-1 downto 0);
      -- sensitivity: 00 - none, 01 falling, 10 raising, 11 both edges
      SENS_RE : in std_logic_vector(DATA_WIDTH-1 downto 0); --sensitivity : raising edge
      SENS_FE : in std_logic_vector(DATA_WIDTH-1 downto 0); --sensitivity : falling edge
      -- change detected
      CHANGE  : out std_logic
   ); 
end data_watch2;


architecture beh of data_watch2 is

signal last : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
signal redetected : std_logic_vector(DATA_WIDTH-1 downto 0);
signal fedetected : std_logic_vector(DATA_WIDTH-1 downto 0);
signal achange : std_logic_vector(DATA_WIDTH-1 downto 0);
signal changed : std_logic;

begin
  redetected <= ((not last) and DIN) and SENS_RE;
  fedetected <= (last and (not DIN)) and SENS_FE;
  achange <= redetected or fedetected;

  changew: entity work.or_tree
           generic map ( INPUTS => DATA_WIDTH )
           port map ( 
              DIN  => achange,
              DOUT  => changed
           );

  process (CLK)
  begin
     if (CLK'event) and (CLK = '1') then
        last <= DIN;
        CHANGE <= changed;
     end if;
  end process;

end architecture;


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity DFIFO is
   generic (
      DATA_WIDTH : integer := 16;
      ADDR_WIDTH : integer := 2
   );
   port( 
      CLK   : in std_logic;
      EN    : in std_logic;   --enable read,should be '0' when not in use.
      WE    : in std_logic;   --enable write,should be '0' when not in use.
      DOUT  : out std_logic_vector(DATA_WIDTH-1 downto 0);    --output data
      DIN   : in  std_logic_vector(DATA_WIDTH-1 downto 0);     --input data
      EMPTY : out std_logic;    --set as '1' when the queue is empty
      FULL  : out std_logic
   );
end DFIFO;

architecture beh of DFIFO is
  type memory_type is array (0 to (2**ADDR_WIDTH)-1) of std_logic_vector(DATA_WIDTH-1 downto 0);
  signal memory : memory_type := (others => (others => '0'));   --memory for queue.
  signal readptr, writeptr : std_logic_vector(ADDR_WIDTH-1 downto 0) := (others => '0');  --read and write pointers.
  signal writeptrnext : std_logic_vector(ADDR_WIDTH-1 downto 0);
begin

  DOUT <= memory(conv_integer(readptr));
  writeptrnext <= writeptr + 1;
  FULL <= '1' when (writeptrnext = readptr) else '0';

  process (CLK)
  begin
     if (CLK'event) and (CLK = '1') then
        if (EN = '1') then
           readptr <= readptr + 1;
        end if;

        if (WE = '1') then
           memory(conv_integer(writeptr)) <= DIN;
           writeptr <= writeptrnext;
        end if;
     end if;
  end process;

  EMPTY <= '1' when (writeptr = readptr) else '0';
end architecture;






library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity XXXX is
   generic (
      DATA_WIDTH : integer := 16;
      FIFO_ADDR_WIDTH : integer := 2
   );
   port( 
      CLK    : in std_logic;
      EN     : in std_logic;

      ACK    : in std_logic;
      DIN    : in std_logic_vector(DATA_WIDTH-1 downto 0);
      DCHG   : in std_logic;
      IRQ    : out std_logic;

      SHIFT  : in std_logic;
      SOUT   : out std_logic
   );
end XXXX;

architecture beh of XXXX is
   signal fifo_en : std_logic;
   signal fifo_we : std_logic;
   signal fifo_empty : std_logic;
   signal fifo_full : std_logic;
   signal fifo_din : std_logic_vector(DATA_WIDTH-1 downto 0);
   signal fifo_dout : std_logic_vector(DATA_WIDTH-1 downto 0);
   signal datareg : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
   signal servereq : std_logic := '0';
   signal iirq : std_logic := '0';
begin

  fifo_din <= DIN;

  reqfifo: if (FIFO_ADDR_WIDTH > 0) generate
    reqfifo_inst:entity work.DFIFO
    generic map ( DATA_WIDTH => DATA_WIDTH, ADDR_WIDTH => FIFO_ADDR_WIDTH)
    port map ( 
       CLK  => CLK,
       EN   => fifo_en,
       WE   => fifo_we,
       DOUT => fifo_dout,
       DIN  => fifo_din,
       EMPTY => fifo_empty,
       FULL => fifo_full
    );
  end generate;
     

  SOUT <= datareg(DATA_WIDTH-1);
  IRQ <= iirq;

  process (CLK)
  begin
     if (CLK'event) and (CLK = '1') and (EN = '1') then
        fifo_en <= '0';
        fifo_we <= '0';
        iirq <= '0';

        if (servereq = '0') then
           if (DCHG = '1') then
              datareg <= DIN;
              servereq <= '1';
              iirq <= '1';
           end if;
        else
           iirq <= '1';

           if (DCHG = '1') and (ACK = '1') then
              if (fifo_empty = '1') then
                 datareg <= DIN;
              else 
                 datareg <= fifo_dout;
                 fifo_en <= '1';
                 fifo_we <= '1';
              end if;
           elsif (DCHG = '1') then
              fifo_we <= not fifo_full;

              if (SHIFT = '1') then
                 datareg <= datareg(DATA_WIDTH-2 downto 0) & '1';--datareg(DATA_WIDTH-1);
              end if;
           elsif (ACK = '1') then
              if (fifo_empty = '1') then
                 iirq <= '0';
                 servereq <= '0';
                 datareg <= datareg(DATA_WIDTH-2 downto 0) & '1'; --do nejvyssiho bitu se dostane 1
              else
                 datareg <= fifo_dout;
                 fifo_en <= '1';
              end if;
           elsif (SHIFT = '1') then
              datareg <= datareg(DATA_WIDTH-2 downto 0) & '1';--datareg(DATA_WIDTH-1);
           end if; 
        end if;

     end if;
  end process;
end architecture;


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity vmon is
   port( 
      CLK    : in std_logic;
      SYSCLK : in std_logic;
      RST    : in std_logic;

      AFBUS  : inout std_logic_vector(11 downto 0);
      D1     : in std_logic_vector(1 downto 0);
      D3     : in std_logic_vector(15 downto 0);

      KBO    : in std_logic_vector(3 downto 0);
      KBOM   : out std_logic_vector(3 downto 0);
      KBI    : in std_logic_vector(3 downto 0)
   );
end vmon;

architecture beh of vmon is

   
  constant DW:integer := 2;
  constant DW2:integer := 16;
  constant DW3:integer := 16;
  signal dlen_mx : std_logic_vector(7 downto 0);
  signal dout_mx : std_logic;

  signal data1_dlen : std_logic_vector(7 downto 0);
  signal data1_dout : std_logic;
  signal data1_sel : std_logic;
  signal data1_ack : std_logic;
  signal data1_shft : std_logic;
  signal data1_din : std_logic_vector(DW-1 downto 0);
  signal data1_irq : std_logic;

  signal data2_dlen : std_logic_vector(7 downto 0);
  signal data2_dout : std_logic;
  signal data2_sel : std_logic;
  signal data2_ack : std_logic;
  signal data2_shft : std_logic;

  signal data3_dlen : std_logic_vector(7 downto 0);
  signal data3_dout : std_logic;
  signal data3_sel : std_logic;
  signal data3_ack : std_logic;
  signal data3_chg : std_logic;
  signal data3_shft : std_logic;
  signal data3_din : std_logic_vector(DW3-1 downto 0);
  signal data3_irq : std_logic;

  signal data4_dlen : std_logic_vector(7 downto 0);
  signal data4_sel : std_logic;
  signal data4_dout : std_logic;
  signal data4_ack : std_logic;
  signal data4_shft : std_logic;


   signal d1_reg : std_logic_vector(DW-1 downto 0);
   signal d3_reg : std_logic_vector(DW3-1 downto 0);
   signal d3_ack, d3_req : std_logic := '0';


  signal reg4 : std_logic_vector(15 downto 0) := "0000000000000000";
  signal reg4w : std_logic_vector(15 downto 0) := "0000000000000000";

  signal aack : std_logic;
  signal ashft : std_logic;
  signal aintreq : std_logic;
  signal ishft : std_logic;
  signal idata : std_logic;

  signal DBGCLK : std_logic;
  signal DBGCLK_rdy : std_logic;

  signal reg2 : std_logic_vector(15 downto 0) := "0111110010010001";
  
begin

   --========================================================================
   -- vyuzit druhe DCM pro hodiny, ktere budou vzorkovat sck a datawatch
   -- f > 2*6MHz ~ 50MHz nebo 100MHz s dblbuferem by melo byt OK
   --========================================================================

   -- DCM - clock generator (default 25MHz)
--DBGCLK <= CLK;

   DCMclkgen: entity work.clkgen
    generic map (
        FREQ        => DCM_50MHz, GENIBUF => false
    )
    port map (
         CLK         => CLK,
         RST         => '0',
         CLK1X_OUT   => open,
         CLKFX_OUT   => DBGCLK,
         LOCKED_OUT  => DBGCLK_rdy
    ); 


   AFBUS(0) <= 'Z';
   AFBUS(1) <= 'Z';
   AFBUS(3) <= 'Z';

   --AFBUS(6) = doplnek AFBUS(4,5) kvuli synchronizaci - pokud neni naprog. FPGA, bude to vse vracet same jednicky nebo same nuly
   AFBUS(4) <= data1_irq;
   AFBUS(5) <= data3_irq;
   AFBUS(6) <= not (data1_irq or data3_irq); 
   AFBUS(7) <= '0';

   spiftd: entity work.ftdi_spi
       port map(
          CLK => DBGCLK,
          RST => RST,

          FTDI_CS => AFBUS(3),
          FTDI_SCK => AFBUS(0),
          FTDI_DO => AFBUS(1),
          FTDI_DI => AFBUS(2),

          DLEN => dlen_mx,
          DIN => dout_mx,
          DSHFT => ashft,

          DSHFTIN => ishft,
          DOUT => idata,
          
          DONE => aack
       );

   data1_sel <= '1' when AFBUS(9 downto 8) = "00" else '0';
   data2_sel <= '1' when AFBUS(9 downto 8) = "01" else '0';
   data3_sel <= '1' when AFBUS(9 downto 8) = "10" else '0';
   data4_sel <= '1' when AFBUS(9 downto 8) = "11" else '0';

   dlen_mx <= data1_dlen when data1_sel='1' else 
              data2_dlen when data2_sel='1' else
              data3_dlen when data3_sel='1' else
              data4_dlen when data4_sel='1' else
              (others => '0');

   dout_mx <= data1_dout when data1_sel='1' else
              data2_dout when data2_sel='1' else
              data3_dout when data3_sel='1' else
              data4_dout when data4_sel='1' else
              '0';
   
   data1_shft <= data1_sel and ashft;
   data1_ack <= data1_sel and aack;
   data2_shft <= data2_sel and ashft;
   data2_ack <= data2_sel and aack;
   data3_shft <= data3_sel and ashft;
   data3_ack <= data3_sel and aack;
   data4_shft <= data4_sel and ashft;
   data4_ack <= data4_sel and aack;

   data4_dlen <= conv_std_logic_vector(16-1, 8);
   data4_dout <= reg4(reg4'high);
   process (RST, DBGCLK)
   begin
      if (DBGCLK'event) and (DBGCLK = '1') then

          if (data4_sel = '1') and (ishft = '1') then
             reg4w <= reg4w(reg4w'high-1 downto 0) & idata;
          end if;

          if (data4_ack = '1') or (data4_shft = '1') then
             reg4 <= reg4(reg4'high-1 downto 0) & (not reg4(reg4'high));
          end if;

 
          -- klavesnice 4x4
          KBOM(0) <= KBO(0);
          if (reg4w(0) = '1') then
             KBOM(0) <= KBI(0);
          elsif (reg4w(1) = '1') then
             KBOM(0) <= KBI(1);
          elsif (reg4w(2) = '1') then
             KBOM(0) <= KBI(2);
          elsif (reg4w(3) = '1') then
             KBOM(0) <= KBI(3);
          end if;

          KBOM(1) <= KBO(1);
          if (reg4w(4) = '1') then
             KBOM(1) <= KBI(0);
          elsif (reg4w(5) = '1') then
             KBOM(1) <= KBI(1);
          elsif (reg4w(6) = '1') then
             KBOM(1) <= KBI(2);
          elsif (reg4w(7) = '1') then
             KBOM(1) <= KBI(3);
          end if;

          KBOM(2) <= KBO(2);
          if (reg4w(8) = '1') then
             KBOM(2) <= KBI(0);
          elsif (reg4w(9) = '1') then
             KBOM(2) <= KBI(1);
          elsif (reg4w(10) = '1') then
             KBOM(2) <= KBI(2);
          elsif (reg4w(11) = '1') then
             KBOM(2) <= KBI(3);
          end if;

          KBOM(3) <= KBO(3);
          if (reg4w(12) = '1') then
             KBOM(3) <= KBI(0);
          elsif (reg4w(13) = '1') then
             KBOM(3) <= KBI(1);
          elsif (reg4w(14) = '1') then
             KBOM(3) <= KBI(2);
          elsif (reg4w(15) = '1') then
             KBOM(3) <= KBI(3);
          end if;          


 
       end if;
   end process;

--         KBOM(0) <= KBI(0) when (reg4w(0) = '1') else 
--                    KBI(1) when (reg4w(1) = '1') else 
--                    KBI(2) when (reg4w(2) = '1') else
--                    KBI(3) when (reg4w(3) = '1') else KBO(0);
--      
--         KBOM(1) <= KBI(0) when (reg4w(4) = '1') else 
--                    KBI(1) when (reg4w(5) = '1') else 
--                    KBI(2) when (reg4w(6) = '1') else 
--                    KBI(3) when (reg4w(7) = '1') else KBO(1);
--      
--         KBOM(2) <= KBI(0) when (reg4w(8) = '1') else 
--                    KBI(1) when (reg4w(9) = '1') else 
--                    KBI(2) when (reg4w(10) = '1') else 
--                    KBI(3) when (reg4w(11) = '1') else KBO(2);
--      
--         KBOM(3) <= KBI(0) when (reg4w(12) = '1') else 
--                    KBI(1) when (reg4w(13) = '1') else 
--                    KBI(2) when (reg4w(14) = '1') else 
--                    KBI(3) when (reg4w(15) = '1') else KBO(3);

--==   -- handshake (SYSCLK x DBGCLK clock domain crossing)
--==   process (RST, SYSCLK)
--==   begin
--==      if (RST = '1') then
--==         d3_req <= '0';
--==      elsif (SYSCLK'event) and (SYSCLK = '1') then
--==         if (d3_req = '0') and ((D3 /= d3_reg) or (D1 /= d1_reg)) then
--==            d3_reg <= D3;
--==            d1_reg <= D1;
--==
--==            d3_req <= '1';
--==         elsif (d3_req = '1') and (d3_ack = '1') then
--==            d3_req <= '0';
--==
--==         end if;
--==      end if;
--==   end process;
--==
--==   process (RST, DBGCLK)
--==   begin
--==      if (RST = '1') then
--==         d3_ack <= '0';
--==      elsif (DBGCLK'event) and (DBGCLK = '1') then
--==         if (d3_ack = '0') and (d3_req = '1') then
--==            d3_ack <= '1';
--==            data1_din <= d1_reg;
--==            data3_din <= d3_reg;
--==         elsif (d3_ack = '1') and (d3_req = '0') then
--==            d3_ack <= '0';
--==         end if;
--==
--==      end if;   
--==   end process;


   process (DBGCLK)
   begin
     if (DBGCLK'event) and (DBGCLK = '1') then
        data1_din <= D1; -- led2 & led;
        data3_din <= D3;--"000000" & l_en & l_rs & l_ld(7 downto 0);
     end if;
   end process;
   
   iiled: entity work.data_watch
         generic map ( DATA_WIDTH => DW )
         port map ( 
            CLK  => DBGCLK,
            RST  => '0',
            EN   => '1',
            ACK  => data1_ack,
            DIN  => data1_din,
            DATA => open,
            IRQ  => data1_irq, --airq
            IRQ_MASK => "01",
            SHIFT => data1_shft,
            SOUT => data1_dout,
            DLEN => data1_dlen
         );


   lcd_watch: entity work.data_watch2 
      generic map ( DATA_WIDTH => 10 )
      port map ( 
          CLK => DBGCLK,
          DIN => data3_din(9 downto 0),
          SENS_RE => "1000000000",
          SENS_FE => "0000000000",
          CHANGE  => data3_chg
      ); 

   data3_dlen <= conv_std_logic_vector(16-1, 8);
      
   irq_feed: entity work.XXXX
      generic map ( DATA_WIDTH => 16, FIFO_ADDR_WIDTH => 7)
--      generic map ( DATA_WIDTH => 16, FIFO_ADDR_WIDTH => 9)
      port map ( 
          CLK => DBGCLK,
          EN  => '1',
          ACK => data3_ack,
          DIN => data3_din,
          DCHG => data3_chg,
          IRQ  => data3_irq,
          SHIFT => data3_shft,
          SOUT => data3_dout
     );
   
--   iilcd: entity work.data_watch
--         generic map ( DATA_WIDTH => DW3 )
--         port map ( 
--            CLK  => DBGCLK,
--            RST  => '0',
--            EN   => '1',
--            ACK  => data3_ack,
--            DIN  => data3_din,
--            DATA => open,
--            IRQ  => data3_irq, --airq
--            IRQ_MASK => "0000001000000000",
--            SHIFT => data3_shft,
--            SOUT => data3_dout,
--            DLEN => data3_dlen
--         );
         

   data2_dlen <= conv_std_logic_vector(DW2-1, 8);
   data2_dout <= reg2(reg2'high);
   process (RST, DBGCLK)
   begin
--      if (RST = '1') then
--         reg <= "11001010";
--      els
      if (DBGCLK'event) and (DBGCLK = '1') then
          if (data2_ack = '1') or (data2_shft = '1') then
--             reg <= "11001010";
             reg2 <= reg2(reg2'high-1 downto 0) & reg2(reg2'high);
--          elsif (data2_shft = '1') then
--             reg <= reg(6 downto 0) & reg(7);
          end if;
      end if;
   end process;

end architecture;
