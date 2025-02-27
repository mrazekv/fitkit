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
      IDE     : inout std_logic_vector(25 downto 0) := (others => 'Z');
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


--signal  data_out_mux : std_logic_vector(15 downto 0);
signal  data    : std_logic_vector(15 downto 0) := (others => 'Z');
signal  data_in  : std_logic_vector(16 downto 0);
signal  data_in_valid : std_logic := '0';
signal  data_out_buff : std_logic_vector(15 downto 0);
signal  data_out : std_logic_vector(16 downto 0);
signal  data_out_ready   : std_logic := '0';
signal  DmaRQ   : std_logic;
signal  DmaACK   : std_logic := '1';
signal  iow     : std_logic := '1'; -- zapis na disk
signal  ior     : std_logic := '1'; -- cteni z disku
signal  read_data     : std_logic; -- chceme cist data
signal  write_data    : std_logic; -- chceme zapsat data
signal  ctrl_out : std_logic_vector(7 downto 0) := (others => '0');
signal  ctrl_in : std_logic_vector(7 downto 0) := (others => '0');
signal  ctrl_adc_out : std_logic_vector(7 downto 0) := (others => 'Z');
signal  ctrl_write    : std_logic; -- chceme zapsat ridici bity
   -- adresace registru v disku, vyvody CS1, CS0, DA2, DA1, DA0
signal  reg_addr: std_logic_vector(4 downto 0);
signal  buff_addr: std_logic_vector(4 downto 0);
signal  reg_adc_addr: std_logic_vector(4 downto 0);
signal  reg_addr_ready : std_logic := '0';
signal  Ctrl_Reg  : std_logic_vector(1 downto 0) := (others => '0');

constant DEL_DMA_RW0 : integer := 240 / CLK_PERIOD; -- 12
constant DEL_DMA_HOLD0 : integer := 280 / CLK_PERIOD; -- 14
constant DEL_DMA_WAIT0 : integer := 480 / CLK_PERIOD; -- 24
constant DEL_DMA_RW1 : integer := 80 / CLK_PERIOD; -- 4
constant DEL_DMA_HOLD1 : integer := (100+CLK_PERIOD-1) / CLK_PERIOD; -- 5
constant DEL_DMA_WAIT1 : integer := (150+CLK_PERIOD-1) / CLK_PERIOD; -- 8


signal  BSDOut  : std_logic_vector(7 downto 0);
signal  BSDIn  : std_logic_vector(7 downto 0);
signal  BSAdr  : std_logic_vector(10 downto 0);
signal  BSWe : std_logic := '0';
signal  BDAdr  : std_logic_vector(9 downto 0) := (others => '0');
signal  BDWe : std_logic := '0';
signal  BDOutEn : std_logic := '0';
signal  BDOut  : std_logic_vector(15 downto 0);

constant SPD_VALID : integer := (70+CLK_PERIOD-1) / CLK_PERIOD; -- 4
constant SPD_HOLD : integer := 360 / CLK_PERIOD; -- 18
constant SPD_WAIT : integer := (390+CLK_PERIOD-1) / CLK_PERIOD; -- 20

signal  spd_addr_en : std_logic := '0';
signal  spd_out  : std_logic_vector(7 downto 0);
signal  spd_addr  : std_logic_vector(4 downto 0);
signal  spd_addr_buff  : std_logic_vector(4 downto 0);
signal  spd_out_buff  : std_logic_vector(7 downto 0);
signal  write_spd : std_logic := '0';
signal  spd_en : std_logic := '0';
signal  spd_iow : std_logic := '1';
signal  SpdCnt : std_logic_vector(5 downto 0) := (others => '0');
type    Spdstate is (SIdle,SVal,SHold,SWait);
signal  pSstate    : Spdstate := SIdle; -- actual state
signal  nSstate    : Spdstate := SIdle; -- next state
signal  spdcmpval : std_logic := '0';
signal  spdcmphold : std_logic := '0';
signal  spdcmpwait : std_logic := '0';

signal  DmaRun : std_logic := '0';
signal  NDmaRun : std_logic := '1';
signal  DmaRst : std_logic := '1';
signal  DmaCtrlW  : std_logic := '0';
signal  DmaBlock  : std_logic_vector(1 downto 0) := (others => '0');
signal  DmaIRW : std_logic := '0';
signal  DMA_Cnt_Rst : std_logic := '1';
signal  DmaInc : std_logic := '0';
signal  cmpDmaHold : std_logic := '0';
signal  cmpDmaWait : std_logic := '0';
signal  cmpDmaRW : std_logic := '0';
signal  DmaEnd : std_logic := '0';
signal  spd_res : std_logic := '0';
signal  DmaCnt : std_logic_vector(5 downto 0) := (others => '0');
signal  DmaSpd : std_logic := '0';
signal  DmaDir : std_logic := '0';
signal  NDmaDir   : std_logic := '1';
type    DMAstate is (SIdle,SRun,SWR,SHold,SWait);
signal  pDstate    : DMAstate := SIdle; -- actual state
signal  nDstate    : DMAstate := SIdle; -- next state
signal  BDOutWr : std_logic := '0';
 	         
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

   for SPI_adc_data: SPI_adc use entity work.SPI_adc(autoincrement);
   for SPI_adc_spd: SPI_adc use entity work.SPI_adc(autoincrement);

   component RAMB16_S9_S18
   port (
      DIA   : in  std_logic_vector(7 downto 0);   -- DATA IN
      DIPA  : in  std_logic_vector(0 downto 0);    -- DATA IN PARITY
      ADDRA : in  std_logic_vector(10 downto 0);    -- ADDRESS
      ENA   : in  std_logic;                       -- ENABLE
      WEA   : in  std_logic;                       -- WRITE ENABLE
      SSRA  : in  std_logic;                       -- SET/RESET
      CLKA  : in  std_logic;                       -- CLK
      DOA   : out std_logic_vector(7 downto 0);   -- DATA OUT
      DOPA  : out std_logic_vector(0 downto 0);     -- DATA OUT PARITY

      DIB   : in  std_logic_vector(15 downto 0);   -- DATA IN
      DIPB  : in  std_logic_vector(1 downto 0);    -- DATA IN PARITY
      ADDRB : in  std_logic_vector(9 downto 0);    -- ADDRESS
      ENB   : in  std_logic;                       -- ENABLE
      WEB   : in  std_logic;                       -- WRITE ENABLE
      SSRB  : in  std_logic;                       -- SET/RESET
      CLKB  : in  std_logic;                       -- CLK
      DOB   : out std_logic_vector(15 downto 0);   -- DATA OUT
      DOPB  : out std_logic_vector(1 downto 0)    -- DATA OUT PARITY
   ); 
   end component;

begin
-- 0-15 data
-- 16 RESET
-- 17 IOW
-- 18 IOR
-- 19-23 DA0 DA1 DA2 CS0 CS1 
-- 24 DMARQ
-- 25 DMAACK
-- datove vodice
idec: for i in 0 to 15 generate
 IDE(i) <= data(i);
end generate;

-- adresni vodice
iaddr: for i in 0 to 4 generate
 IDE(19+i) <= reg_addr(i);
end generate;

DmaRQ <= IDE(24);IDE(25) <= DmaACK;
BDOutWr <= '1' when (BDOutEn = '1' AND DmaDir = '1') else '0';

data <= data_out_buff when data_out_ready='1' else
				("00000000" & spd_out_buff) when spd_en='1' else
      	BDOut when BDOutWr='1' else
				(others => 'Z'); 

--data_out_mux <= BDOut when (BDOutWr = '1') else data_out_buff;
--data <= data_out_mux when (data_out_ready = '1' OR BDOutWr = '1') else (others => 'Z');

IDE(16) <= ctrl_out(0); -- RESET-
ctrl_in <= DmaRun & DmaRQ & DmaSpd & DmaDir & "00" & DmaBlock(1 downto 0) when (Ctrl_Reg = "01") else 
DmaACK & DmaRQ & DmaIRW & reg_addr(4 downto 0);

cmp_avld <= '1' when (delcntr = IORW_VALID) else '0';
cmp_rwhold <= '1' when (delcntr = IORW_HOLD) else '0';
cmp_rhold <= '1' when (delcntr = IOR_HOLD) else '0';
cmp_whold <= '1' when (delcntr = IOW_HOLD) else '0';
-- cmp_state_ch <= '0' when (pstate = nstate) else '1';

reg_addr <= buff_addr when (reg_addr_ready = '1') else 
  spd_addr_buff when (spd_addr_en = '1') else "11000";

NDmaDir <= '1' when (DmaDir = '0') else '0';
IDE(17) <= (iow AND spd_iow) when (DmaIRW='0') else NDmaDir;
IDE(18) <= ior when (DmaIRW='0') else DmaDir;

   SPI_adc_cmd: SPI_adc
      generic map(
         ADDR_WIDTH => 15,     
         DATA_WIDTH => 17,     
         ADDR_OUT_WIDTH => 5, 
         BASE_ADDR  => 16#0180#,
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

   SPI_adc_spd: SPI_adc
      generic map(
         ADDR_WIDTH => 16,     
         DATA_WIDTH => 8,     
         ADDR_OUT_WIDTH => 5, 
         BASE_ADDR  => 16#0000#,
         DELAY  => 0
      )
      port map(
         CLK      => CLK,

         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => spd_addr,
         DATA_IN  => "00000000",
         READ_EN  => open,

         DATA_OUT => spd_out,
         WRITE_EN => write_spd
      );

   SPI_adc_ctrl: SPI_adc
      generic map(
         ADDR_WIDTH => 8,     
         DATA_WIDTH => 8,     
         ADDR_OUT_WIDTH => 2, 
         BASE_ADDR  => 16#04#,
         DELAY => 1
      )
      port map(
         CLK      => CLK,

         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => Ctrl_Reg,
         DATA_IN  => ctrl_in,
         READ_EN  => open,

         DATA_OUT => ctrl_adc_out,
         WRITE_EN => ctrl_write
      );

   SPI_adc_data: SPI_adc
      generic map(
         ADDR_WIDTH => 16,     
         DATA_WIDTH => 8,     
         ADDR_OUT_WIDTH => 11, 
         BASE_ADDR  => 16#0800#,
         DELAY => 1
      )
      port map(
         CLK      => CLK,

         CS       => CS,
         DO       => DO,
         DO_VLD   => DO_VLD,
         DI       => DI,
         DI_REQ   => DI_REQ,

         ADDR     => BSAdr,
         DATA_IN  => BSDIn,
         READ_EN  => open,

         DATA_OUT => BSDOut,
         WRITE_EN => BSWe
      ); 

   BUF: RAMB16_S9_S18
   port map(
      DIA   => BSDOut,
      DIPA  => "0",
      ADDRA => BSAdr,
      ENA   => '1',
      WEA   => BSWe,
      SSRA  => '0',
      CLKA  => CLK,
      DOA   => BSDIn,
      DOPA  => open,

      DIB   => data,
      DIPB  => "00",
      ADDRB => BDAdr,
      ENB   => '1',
      WEB   => BDWe,
      SSRB  => '0',
      CLKB  => CLK,
      DOB   => BDOut,
      DOPB  => open
   ); 

spdcmpval <= '1' when (SpdCnt = SPD_VALID) else '0';
spdcmphold <= '1' when (SpdCnt = SPD_HOLD) else '0';
spdcmpwait <= '1' when (SpdCnt = SPD_WAIT) else '0';

   spd_addr_store: process(CLK)
   begin
      if CLK'event and CLK = '1' then
      	if write_spd = '1' then
      		spd_addr_buff <= spd_addr;
      		spd_out_buff <= spd_out;
				end if;
      end if;
   end process;

   spd_cnt: process(CLK,spd_res)
   begin
   		if spd_res = '1' then
   			SpdCnt <= (others => '0');
   		elsif CLK'event and CLK = '1' then
     		SpdCnt <= SpdCnt + 1;
      end if;
   end process;

   spd_tick: process(CLK,RST)
   begin
  		if RST = '1' then
  			pSstate <= SIdle;
      elsif CLK'event and CLK = '1' then
				pSstate <= nSstate;
      end if;
   end process;

   spd_FSA: process(pSstate,spdcmpval,spdcmphold,spdcmpwait,write_spd)
   begin
      case pSstate is
        when SIdle =>
		   		nSstate <= SIdle;
        	if write_spd = '1' then
	       		nSstate <= SVal;
	       	end if;
        when SVal =>
       		nSstate <= SVal;
        	if (spdcmpval = '1') then
        		nSstate <= SHold;
        	end if;
        when SHold =>
      		nSstate <= SHold;
        	if (spdcmphold = '1') then
        		nSstate <= SWait;
        	end if;
        when SWait =>
       		nSstate <= SWait;
        	if (spdcmpwait = '1') then
        		nSstate <= SIdle;
        	end if;
     end case;
   end process;

   spd_logic: process(pSstate)
   begin
   		spd_addr_en <= '0';
   		spd_en <= '0';
   		spd_iow <= '1';
   		spd_res <= '0';
      case pSstate is
        when SIdle =>
		   		spd_res <= '1';
        when SVal =>
   				spd_addr_en <= '1';
        when SHold =>
   				spd_addr_en <= '1';
   				spd_iow <= '0';
   				spd_en <= '1';
        when SWait =>
   				spd_en <= '1';
   				spd_addr_en <= '1';
     end case;
   end process;

-- ******** normal *******

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
         if ctrl_write = '1' AND Ctrl_Reg = "00" then
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

-- ******************* DMA Sekce ********************
-- ctrl registr = R/W: DmaRun DmaRst DmaSpd DmaDir N/A N/A DmaBlock(2)
-- DmaSpd = 0/1 -> Dma0/Dma1
-- DmaDir = 0/1 -> R/W
-- pri R je misto DmaRst DmaRQ

DmaCtrlW <= '1' when ctrl_write = '1' AND Ctrl_Reg = "01" else '0';
DmaRst <= ctrl_adc_out(6) OR RST when Ctrl_Reg = "01" else RST;
NDmaRun <= '0' when (DmaRun = '1' AND DmaRQ = '1') else '1';
DmaEnd <= '0' when (BDAdr(9 downto 8) = DmaBlock(1 downto 0)) else '1'; 
cmpDmaRW <= '1' when (DmaCnt = DEL_DMA_RW0 AND DmaSpd = '0') OR 
	(DmaCnt = DEL_DMA_RW1 AND DmaSpd = '1') else '0';   
cmpDmaHold <= '1' when (DmaCnt = DEL_DMA_HOLD0 AND DmaSpd = '0') OR 
	(DmaCnt = DEL_DMA_HOLD1 AND DmaSpd = '1') else '0';   
cmpDmaWait <= '1' when (DmaCnt = DEL_DMA_WAIT0 AND DmaSpd = '0') OR 
	(DmaCnt = DEL_DMA_WAIT1 AND DmaSpd = '1') else '0';   

   ctrl_dma_set: process(CLK,RST)
   begin
  		if RST = '1' then
			  DmaRun <= '0';
			  DmaSpd <= '0';
			elsif CLK'event and CLK = '1' then
         if DmaCtrlW = '1' then
         	DmaBlock <= ctrl_adc_out(1 downto 0);
         	DmaRun <= ctrl_adc_out(7);
         	DmaSpd <= ctrl_adc_out(5);
         	DmaDir <= ctrl_adc_out(4);
         else
          if DmaEnd = '1' then DmaRun <= '0';
          end if;
         end if;
      end if;
   end process;

   Dma_Addr: process(CLK,DmaRst)
   begin
   		if DmaRst = '1' then
   			BDAdr(7 downto 0) <= (others => '0');
   		else
	      if CLK'event and CLK = '1' then
	        if DmaCtrlW = '1' then
	          BDAdr(9 downto 8) <= ctrl_adc_out(1 downto 0);
	        else 
		      	if DmaInc = '1' then
		      		BDAdr <= BDAdr + 1;
						end if;
					end if;
	      end if;
	    end if;
   end process;

   DMA_tick: process(CLK,DmaRst)
   begin
  		if DmaRst = '1' then
  			pDstate <= SIdle;
      elsif CLK'event and CLK = '1' then
				pDstate <= nDstate;
      end if;
   end process;

   DMA_cnt_tick: process(CLK,DMA_Cnt_Rst,DmaRst)
   begin
  		if DmaRst = '1' then
  			DmaCnt <= (others => '0');
      elsif CLK'event and CLK = '1' then
      	if DMA_Cnt_Rst = '1' then
      		DmaCnt <= (others => '0');
      	else
					DmaCnt <= DmaCnt + 1;
	      end if;
      end if;
   end process;

   DMA_FSA: process(pDstate,DmaRun,cmpDmaRW,cmpDmaHold,cmpDmaWait,DMARQ,DmaEnd,NDmaDir)
   begin
      BDWe <= '0';
      DmaInc <= '0';
      case pDstate is
        when SIdle =>
		   		nDstate <= SIdle;
        	if (DmaRun = '1') AND (DmaEnd = '0') then
	       		nDstate <= SRun;
	       	end if;
        when SRun =>
		   		nDstate <= SRun;
        	if (DmaRun = '1') AND (DmaRQ = '1') then
	       		nDstate <= SWR;
	       	end if;
        when SWR =>
       		nDstate <= SWR;
        	if (cmpDmaRW = '1') then
        		nDstate <= SHold;
        		BDWe <= NDmaDir;
        	end if;
        when SHold =>
       		nDstate <= SHold;
        	if (cmpDmaHold = '1') then
        		nDstate <= SWait;
        		DmaInc <= '1';
        	end if;
        when SWait =>
       		nDstate <= SWait;
       		if (DmaEnd = '1') then 
        		nDstate <= SIdle;
        	else
	        	if (cmpDmaWait = '1') then
	        		nDstate <= SRun;
	        	end if;
	        end if;
     end case;
   end process;

   DMA_logic: process(pDstate,NDmaRun)
   begin
      BDOutEn <= '0';
      DmaIRW <= '0';
      DMA_Cnt_Rst <= '0';
      DmaACK <= '1';
      case pDstate is
        --RW Idle
        when SIdle =>
		      DMA_Cnt_Rst <= '1';
        when SRun =>
      		DmaACK <= NDmaRun;
		      DMA_Cnt_Rst <= '1';
        when SWR =>
          DmaIRW <= '1';
          BDOutEn <= '1';
      		DmaACK <= '0';
        when SHold =>
      		DmaACK <= '0';
          BDOutEn <= '1';
        when SWait =>
      		DmaACK <= '0';
     end case;
   end process;
end basic;
