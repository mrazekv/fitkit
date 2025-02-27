-- video player
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

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;
use work.sdram_controller_cfg.all;

architecture arch_ide_test of tlv_ide_ifc is
   signal key_data_in   : std_logic_vector (15 downto 0);

   signal ide_bus        : std_logic_vector(25 downto 0) := (others => 'Z');
   signal CLK_divided    : std_logic_vector(2 downto 0);
   signal req_row        : std_logic_vector(11 downto 0);
   signal req_col        : std_logic_vector(11 downto 0);
   signal isettings      : std_logic_vector(60 downto 0);
   signal color          : std_logic_vector(8 downto 0) := (others => '0');
   signal vga_hstate     : vga_hfsm_state;
   signal vga_vstate     : vga_vfsm_state;
   signal lcolor         : std_logic_vector(8 downto 0) := (others => '0');

   signal vga_conf       : std_logic_vector(7 downto 0) := (others => '0');
   signal vga_max_row    : std_logic_vector(7 downto 0) := (others => '0');
   signal vga_max_col    : std_logic_vector(7 downto 0) := (others => '0');
   signal vga_conf_out   : std_logic_vector(7 downto 0) := (others => '0');
   signal vga_conf_port  : std_logic_vector(1 downto 0) := (others => '0');
   signal vga_conf_WE    : std_logic := '0';

   type   line_states is (sStart,sRefresh,sWLoad,sLoad,sDelay,sRow,sLine);
   signal line_state     : line_states := sStart; -- actual state
   signal line_nstate    : line_states := sStart; -- next state
   signal line_col_rst   : std_logic := '0';
   signal line_row_rst   : std_logic := '0';
   signal line_row_inc   : std_logic := '0';
   signal vga_bank_set   : std_logic := '0';
   signal cmp_vga_col    : std_logic := '0';
   signal cmp_vga_ccol   : std_logic := '0';
   signal cmp_vga_row    : std_logic := '0';
   signal vga_row        : std_logic_vector(8 downto 0) := (others => '0');
   signal vga_col        : std_logic_vector(9 downto 0) := (others => '0');
   signal line_buff_addr : std_logic_vector(10 downto 0) := (others => '0');
   signal line_buff_in_addr : std_logic_vector(10 downto 0) := (others => '0');
   signal vga_bank       : std_logic_vector(3 downto 0) := (others => '0');
   signal ram_vga_we     : std_logic := '0';
--   signal tim_RD       : std_logic_vector(2 downto 0);
--   signal free_buff    : std_logic;
   signal vga_ccol       : std_logic_vector(9 downto 0) := (others => '0');
   signal req_c          : std_logic_vector(9 downto 0) := (others => '0');
   signal out_size       : std_logic_vector(1 downto 0) := (others => '0');
   signal req_r          : std_logic := '0';

   signal vga_RD         : std_logic := '0';
   signal vga_RQ         : std_logic := '0';
   signal vga_granted    : std_logic := '0';
   signal SD_DMA_RQ      : std_logic := '0';
   signal SD_DMA_granted : std_logic := '0';
   signal reserved       : std_logic := '0';

   signal SD_DMA_WE      : std_logic := '0';
   signal SD_DMA_WRQ     : std_logic := '0';
   signal SD_DMA_out     : std_logic_vector(23 downto 0) := (others => '0');
   signal SD_DMA_out_buff : std_logic_vector(7 downto 0) := (others => '0');
   signal SD_DMA_addr_buff : std_logic_vector(22 downto 0) := (others => '0');

   signal sdram_ra       : std_logic_vector(13 downto 0);
   signal sdram_busy     : std_logic;
   signal Nsdram_busy    : std_logic;
   signal sdram_vld      : std_logic;
   signal sdram_read     : std_logic;
   signal sdram_write    : std_logic;
   signal sdram_addr     : std_logic_vector(22 downto 0);
   signal sdram_data_out : std_logic_vector(7 downto 0);
   signal sdram_data_in  : std_logic_vector(7 downto 0);
   signal sdr_req        : std_logic_vector(2 downto 0);
   signal sdram_ref      : std_logic;
   signal sdram_we       : std_logic;
   signal sdram_cmd      : sdram_func;
   signal ref_cnt        : std_logic_vector(11 downto 0);

	 
	 component IDE_port
	    port (
	       IDE : inout std_logic_vector(25 downto 0) := (others => 'Z');
	       X   : inout std_logic_vector(45 downto 0) := (others => 'Z')
	    );
	 end component;

   for ide_p: IDE_port use entity work.IDE_port(small);

   component IDE_ctrl
      generic (
         CLK_PERIOD  : integer
      );
   port (
      -- Reset a synchronizace
      IDE     : inout std_logic_vector(25 downto 0) := (others => 'Z');
      CLK     : in std_logic; -- Hodiny radice (min. 4x vetsi, nez hodiny SPI)
      RST			:	in std_logic; -- RESET radice
			-- DMA kanal
      READ_EN :	out std_logic;	-- Data z radice jsou pripraveny
      WRITE_EN:	out std_logic;	-- Radic pripraven prijimat data
      ACCEPT  :	in std_logic;	-- Data prijata/platna data na vstupu
      DATA_IN : in std_logic_vector(7 downto 0) := (others => '0');
      DATA_OUT: out std_logic_vector(7 downto 0) := (others => '0');  

      -- Interni rozhrani
      DI      : out std_logic;   -- Serial data out (MOSI)
      DI_REQ  : in std_logic;   -- DI request
      DO      : in std_logic;   -- Serial data in (MISO)
      DO_VLD  : in std_logic;   -- DO valid
      CS      : in std_logic    -- internal SPI active
   );
   end component;

   component keyboard_controller
      port(
         CLK      : in std_logic;
         RST      : in std_logic;

         DATA_OUT : out std_logic_vector(15 downto 0);
         DATA_VLD : out std_logic;

         KB_KIN   : out std_logic_vector(3 downto 0);
         KB_KOUT  : in  std_logic_vector(3 downto 0)
      );
   end component;
	 
	 component vga_controller
	   generic (
	      REQ_DELAY : natural := 1
	      );
	   port (
	      -- Hodiny + reset
	      CLK     : in std_logic;
	      RST     : in std_logic;
	      ENABLE  : in std_logic;
	
	      -- Nastaveni rozliseni (popis v VGA_CONFIG.VHD)
	      -- Pozor: Hodnota musi zustat stale platna!
	      MODE    : in std_logic_vector(60 downto 0);
	
	      -- Vstup / pozadavek
	      DATA_RED    : in  std_logic_vector(2 downto 0);
	      DATA_GREEN  : in  std_logic_vector(2 downto 0);
	      DATA_BLUE   : in  std_logic_vector(2 downto 0);
	      ADDR_COLUMN : out std_logic_vector(11 downto 0);
	      ADDR_ROW    : out std_logic_vector(11 downto 0);
	
	      -- Vystup na VGA
	      VGA_RED   : out std_logic_vector(2 downto 0);
	      VGA_GREEN : out std_logic_vector(2 downto 0);
	      VGA_BLUE  : out std_logic_vector(2 downto 0);
	      VGA_HSYNC : out std_logic;
	      VGA_VSYNC : out std_logic;
	      
	      -- H/V Status
	      STATUS_H  : out vga_hfsm_state;
	      STATUS_V  : out vga_vfsm_state     
	      );
   end component;

   component RAMB16_S9_S9
      port (
         DOA   : out std_logic_vector(7 downto 0);
         DOB   : out std_logic_vector(7 downto 0);
         DOPA   : out std_logic_vector(0 downto 0);
         DOPB   : out std_logic_vector(0 downto 0);
         ADDRA : in std_logic_vector(10 downto 0);
         ADDRB : in std_logic_vector(10 downto 0);
         CLKA  : in std_logic;
         CLKB  : in std_logic;
         DIA   : in std_logic_vector(7 downto 0);
         DIB   : in std_logic_vector(7 downto 0);
         DIPA   : in std_logic_vector(0 downto 0);
         DIPB   : in std_logic_vector(0 downto 0);
         ENA   : in std_logic;
         ENB   : in std_logic;
         SSRA  : in std_logic;
         SSRB  : in std_logic;
         WEA   : in std_logic;
         WEB   : in std_logic
      );
   end component;

   component sdram_raw_controller
      generic (
         -- Generovani prikazu refresh radicem automaticky
         GEN_AUTO_REFRESH : boolean := false;
	       OPTIMIZE_REFRESH : sdram_optimize := oAlone
      );
      port (
         -- Hodiny, reset, ...
         CLK     : in std_logic;
         RST     : in std_logic;
         ENABLE  : in std_logic;
         BUSY    : out std_logic;
         
         -- Adresa/data
         ADDR_ROW    : in std_logic_vector(11 downto 0);
         ADDR_COLUMN : in std_logic_vector(8 downto 0);
         BANK        : in std_logic_vector(1 downto 0);
         DATA_IN     : in std_logic_vector(7 downto 0);
         DATA_OUT    : out std_logic_vector(7 downto 0);
         DATA_VLD    : out std_logic; -- Output data valid
   
         -- Pozadavek + jeho potvrzeni
         CMD     : in sdram_func;
         CMD_WE  : in std_logic;
   
         -- SDRAM
         RAM_A      : out std_logic_vector(13 downto 0);
         RAM_D      : inout std_logic_vector(7 downto 0);
         RAM_DQM    : out std_logic;
         RAM_CS     : out std_logic;
         RAM_RAS    : out std_logic;
         RAM_CAS    : out std_logic;
         RAM_WE     : out std_logic;
         RAM_CLK    : out std_logic;
         RAM_CKE    : out std_logic
         ); 
   end component;

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
   for SPI_adc_SD_DMA: SPI_adc use entity work.SPI_adc(autoincrement);

 begin

   SetMode(r640x480x60, isettings);

   ide_p: IDE_port
      port map(
         IDE  => ide_bus,
         X  => X(45 downto 0)
      );

   ide_c: IDE_ctrl
      generic map (
         CLK_PERIOD  => 20 --100 MHz - 10 ns 50 MHz = 20 ns
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,
	       RST			=> RESET,
	       READ_EN	=> SD_DMA_RQ,
	       WRITE_EN	=> open,
	       ACCEPT  	=> SD_DMA_WRQ,
	       DATA_IN	=> "00000000",
	       DATA_OUT	=> SD_DMA_out_buff,

         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
         IDE  => ide_bus
      );

	vga: vga_controller
	   generic map(
	      REQ_DELAY => 0
	      )
	   port map(
	      -- Hodiny + reset
	      CLK     => CLK_divided(0),
	      RST     => RESET,
	      ENABLE  => vga_conf(7),
	      MODE    => isettings,
	
	      DATA_RED    => color(8 downto 6),
	      DATA_GREEN  => color(5 downto 3),
	      DATA_BLUE   => color(2 downto 0),
	      ADDR_COLUMN => req_col,
	      ADDR_ROW    => req_row,
	
	      VGA_RED   => RED_V,
	      VGA_GREEN => GREEN_V,
	      VGA_BLUE  => BLUE_V,
	      VGA_HSYNC => HSYNC_V,
	      VGA_VSYNC => VSYNC_V,
	      
	      STATUS_H  => vga_hstate,
	      STATUS_V  => vga_vstate    
	      );

   -- 50 Mhz / 2 => 25 MHz

   div_clk: process(CLK)
   begin
      if CLK'event and CLK = '1' then
         CLK_divided <= CLK_divided + 1;
      end if;
   end process;
   
   vga_bank_buff: process(CLK, RESET)
   begin
   		if RESET='1' then
   			vga_bank <= "0000";
      elsif CLK'event and CLK = '1' and vga_bank_set='1' then
         vga_bank <= vga_conf(3 downto 0);
      end if;
   end process;

   SPI_adc_vga: SPI_adc
      generic map(
         ADDR_WIDTH => 8,     
         DATA_WIDTH => 8,     
         ADDR_OUT_WIDTH => 2, 
         BASE_ADDR  => 16#40#,
         DELAY  => 0
      )
      port map(
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => vga_conf_port,
         DATA_IN(7 downto 2)  => "000000",
         DATA_IN(1)  => SD_DMA_granted,
         DATA_IN(0)  => vga_granted,
         READ_EN  => open,

         DATA_OUT => vga_conf_out,
         WRITE_EN => vga_conf_WE
      );

   SPI_adc_SD_DMA: SPI_adc
      generic map(
         ADDR_WIDTH => 8,     
         DATA_WIDTH => 24,     
         ADDR_OUT_WIDTH => 1, 
         BASE_ADDR  => 16#80#,
         DELAY  => 0
      )
      port map(
         CLK      => CLK,

         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,

         ADDR     => open,
         DATA_IN(23)  => '0',
         DATA_IN(22 downto 0)  => SD_DMA_addr_buff,
         READ_EN  => open,

         DATA_OUT => SD_DMA_out,
         WRITE_EN => SD_DMA_WE
      );

	 SD_DMA_WRQ <= SD_DMA_granted and (not sdram_busy);

   sd_dma_addr_b: process(CLK, RESET, SD_DMA_WE)
   begin
   		if RESET='1' then
   			SD_DMA_addr_buff <= (others => '0');
      elsif CLK'event and CLK = '1' then
      	if SD_DMA_WE='1' then
        	SD_DMA_addr_buff <= SD_DMA_out(22 downto 0);
        elsif SD_DMA_WRQ='1' then
        	SD_DMA_addr_buff <= SD_DMA_addr_buff + 1;
        end if;
      end if;
   end process;
   
   vga_conf_buff: process(CLK, RESET,vga_conf_WE,vga_conf_port)
   begin
   		if RESET='1' then
   			vga_conf <= (others => '0');
      elsif CLK'event and CLK = '1' and vga_conf_WE='1' and vga_conf_port="00" then
         vga_conf <= vga_conf_out;
      end if;
   end process;

   vga_conf_max_col_buff: process(CLK, RESET,vga_conf_WE,vga_conf_port)
   begin
   		if RESET='1' then
   			vga_max_col <= (others => '0');
      elsif CLK'event and CLK = '1' and vga_conf_WE='1' and vga_conf_port="01" then
         vga_max_col <= vga_conf_out;
      end if;
   end process;

   vga_conf_max_row_buff: process(CLK, RESET,vga_conf_WE,vga_conf_port)
   begin
   		if RESET='1' then
   			vga_max_row <= (others => '0');
      elsif CLK'event and CLK = '1' and vga_conf_WE='1' and vga_conf_port="10" then
         vga_max_row <= vga_conf_out;
      end if;
   end process;
   vga_conf_size: process(CLK, RESET,vga_conf_WE,vga_conf_port)
   begin
   		if RESET='1' then
   			out_size <= (others => '0');
      elsif CLK'event and CLK = '1' and vga_conf_WE='1' and vga_conf_port="11" then
         out_size <= vga_conf_out(1 downto 0);
      end if;
   end process;

   vga_col_cnt: process(CLK, line_col_rst,vga_RD)
   begin
   		if line_col_rst='1' then
   			vga_col <= (others => '0');
      elsif CLK'event and CLK = '1' and vga_RD='1' then
         vga_col <= vga_col + 1;
      end if;
   end process;

   vga_ccol_cnt: process(CLK, line_col_rst,ram_vga_we)
   begin
   		if line_col_rst='1' then
   			vga_ccol <= (others => '0');
      elsif CLK'event and CLK = '1' and ram_vga_we='1' then
         vga_ccol <= vga_ccol + 1;
      end if;
   end process;

   vga_row_cnt: process(line_row_rst,CLK,line_row_inc)
   begin
   		if line_row_rst='1' then
   			vga_row <= (others => '0');
      elsif CLK'event and CLK = '1' and line_row_inc='1' then
         vga_row <= vga_row + 1;
      end if;
   end process;
   
   cmp_vga_col <= '1' when (vga_col(9 downto 3)=vga_max_col(6 downto 0)) else '0';
   cmp_vga_ccol <= '1' when (vga_ccol(9 downto 3)=vga_max_col(6 downto 0)) else '0';
   cmp_vga_row <= '1' when (vga_row(8 downto 3)=vga_max_row(5 downto 0)) else '0';
   Nsdram_busy <= '0' when (sdram_busy='1') else '1';

   line_tick: process(CLK, RESET)
   begin
   		if RESET='1' then
   			line_state <= sStart;
      elsif CLK'event and CLK = '1' then
         line_state <= line_nstate;
      end if;
   end process;

--   vld_tick: process(CLK, RESET)
--   begin
--   		if RESET='1' then
--   			tim_RD <= "000";
--      elsif CLK'event and CLK = '1' then
--         tim_RD <= tim_RD(1 downto 0) & vga_RD;
--      end if;
--   end process;

   ref_cnt_tick: process(CLK, sdram_ref)
   begin
   		if sdram_ref='0' then
   			ref_cnt <= "000000000000";
      elsif CLK'event and CLK = '1' then
         ref_cnt <= ref_cnt + 1;
      end if;
   end process;

--   sdram_vld <= tim_RD(2);
--   free_buff <= '0' when tim_RD(0)='1' or tim_RD(1)='1' or tim_RD(2)='1' else '1';  

   line_FSM: process(line_state,sdram_busy,vga_granted,cmp_vga_ccol,cmp_vga_col,req_row,vga_row,vga_vstate,Nsdram_busy, cmp_vga_row)
   begin
		  line_col_rst <= '0';
		  line_row_rst <= '0';
   		line_row_inc <= '0';
		  vga_RD <= '0';
		  vga_RQ <= '0';
		  vga_bank_set <= '0';
		  sdram_ref <= '0';
      case line_state is
        when sStart =>
		   		line_nstate <= sStart;
		   		line_col_rst <= '1';
		   		line_row_rst <= '1';
		  		vga_bank_set <= '1';
        	if (vga_vstate=vstSync) then
	       		line_nstate <= sWLoad;
	       	end if;
        when sRefresh =>
		   		line_nstate <= sRefresh;
		  		vga_RQ <= '1';
		  		sdram_ref <= '1';
        	if (ref_cnt(11)='1') then
	       		line_nstate <= sStart;
	       	end if;
        when sWLoad =>
		   		line_nstate <= sWLoad;
		  		vga_RQ <= '1';
        	if (vga_granted='1') then
	       		line_nstate <= sLoad;
	       	end if;
        when sLoad =>
		   		line_nstate <= sLoad;
		  		vga_RQ <= '1';
		  		vga_RD <= Nsdram_busy;
        	if (cmp_vga_col = '1') then
	       		line_nstate <= sDelay;
        	end if;
        when sDelay =>
		  		vga_RQ <= '1';
		   		line_nstate <= sDelay;
        	if (cmp_vga_ccol = '1') then
	       		line_nstate <= sRow;
        	end if;
        when sRow =>
		  		vga_RQ <= '1';
		   		line_row_inc <= '1';
		   		line_col_rst <= '1';
		   		line_nstate <= sLine;
        	if (cmp_vga_row='1') then
	       		line_nstate <= sRefresh;
        	end if;
        when sLine =>
		   		line_nstate <= sLine;
        	if (req_r/=vga_row(0)) then
	       		line_nstate <= sWLoad;
        	end if;
     end case;
   end process;
   
   req_r <= req_row(0) when out_size="00" else req_row(1) when out_size="01" else 
	   req_row(2) when out_size="10" else req_row(3);
   req_c <= req_col(9 downto 0) when out_size="00" else "0" & req_col(9 downto 1) when out_size="01" else 
	   "00" & req_col(9 downto 2) when out_size="10" else "000" & req_col(9 downto 3);
   line_buff_addr(10 downto 0) <= req_r & req_c;
   line_buff_in_addr(10 downto 0) <= vga_row(0) & vga_ccol(9 downto 0);
   ram_vga_we <= sdram_vld and vga_granted;

   color <= lcolor;-- when req_col(9 downto 0)/="0000000000" else "000000000";-- when req_row(8 downto 3)<vga_max_row(5 downto 0) and req_col(9 downto 3)<vga_max_col(6 downto 0) else "000000000";
   line_buff: RAMB16_S9_S9
      port map(
         DOA   => open,
         DOB   => lcolor(8 downto 1),
         DOPA  => open,
         DOPB  => lcolor(0 downto 0),
         ADDRA => line_buff_in_addr(10 downto 0),
         ADDRB => line_buff_addr(10 downto 0),
         CLKA  => CLK,
         CLKB  => CLK,
         DIA   => sdram_data_out(7 downto 0),
         DIB   => "00000000",
         DIPA  => sdram_data_out(2 downto 2), -- posledni kopiruje nejvyssi
         DIPB  => "0",
         ENA   => '1',
         ENB   => '1',
         SSRA  => '0',
         SSRB  => '0',
         WEA   => ram_vga_we,
         WEB   => '0'
      );
   
   sdram_addr <= vga_bank(3 downto 0) & vga_row(8 downto 0) & vga_col(9 downto 0)
	 when vga_granted='1' else SD_DMA_addr_buff when SD_DMA_granted='1' else "00000000000000000000000";
   sdram_data_in <= SD_DMA_out_buff when SD_DMA_granted='1' else "00000000";
	
	 sdram_write <= SD_DMA_WRQ;
	 sdram_read <= vga_RD and vga_granted;
	 
	 reserved <= (vga_granted and vga_RQ) or (SD_DMA_granted and SD_DMA_RQ);

   grant_system: process(CLK,vga_RQ,SD_DMA_RQ,reserved)
   begin
   		if CLK'event and CLK = '1' then
   			vga_granted <= vga_RQ and ((not reserved) or vga_granted);
   			SD_DMA_granted <= SD_DMA_RQ and ((not vga_RQ) or SD_DMA_granted);
   		end if;
   end process;
	 
   RA <= sdram_ra(13 downto 12) & '0' & sdram_ra(11 downto 0);
   sdram: sdram_raw_controller
      generic map (
         -- Generovani prikazu refresh radicem automaticky
         GEN_AUTO_REFRESH => false,
         OPTIMIZE_REFRESH => oMultiple
      )
      port map (
         -- Adresa
         ADDR_ROW    => sdram_addr(20 downto 9),
         ADDR_COLUMN => sdram_addr(8 downto 0),
         BANK        => sdram_addr(22 downto 21),
   
         -- Pozadavek + jeho potvrzeni
         CMD      => sdram_cmd,
         CMD_WE   => sdram_we,

         -- Hodiny, reset, ...
         CLK      => CLK,
         RST      => RESET,
         ENABLE   => '1',
         BUSY     => sdram_busy,
   
         -- Data
         DATA_OUT    => sdram_data_out,
         DATA_VLD    => sdram_vld,
         DATA_IN     => sdram_data_in,

         -- SDRAM
         RAM_A       => sdram_ra,
         RAM_D       => RD,
         RAM_DQM     => RDQM,
         RAM_CS      => RCS,
         RAM_RAS     => RRAS,
         RAM_CAS     => RCAS,
         RAM_WE      => RWE,
         RAM_CLK     => RCLK,
         RAM_CKE     => RCKE
      );

   sdram_we <= sdram_read or sdram_write or sdram_ref;
   sdr_req <= sdram_read & sdram_write & sdram_ref;
   with sdr_req select
      sdram_cmd <= fRead    when "100",
                   fWrite   when "010",
                   fRefresh when "001",
                   fNop when others;

   SPI_adc_keybrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu min. 1 bit
         BASE_ADDR  => 16#20#,  -- adresovy prostor od 0x0002
         DELAY  => 0
      )
      port map(
         CLK   =>  CLK,
   
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
   
         ADDR     => open,
         DATA_OUT => open,
         DATA_IN  => key_data_in,
         WRITE_EN => open,
         READ_EN  => open
      );
     
   -- Radic klavesnice
   keybrd: keyboard_controller
      port map(
         CLK      => CLK_divided(1),
         RST      => RESET,       -- reset
         
         DATA_OUT => key_data_in,         
         DATA_VLD => open,

         -- Keyboart
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

end arch_ide_test;
