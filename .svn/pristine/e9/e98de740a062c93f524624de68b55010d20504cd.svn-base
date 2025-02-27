library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity sig_watch is
   generic (
      INPUTS : integer := 4
   );
   port( 
      CLK    : in std_logic;
      RST    : in std_logic;
      EN     : in std_logic;
      ACK    : in  std_logic_vector(INPUTS-1 downto 0);
      DIN    : in  std_logic_vector(INPUTS-1 downto 0);
      DATA   : out std_logic_vector(INPUTS-1 downto 0);
      IRQ_MASK : in std_logic_vector(INPUTS-1 downto 0);
      IRQ    : out std_logic_vector(INPUTS-1 downto 0)
   ); 
end sig_watch;


architecture beh of sig_watch is

signal watch : std_logic_vector(INPUTS-1 downto 0) := (others => '0');
signal dirq : std_logic_vector(INPUTS-1 downto 0) := (others => '0');
signal change : std_logic_vector(INPUTS-1 downto 0);

begin
   IRQ <= dirq;
   DATA <= watch;
   change <= (watch XOR DIN) and IRQ_MASK;
---   SOUT <= hold(INPUTS-1);
  
   process(CLK, RST) 
   begin 
      if (RST = '1') then
         watch <= (others => '0');
---         hold <= (others => '0');
         dirq  <= (others => '0');
      elsif (CLK'event) and (CLK='1') then 
         if (EN='1') then 

           for i in INPUTS-1 downto 0 loop
              if (IRQ_MASK(i) = '0') then
                 watch(i) <= DIN(i); 
                 dirq(i) <= '0';
              else
                 if (dirq(i) = '0') then
                    if (change(i) = '1')  then
                       dirq(i) <= '1';
                       watch(i) <= DIN(i);  --sample, change(i) -> 0
   ---                    hold(i) <= DIN(i);  --hold
                    end if;
                 else
     --               if (change(i) = '1') then -- data changed between irq and ack
     --               end if;
                    if (ACK(i) = '1') then
                       dirq(i) <= '0';
                    end if;
   ---              elsif (SHIFT = '1') then
   ---                    if (i > 0) then
   ---                       hold(i) <= hold(i-1);
   ---                    else
   ---                       hold(i) <= '0';
   ---                    end if;
   ---                 end if;
                 end if;
              end if;
           end loop;
 
        end if;
      end if;  
   end process;

end architecture;




library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity data_watch is
   generic (
      DATA_WIDTH : integer := 4
   );
   port( 
      CLK    : in std_logic;
      RST    : in std_logic;
      EN     : in std_logic;

      ACK    : in  std_logic;
      DIN    : in  std_logic_vector(DATA_WIDTH-1 downto 0);
      DATA   : out std_logic_vector(DATA_WIDTH-1 downto 0);
      IRQ    : out std_logic;
      IRQ_MASK : in std_logic_vector(DATA_WIDTH-1 downto 0);

      SHIFT  : in std_logic;
      SOUT   : out std_logic;
      DLEN   : out std_logic_vector(7 downto 0)
   ); 
end data_watch;


architecture beh of data_watch is

signal iack : std_logic_vector(DATA_WIDTH-1 downto 0);
signal idata : std_logic_vector(DATA_WIDTH-1 downto 0);
signal iirq : std_logic_vector(DATA_WIDTH-1 downto 0);
signal hold : std_logic_vector(DATA_WIDTH-1 downto 0) := (others => '0');
signal girq : std_logic;
signal girqacpt : std_logic := '0';

begin
   IRQ <= girqacpt;
   DATA <= idata;
   DLEN <= conv_std_logic_vector(DATA_WIDTH-1, 8);
   SOUT <= hold(DATA_WIDTH-1);

   ackmap: for i in 0 to DATA_WIDTH-1 generate
       iack(i) <= ACK;
   end generate;

   process (RST, CLK)
   begin
      if (RST = '1') then
         hold <= (others => '0');
         girqacpt <= '0';
      elsif (CLK'event) and (CLK = '1') then
         if (EN = '1') then
            if (girqacpt = '1') then
               if (ACK = '1') then
                  girqacpt <= '0';
               elsif (SHIFT = '1') then
                  hold <= hold(DATA_WIDTH-2 downto 0) & hold(DATA_WIDTH-1);
               end if;
            else
               if (girq = '1') then
                  hold <= idata;
                  girqacpt <= '1';
               end if;
            end if;
         end if;
      end if;
   end process;

   signals_watch: entity work.sig_watch
         generic map (
            INPUTS => DATA_WIDTH
         )
         port map ( 
            CLK  => CLK,
            RST  => RST,
            EN   => EN,
            ACK  => iack,
            DIN  => DIN,
            IRQ_MASK => IRQ_MASK,
            DATA => idata,
            IRQ  => iirq
         );
   
   interrupt_logic: entity work.or_tree
         generic map (
            INPUTS => DATA_WIDTH
         )
         port map ( 
            DIN  => iirq,
            DOUT  => girq
         );

end architecture;



