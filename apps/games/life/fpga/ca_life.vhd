-- Author:      Michal Kajan
-- Project:     BIN 2007/2008 Game of Life for FITkit
-- Date:        2.5.2008  
-- File:        ca_life.vhd
-- Description: Architecture file. Interconnects Block RAMs and FSM.
--

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;

-- pragsadfma transasdlate_off
library UNISIM;
use UNISIM.vcomponents.all;
-- pragmsadfa tsdfranslate_on

-- ----------------------------------------------------------------------------
--  Architecture declaration: CA_LIFE
-- ----------------------------------------------------------------------------

architecture CA_LIFE_ARCH of CA_LIFE is 

   -- signal declarations -----------------------------------------------------
   -- address signals (from multiplexors)
   signal bram_1_addr_fsm      : std_logic_vector(12 downto 0);
   signal bram_2_addr_fsm      : std_logic_vector(12 downto 0);

   -- signal from multiplexor that selects between component input address
   -- and FSM generated address, used when writing external data to Block RAM
   signal bram_1_addr          : std_logic_vector(12 downto 0) := (others => '0');
   
   -- Block RAM select signal from FSM
   signal bram_sel             : std_logic;

   -- Block RAM write enable signal from FSM
   signal bram_we              : std_logic := '0';

   -- Block RAM output data
   signal bram_1_data_fsm      : std_logic_vector(1 downto 0);
   signal bram_2_data_fsm      : std_logic_vector(1 downto 0);
   
   -- read and write addresses from FSM
   signal bram_read_addr_fsm   : std_logic_vector(12 downto 0);
   signal bram_write_addr_fsm  : std_logic_vector(12 downto 0);

   -- data signals, output from multiplexers
   signal bram_output_data_fsm : std_logic_vector(1 downto 0);
   signal bram_input_data_fsm  : std_logic_vector(1 downto 0) := (others => '0');

   -- data for Block RAM 1, carries either data from FSM or external data
   -- when external write operation is performed
   signal bram_1_data          : std_logic_vector(1 downto 0);

   -- Block RAMs write enable - for FSM purposes
   signal bram_1_we_fsm        : std_logic; 
   signal bram_2_we_fsm        : std_logic;

   -- Block RAM 1 WE signal (PORT A)
   signal bram_1_we            : std_logic;

   -- data signals from B ports of Block RAMs
   signal bram_1_vga_data      : std_logic_vector(1 downto 0);
   signal bram_2_vga_data      : std_logic_vector(1 downto 0);

   -- chip select signals for Block RAM (used for writing data)  
   signal cs_bram_1            : std_logic;
   signal cs_bram_2            : std_logic;

   -- fsm_enable - enable for FSM for controlling Block RAM operations
   -- is set by another FSM, that detects DONE signal and waits for
   -- flag from comparator that 1/4 of second has elapsed
   signal fsm_enable           : std_logic;

   -- signalizes that given matrix has been fully processed
   signal done                 : std_logic;

begin

   -- setting write enable signal for given Block RAM
   bram_1_we_fsm <= cs_bram_1 AND bram_we;
   bram_2_we_fsm <= cs_bram_2 AND bram_we;

   -- address decoder for Block RAMs, uses bram_sel signal
   addr_dec : process(bram_sel)
   begin
      cs_bram_1 <= '0';
      cs_bram_2 <= '0';
      case bram_sel is
         when '1'    => cs_bram_1 <= '1';
         when '0'    => cs_bram_2 <= '1';
         when others => null;
      end case;
   end process addr_dec;


   -- Block Ram instantiation -------------------------------------------------
   -- RAMB16_S2_S2: Virtex-II/II-Pro, Spartan-3/3E 8k x 2 Dual-Port RAM
   BRAM_1 : RAMB16_S2_S2
   port map (
      DOA   => bram_1_data_fsm,                   -- Port A 2-bit Data Output
      DOB   => bram_1_vga_data,               -- Port B 2-bit Data Output
      ADDRA => bram_1_addr,                   -- Port A 13-bit Address Input
      ADDRB => ADDRESS_VGA,                   -- Port B 13-bit Address Input
      CLKA  => CLK,                           -- Port A Clock
      CLKB  => CLK,                           -- Port B Clock
      DIA   => bram_1_data,                   -- Port A 2-bit Data Input
      DIB   => "00",                          -- Port B 2-bit Data Input
      ENA   => '1',                           -- Port A RAM Enable Input
      ENB   => '1',                           -- Port B RAM Enable Input
      SSRA  => RESET,                         -- Port A Synchronous Set/Reset Input
      SSRB  => RESET,                         -- Port B Synchronous Set/Reset Input
      WEA   => bram_1_we,                     -- Port A Write Enable Input
      WEB   => '0'                            -- Port B Write Enable Input
   );

   -- RAMB16_S2_S2: Virtex-II/II-Pro, Spartan-3/3E 8k x 2 Dual-Port RAM
   BRAM_2 : RAMB16_S2_S2
   port map (
      DOA   => bram_2_data_fsm,     -- Port A 2-bit Data Output
      DOB   => bram_2_vga_data,     -- Port B 2-bit Data Output
      ADDRA => bram_2_addr_fsm,     -- Port A 13-bit Address Input
      ADDRB => ADDRESS_VGA,         -- Port B 13-bit Address Input
      CLKA  => CLK,                 -- Port A Clock
      CLKB  => CLK,                 -- Port B Clock
      DIA   => bram_input_data_fsm, -- Port A 2-bit Data Input
      DIB   => "00",                -- Port B 2-bit Data Input
      ENA   => '1',                 -- Port A RAM Enable Input
      ENB   => '1',                 -- Port B RAM Enable Input
      SSRA  => RESET,               -- Port A Synchronous Set/Reset Input
      SSRB  => RESET,               -- Port B Synchronous Set/Reset Input
      WEA   => bram_2_we_fsm,       -- Port A Write Enable Input
      WEB   => '0'                  -- Port B Write Enable Input
   );

   -- End of RAMB16_S2_S2_inst instantiation 

   -- FSM instatiation --------------------------------------------------------
   fsmi : entity work.CA_LIFE_FSM 
      port map (
      -- INPUT PORTS
      -- general signals
      CLK         => CLK,
      RESET       => RESET,

      -- data from given Block RAM
      INPUT_DATA  => bram_output_data_fsm,

      -- write operation to Block is being processed - FSM must detect it
      WE_REQUEST  => WRITE_EN,

      -- connecting signal enabling FSM activity
      FSM_EN      => fsm_enable,

      -- OUTPUT PORTS
      -- address to read data from selected Block RAM
      READ_ADDR   => bram_read_addr_fsm,

      -- address to write data to selected Block RAM
      WRITE_ADDR  => bram_write_addr_fsm,

      -- selection of Block RAM, used as select signal for multiplexors
      BRAM_SELECT => bram_sel,

      -- write enable signal for writing in selected Block RAM
      WRITE_EN    => bram_we,

      -- signalizes that given matrix has been fully processed
      DONE        => done,

      -- data to be sent to selected Block RAM
      OUTPUT_DATA => bram_input_data_fsm

      );

   fsmi_en_gen : entity work.FSM_EN_GEN
      port map(
      -- general input
         CLK       => CLK,
         RESET     => RESET,

         -- signal from FSM controlling memory operations that whole matrix
         -- has been processed
         DONE_IN   => done,

         -- global enable input from toplevel component
         GLOBAL_EN => FSM_EN,

         -- signalizes elapsing 1/4 of second
         TIME_FLAG => TIME_FLAG,

         -- enable output for FSM controlling memory operations
         EN_FSM    => fsm_enable         
      );
 

   -- this multiplexor selects between FSM WE signal and interface WRITE_EN
   -- signal
   mux_we_select : process(WRITE_EN, bram_we)
   begin
      case(WRITE_EN) is
         -- FSM WE will be used
         when '0'    =>
            bram_1_we   <= bram_1_we_fsm;
         -- '1' will be used
         when '1'    =>
            bram_1_we   <= '1';
         when others =>
            null;
      end case;
   end process mux_we_select;


   -- this multiplexor is used to select between FSM address signals
   -- and from entity address signals and is used when write request
   -- from MCU is set
   bram_1_top_addr_mux : process(WRITE_EN, ADDRESS_WR, bram_1_addr_fsm)
   begin
      case(WRITE_EN) is
         -- signal from FSM will be used
         when '0'    =>
            bram_1_addr <= bram_1_addr_fsm; 
         -- signal from interface will be used
         when '1'    =>
            bram_1_addr <= ADDRESS_WR;
         when others =>
            null;
       end case;
    end process bram_1_top_addr_mux;

   -- this multiplexor is used to select between FSM data signals
   -- and from entity data signals and is used when write request from
   -- MCU is set
   bram_1_input_mux : process(WRITE_EN, DATA_IN, bram_input_data_fsm)
   begin
      case(WRITE_EN) is
         -- signal from FSM will be used
         when '0'    =>
            bram_1_data <= bram_input_data_fsm;
         -- signal from interface will be used
         when '1'    =>
            bram_1_data <= DATA_IN;
         when others =>
            null;
      end case;      
   end process bram_1_input_mux;


   -- Block RAM 1 address select (PORT A) - address from FSM
   bram_1_addr_mux : process(bram_read_addr_fsm, bram_write_addr_fsm, bram_sel)   
   begin
      case(bram_sel) is 
         when '0'    => 
            bram_1_addr_fsm <= bram_read_addr_fsm;
         when '1'    =>
            bram_1_addr_fsm <= bram_write_addr_fsm;
         when others =>
            null;
      end case;
   end process bram_1_addr_mux;

   -- Block RAM 2 address select (PORT A) - address from FSM
   bram_2_addr_mux : process(bram_read_addr_fsm, bram_write_addr_fsm, bram_sel)   
   begin
      case(bram_sel) is 
         when '0'    => 
            bram_2_addr_fsm <= bram_write_addr_fsm;
         when '1'    =>
            bram_2_addr_fsm <= bram_read_addr_fsm;
         when others =>
            null;
      end case;
   end process bram_2_addr_mux;

   -- Block RAM 1 and 2 data output select (PORT A) - data for FSM
   bram_data_out_mux : process(bram_1_data_fsm, bram_2_data_fsm, bram_sel)   
   begin
      case(bram_sel) is 
         when '0'    => 
            bram_output_data_fsm <= bram_1_data_fsm;
         when '1'    =>
            bram_output_data_fsm <= bram_2_data_fsm;
         when others =>
            null;
      end case;
   end process bram_data_out_mux;

   -- data output from B ports of Block RAMs must be multiplexed
   -- this is data for VGA controller  
   bram_vga_out_mux : process(bram_1_vga_data, bram_2_vga_data, bram_sel)
   begin
      case(bram_sel) is 
         when '0'    =>
            DATA_OUT <= bram_1_vga_data;
         when '1'    =>
            DATA_OUT <= bram_2_vga_data;
         when others =>
            null;
      end case;
   end process bram_vga_out_mux;

end architecture CA_LIFE_ARCH; 
