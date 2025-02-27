-- ide_ctrl.vhd : IDE Controler
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Stanislav Sigmund <xsigmu02 AT stud.fit.vutbr.cz>
--
-- LICENSE TERMS
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
-- 1. Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in
--    the documentation and/or other materials provided with the
--    distribution.
-- 3. All advertising materials mentioning features or use of this software
--    or firmware must display the following acknowledgement:
--
--      This product includes software developed by the University of
--      Technology, Faculty of Information Technology, Brno and its
--      contributors.
--
-- 4. Neither the name of the Company nor the names of its contributors
--    may be used to endorse or promote products derived from this
--    software without specific prior written permission.
--
-- This software or firmware is provided ``as is'', and any express or implied
-- warranties, including, but not limited to, the implied warranties of
-- merchantability and fitness for a particular purpose are disclaimed.
-- In no event shall the company or contributors be liable for any
-- direct, indirect, incidental, special, exemplary, or consequential
-- damages (including, but not limited to, procurement of substitute
-- goods or services; loss of use, data, or profits; or business
-- interruption) however caused and on any theory of liability, whether
-- in contract, strict liability, or tort (including negligence or
-- otherwise) arising in any way out of the use of this software, even
-- if advised of the possibility of such damage.
--
-- $Id$
--
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity IDE_ctrl is
      generic (
         CLK_PERIOD  : integer := 20
      );
   port (
      -- Reset a synchronizace
      IDE     : inout std_logic_vector(45 downto 1) := (others => 'Z');
      CLK     : in std_logic; -- Hodiny radice (min. 4x vetsi, nez hodiny SPI)
      RST			:	in std_logic; -- RESET radice
      INTRQ		:	out std_logic;	-- Zadost o preruseni

      -- Interni rozhrani
      DI      : out std_logic;   -- Serial data out (MOSI)
      DI_REQ  : in std_logic;   -- DI request
      DO      : in std_logic;   -- Serial data in (MISO)
      DO_VLD  : in std_logic;   -- DO valid
      CS      : in std_logic    -- internal SPI active
   );
end IDE_ctrl;

architecture basic of IDE_ctrl is
-- system casovani, zatim se pocita s PIO 0
-- ior/iow ma byt nahozen az za 70 ns po platne adrese
constant IORW_VALID : integer := 70 / CLK_PERIOD + 1; -- 4
-- ior/iow ma drzet nejmene 290 ns (pro 8-bitovy rezim...)
constant IORW_HOLD : integer := 290 / CLK_PERIOD; -- 14
-- po shozeni iow maji byt data pritomna jeste min. 30 ns
constant IOW_HOLD : integer := 30 / CLK_PERIOD + 1; -- 2
-- po shozeni ior jsou data pritomna jeste 5ns - shodime ior pozdeji
constant IOR_HOLD : integer := 5 / CLK_PERIOD + 1; -- 1

constant IO_DELAY : integer := (IORW_HOLD + IORW_VALID + IOW_HOLD + 2);
subtype tcntr is integer range 0 to (IORW_HOLD + 1);
signal  delcntr : tcntr := 0; -- pocitadlo casovani
signal  delcntr_rst : std_logic := '1'; -- povoleni pocitadla 
    -- datove vodice d15 - d0:
type    RWstate is (SIdle,SRAVld,SWAVld,SIowHold,SIorHold,SWHold,SRHold);
signal  pstate    : RWstate := SIdle; -- actual state
signal  nstate    : RWstate := SIdle; -- next state
signal  cmp_avld : std_logic;
signal  cmp_rwhold : std_logic;
signal  cmp_rhold : std_logic;
signal  cmp_whold : std_logic;
-- signal  cmp_state_ch : std_logic;
signal  state_ch : std_logic;


signal  data    : std_logic_vector(15 downto 0) := (others => 'Z');
signal  data_in  : std_logic_vector(16 downto 0);
signal  data_in_valid : std_logic := '0';
signal  data_out_buff : std_logic_vector(15 downto 0);
signal  data_out : std_logic_vector(16 downto 0);
signal  data_out_ready   : std_logic := '0';
signal  iow     : std_logic := '1'; -- zapis na disk
signal  ior     : std_logic := '1'; -- cteni z disku
signal  read_data     : std_logic; -- chceme cist data
signal  write_data    : std_logic; -- chceme zapsat data
signal  ctrl_out : std_logic_vector(8 downto 0) := (others => '0');
signal  ctrl_in : std_logic_vector(8 downto 0) := (others => '0');
signal  ctrl_adc_out : std_logic_vector(8 downto 0) := (others => 'Z');
signal  ctrl_write    : std_logic; -- chceme zapsat ridici bity
   -- adresace registru v disku, vyvody CS1, CS0, DA2, DA1, DA0
signal  reg_addr: std_logic_vector(4 downto 0);
signal  buff_addr: std_logic_vector(4 downto 0);
signal  reg_adc_addr: std_logic_vector(4 downto 0);
signal  reg_addr_ready : std_logic := '0';

   component SPI_adc
      generic (
         ADDR_WIDTH : integer;
         DATA_WIDTH : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR  : integer;
         DELAY  : integer
      );
      port (
         CLK      : in  std_logic;

         CS       : in  std_logic;
         DO       : in  std_logic;
         DO_VLD   : in  std_logic;
         DI       : out std_logic;
         DI_REQ   : in  std_logic;

         ADDR     : out  std_logic_vector(ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out  std_logic_vector(DATA_WIDTH-1 downto 0);
         DATA_IN  : in   std_logic_vector(DATA_WIDTH-1 downto 0);

         WRITE_EN : out  std_logic;
         READ_EN  : out  std_logic
      );
   end component;

begin
-- datove vodice

idec: for i in 0 to 7 generate
 IDE(23+2*i) <= data(i);
 IDE(36-2*i) <= data(i+8);
end generate;

data <= data_out_buff when data_out_ready = '1' else (others => 'Z');

-- IDE(13) = IORDY
INTRQ <= IDE(9);

IDE(39) <= ctrl_out(0); -- RESET-
ctrl_in <= (others => '0');
   -- zemnici vodice
IDE(38) <= '0';IDE(21) <= '0';IDE(18) <= '0';IDE(16) <= '0';
IDE(14) <= '0';IDE(10) <= '0';-- IDE(0) <= '0';

cmp_avld <= '1' when (delcntr = IORW_VALID) else '0';
cmp_rwhold <= '1' when (delcntr = IORW_HOLD) else '0';
cmp_rhold <= '1' when (delcntr = IOR_HOLD) else '0';
cmp_whold <= '1' when (delcntr = IOW_HOLD) else '0';
-- cmp_state_ch <= '0' when (pstate = nstate) else '1';

IDE(2) <= reg_addr(4);IDE(3) <= reg_addr(3);
IDE(4) <= reg_addr(2);IDE(7) <= reg_addr(1);
IDE(5) <= reg_addr(0); -- cs1, cs0, da2, da1, da0
reg_addr <= buff_addr when reg_addr_ready = '1' else "11000";

IDE(17) <= iow;
IDE(15) <= ior;

-- nezapojene vyvody
-- DASP- DIAG- IOCS16- DMACK- CSEL
IDE(1) <= 'Z';IDE(6) <= 'Z';IDE(8) <= 'Z';IDE(11) <= 'Z';IDE(12) <= 'Z';
-- IORDY DMARQ keypin_20, nic
IDE(13) <= 'Z';IDE(19) <= 'Z';IDE(20) <= 'Z';IDE(40) <= 'Z';IDE(41) <= 'Z';
-- nic nic nic nic
IDE(42) <= 'Z';IDE(43) <= 'Z';IDE(44) <= 'Z';IDE(45) <= 'Z';
    
   SPI_adc_data: SPI_adc
      generic map(
         ADDR_WIDTH => 7,     
         DATA_WIDTH => 17,     
         ADDR_OUT_WIDTH => 5, 
         BASE_ADDR  => 16#00#,
         DELAY  => IO_DELAY
      )
      port map(
         CLK      => CLK,

         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => reg_adc_addr,
         DATA_IN  => data_in,
         READ_EN  => read_data,

         DATA_OUT => data_out,
         WRITE_EN => write_data
      );

   SPI_adc_ctrl: SPI_adc
      generic map(
         ADDR_WIDTH => 7,     
         DATA_WIDTH => 9,     
         ADDR_OUT_WIDTH => 1, 
         BASE_ADDR  => 16#20#,
         DELAY => 0
      )
      port map(
         CLK      => CLK,

         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => open,
         DATA_IN  => ctrl_in,
         READ_EN  => open,

         DATA_OUT => ctrl_adc_out,
         WRITE_EN => ctrl_write
      );
   data_store: process(CLK)
   begin
      if CLK'event and CLK = '1' then
      	if write_data = '1' then
      		data_out_buff <= data_out(15 downto 0);
				end if;
      end if;
   end process;

   data_in_store: process(CLK)
   begin
      if CLK'event and CLK = '1' then
      	if data_in_valid = '1' then
      		data_in <= '0' & data;
				end if;
      end if;
   end process;

   addr_store: process(CLK)
   begin
      if CLK'event and CLK = '1' then
      	if (read_data = '1') or (write_data = '1') then
      		buff_addr <= reg_adc_addr;
				end if;
      end if;
   end process;

   ctrl_set: process(CLK,RST)
   begin
  		if RST = '1' then
			  ctrl_out <= (others => '0');
			elsif CLK'event and CLK = '1' then
         if ctrl_write = '1' then
         	ctrl_out <= ctrl_adc_out;
         end if;
      end if;
   end process;

   rw_tick: process(CLK,RST)
   begin
  		if RST = '1' then
  			pstate <= SIdle;
      elsif CLK'event and CLK = '1' then
				pstate <= nstate;
      end if;
   end process;

   del_tick: process(CLK,delcntr_rst)
   begin
  		if delcntr_rst = '1' then
  			delcntr <= 0;
      elsif CLK'event and CLK = '1' then
      	if state_ch = '1' then
      		delcntr <= 0;
      	else
					delcntr <= delcntr + 1;
	      end if;
      end if;
   end process;

   rw_FSA: process(pstate,cmp_avld,cmp_rwhold,cmp_rhold,cmp_whold,read_data,write_data)
   begin
	   	state_ch <= '0';
      case pstate is
        --RW Idle
        when SIdle =>
		   		nstate <= SIdle;
        	if read_data = '1' then
	       		nstate <= SRAVld;
				   	state_ch <= '1';
	       	elsif write_data = '1' then
	       		nstate <= SWAVld;
				   	state_ch <= '1';
	       	end if;
        when SRAVld =>
       		nstate <= SRAVld;
        	if (cmp_avld = '1') then
        		nstate <= SIorHold;
				   	state_ch <= '1';
        	end if;
        when SWAVld =>
       		nstate <= SWAVld;
        	if (cmp_avld = '1') then
        		nstate <= SIowHold;
				   	state_ch <= '1';
        	end if;
        when SIorHold =>
       		nstate <= SIorHold;
        	if (cmp_rwhold = '1') then
        		nstate <= SRHold;
				   	state_ch <= '1';
        	end if;
        when SIowHold =>
       		nstate <= SIowHold;
        	if (cmp_rwhold = '1') then
        		nstate <= SWHold;
				   	state_ch <= '1';
        	end if;
        when SRHold =>
       		nstate <= SRHold;
        	if (cmp_rhold = '1') then
        		nstate <= SIdle;
				   	state_ch <= '1';
        	end if;
        when SWHold =>
       		nstate <= SWHold;
        	if (cmp_whold = '1') then
        		nstate <= SIdle;
				   	state_ch <= '1';
        	end if;
     end case;
   end process;

   rw_logic: process(pstate)
   begin
      data_out_ready <= '0';
      ior <= '1';
      iow <= '1';
      data_in_valid <= '0';
      reg_addr_ready <= '1';
      delcntr_rst <= '0';
      case pstate is
        --RW Idle
        when SIdle =>
		      delcntr_rst <= '1';
		      reg_addr_ready <= '0';
        when SRAVld =>
        when SWAVld =>
        when SIorHold =>
		      ior <= '0';
		      data_in_valid <= '1';
        when SIowHold =>
		      iow <= '0';
          data_out_ready <= '1';
        when SRHold =>
        when SWHold =>
          data_out_ready <= '1';
     end case;
   end process;

end basic;
