-- top_level.vhd : infrared communication
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Michal Růžek <xruzek01 AT stud.fit.vutbr.cz>
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
use ieee.std_logic_1164.ALL;
use ieee.std_logic_ARITH.ALL;
use ieee.std_logic_UNSIGNED.ALL;



architecture main of tlv_gp_ifc is

   --signaly
   signal mux_out      : std_logic_vector(23 downto 0);
   signal mux_in0      : std_logic_vector(23 downto 0);
   signal mux_in1      : std_logic_vector(23 downto 0);
   signal mux_in2      : std_logic_vector(23 downto 0);
   signal data_bus     : std_logic_vector(23 downto 0); 
   signal addr         : std_logic_vector(2 downto 0);
   signal addr_r       : std_logic_vector(2 downto 0);   
   signal write_en     : std_logic;
   signal write_en_sl  : std_logic;
   signal rc5_wr       : std_logic;
   signal nec_wr       : std_logic;
   signal nec_rpt      : std_logic;
   signal sirc12_wr    : std_logic;
   signal sirc15_wr    : std_logic;
   signal sirc20_wr    : std_logic;
   signal rc5_busy     : std_logic;
   signal nec_busy     : std_logic;
   signal sirc_busy    : std_logic; 
   signal rc5_ir       : std_logic;
   signal nec_ir       : std_logic;
   signal sirc_ir      : std_logic;    
   signal ir_recv      : std_logic;  
   signal rc5_vld      : std_logic;
   signal nec_vld      : std_logic;
   signal sirc12_vld   : std_logic;
   signal sirc15_vld   : std_logic;
   signal sirc20_vld   : std_logic;
   signal interrupts   : std_logic_vector(7 downto 0);
   signal irq_in       : std_logic_vector(7 downto 0);
   signal irq_out      : std_logic_vector(7 downto 0);
   signal irq_rd       : std_logic;
   signal irq_wr       : std_logic;
   signal total_irq    : std_logic;
   signal reg_a        : std_logic;
   signal reg_b        : std_logic;
   signal key_data_in  : std_logic_vector (15 downto 0);
   signal dmx_in       : std_logic_vector(3 downto 0);
   signal dmx_out      : std_logic_vector(5 downto 0);
   signal dis_data_out : std_logic_vector(15 downto 0);
   signal dis_write_en : std_logic;  
   
   
   --prevod signalu mezi ruzne rychlymi CLK domenami + registr adresy
   component clk_sync
      port ( 
      RESET     : in std_logic; 
      CLK_FAST  : in std_logic; 
      CLK_SLOW  : in std_logic; 

      FAST_IN   : in std_logic;
      SLOW_OUT  : out std_logic;
      
      ADDR_IN   : in std_logic_vector(2 downto 0);
      ADDR_OUT  : out std_logic_vector(2 downto 0)      
      );
   end component;    


   --adresovy SPI dekoder
   component SPI_adc
      generic (
         ADDR_WIDTH : integer;
         DATA_WIDTH : integer;
         ADDR_OUT_WIDTH : integer;
         BASE_ADDR  : integer
      );
      port (
         CLK      : in  std_logic;
         CS       : in  std_logic;
         DO       : in  std_logic;
         DO_VLD   : in  std_logic;
         DI       : out std_logic;
         DI_REQ   : in  std_logic;
         ADDR     : out  std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
         DATA_OUT : out  std_logic_vector (DATA_WIDTH-1 downto 0);
         DATA_IN  : in   std_logic_vector (DATA_WIDTH-1 downto 0);
         WRITE_EN : out  std_logic;
         READ_EN  : out  std_logic
      );
   end component;
   
   
   -- radic displeje
   component lcd_raw_controller
      port (
         RST      : in std_logic;
         CLK      : in std_logic;
         DATA_IN  : in std_logic_vector (15 downto 0);
         WRITE_EN : in std_logic;
         DISPLAY_RS   : out   std_logic;
         DISPLAY_DATA : inout std_logic_vector(7 downto 0);
         DISPLAY_RW   : out   std_logic;
         DISPLAY_EN   : out   std_logic
      );
   end component;
      
   
   --dekoder prokolu RC5x
   component ir_RC5
      port (
         RST      : in std_logic;
         CLK      : in std_logic;
         IR_IN    : in std_logic;
         IR_OUT   : out std_logic;
         DATA_IN  : in std_logic_vector(23 downto 0);
         WRITE_EN : in std_logic;
         BUSY     : out std_logic;
         DATA_OUT : out std_logic_vector(23 downto 0); 
         DATA_VLD : out std_logic
      );
   end component;  
   
   --dekoder prokolu NEC
   component ir_NEC
      port (
         RST       : in std_logic;
         CLK       : in std_logic;
         IR_IN     : in std_logic;
         IR_OUT    : out std_logic;
         DATA_IN   : in std_logic_vector(23 downto 0);
         WRITE_EN  : in std_logic;
         WR_REPEAT : in std_logic;
         BUSY      : out std_logic;
         DATA_OUT  : out std_logic_vector(23 downto 0); 
         DATA_VLD  : out std_logic
      );
   end component;  
     
   --dekoder prokolu SIRC
   component ir_SIRC
      port (
         RST         : in std_logic;
         CLK         : in std_logic;
         IR_IN       : in std_logic;
         IR_OUT      : out std_logic;
         DATA_IN     : in std_logic_vector(23 downto 0);
         WRITE_12_EN : in std_logic;
         WRITE_15_EN : in std_logic;
         WRITE_20_EN : in std_logic;
         BUSY        : out std_logic;
         DATA_OUT    : out std_logic_vector(23 downto 0); 
         DATA_12_VLD : out std_logic;
         DATA_15_VLD : out std_logic;
         DATA_20_VLD : out std_logic
      );
   end component;         
   
   --radic preruseni
   component interrupt_controller
      generic (
         WIDTH : integer := 8
      );
      port(
         CLK      : in  std_logic;
         RST      : in  std_logic;
         IRQ_IN   : in std_logic_vector(WIDTH-1 downto 0);
         IRQ_OUT  : out std_logic;
         DATA_IN  : in std_logic_vector(WIDTH-1 downto 0);
         WRITE_EN : in std_logic;
         DATA_OUT : out std_logic_vector(WIDTH-1 downto 0);
         READ_EN  : in std_logic
         );
   end component;
   
   -- klavesnice 4x4
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

begin

   -- SPI dekoder pro displej
   spidecd: SPI_adc
      generic map (
         ADDR_WIDTH => 8,     -- sirka adresy 8 bitu
         DATA_WIDTH => 16,    -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 1, -- sirka adresy na vystupu 1 bit
         BASE_ADDR  => 16#00# -- adresovy prostor od 0x00-0x01
      )
      port map (
         CLK      => CLK,
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
         ADDR     => open,
         DATA_OUT => dis_data_out,
         DATA_IN  => "0000000000000000",
         WRITE_EN => dis_write_en,
         READ_EN  => open
         );

   -- radic LCD displeje
   lcdctrl: lcd_raw_controller
      port map (
         RST    =>  RESET,
         CLK    =>  CLK,
         DATA_IN  => dis_data_out,
         WRITE_EN => dis_write_en,
         DISPLAY_RS   => LRS,
         DISPLAY_DATA => LD,
         DISPLAY_RW   => LRW,
         DISPLAY_EN   => LE
      );

   --SPI dekoder pro komunikaci radicu protokolu s MCU
   SPI_adc_ir: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 24,      -- sirka dat 24 bitu
         ADDR_OUT_WIDTH => 3,   -- sirka adresy na vystupu 3 bit
         BASE_ADDR  => 16#0090# -- adresovy prostor od 0x0090
      )
      port map(
         CLK      =>  CLK,
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
         ADDR     => addr,
         DATA_IN  => mux_out,
         READ_EN  => open,         
         WRITE_EN => write_en,
         DATA_OUT => data_bus
      );
                
      
   --radic protokolu RC5
   rc5: ir_RC5 
      port map(
         RST      => RESET,
         CLK      => SMCLK,
         IR_IN    => ir_recv,
         IR_OUT   => rc5_ir,
         BUSY     => rc5_busy,
         DATA_IN  => data_bus,     
         WRITE_EN => rc5_wr,
         DATA_OUT => mux_in0,
         DATA_VLD => rc5_vld
      );  
     
      
   --radic protokolu NEC
   nec: ir_NEC 
      port map(
         RST      => RESET,
         CLK      => SMCLK,
         IR_IN    => ir_recv,
         IR_OUT   => nec_ir,
         BUSY     => nec_busy,
         DATA_IN  => data_bus,     
         WRITE_EN => nec_wr,
         WR_REPEAT => nec_rpt,
         DATA_OUT => mux_in1,
         DATA_VLD => nec_vld
      ); 
      
      
   --radic protokolu SIRC
   sirc: ir_SIRC 
      port map(
         RST         => RESET,
         CLK         => SMCLK,
         IR_IN       => ir_recv,
         IR_OUT      => sirc_ir,
         BUSY        => sirc_busy,
         DATA_IN     => data_bus,     
         WRITE_12_EN => sirc12_wr,
         WRITE_15_EN => sirc15_wr,
         WRITE_20_EN => sirc20_wr,
         DATA_OUT    => mux_in2,
         DATA_12_VLD => sirc12_vld,
         DATA_15_VLD => sirc15_vld,
         DATA_20_VLD => sirc20_vld
      );            
            

   --prevod zapisoveho signalu z FAST na SLOW  + registr ADRESY ADDR      
   en_sync: clk_sync 
      port map(
        RESET => RESET,
        CLK_FAST => CLK,
        CLK_SLOW => SMCLK,
        FAST_IN  => write_en,
        SLOW_OUT => write_en_sl,
        ADDR_IN  => addr,
        ADDR_OUT => addr_r        
      ); 
  
      
   -- SPI dekoder  pro klavesnici
   SPI_adc_keybrd: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 16,      -- sirka dat 16 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu 1 (nejde mensi)
         BASE_ADDR  => 16#0002# -- adresovy prostor od 0x0002
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
         CLK      => CLK,
         RST      => RESET,       
         DATA_OUT => key_data_in,         
         DATA_VLD => open,
         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );       
      

  --SPI dekoder pro radic preruseni  
   SPI_adc_irq: SPI_adc
      generic map(
         ADDR_WIDTH => 8,       -- sirka adresy 8 bitu
         DATA_WIDTH => 8,       -- sirka dat 8 bitu
         ADDR_OUT_WIDTH => 1,   -- sirka adresy na vystupu 1 (nejde mensi)
         BASE_ADDR  => 16#0080# -- adresovy prostor od 0x0080
      )
      port map(
         CLK      => CLK,
         CS       => SPI_CS,
         DO       => SPI_DO,
         DO_VLD   => SPI_DO_VLD,
         DI       => SPI_DI,
         DI_REQ   => SPI_DI_REQ,
         ADDR     => open,
         DATA_IN  => irq_in,
         READ_EN  => irq_rd,         
         WRITE_EN => irq_wr,
         DATA_OUT => irq_out
      );
   
      
   --vlastni radic preruseni
   IRQctrl: interrupt_controller
      port map(
         CLK      => CLK,
         RST      => RESET,
         IRQ_IN   => interrupts, --vektor zdroju preruseni
         IRQ_OUT  => total_irq,  --celkove vystupni preruseni, urceno pro MCU
         DATA_OUT => irq_in,
         READ_EN  => irq_rd,
         DATA_IN  => irq_out,
         WRITE_EN => irq_wr
      );          
      
   -- vystupni interrupt se nahodi jen pokud neprobiha prenos na sbernici SPI
   IRQ <= total_irq and not SPI_CS;
   LEDF <= not (total_irq and not SPI_CS);

   --napojeni radicu protokolu na radic preruseni
   interrupts(7 downto 6) <= (others => '0');    
   interrupts(0) <= rc5_vld;
   interrupts(1) <= nec_vld;
   interrupts(2) <= sirc12_vld;
   interrupts(3) <= sirc15_vld;
   interrupts(4) <= sirc20_vld;
  
   --detektor preruseni indikujici pripravenost  k vysilani
   process(RESET,CLK)
   begin
      if (RESET = '1') then
         reg_a  <= '0';
         reg_b <= '0';
      elsif (CLK'event) and (CLK = '1') then 
         reg_a  <= (rc5_busy or nec_busy or sirc_busy);
         reg_b <= reg_a;
      end if;   
   end process;          
   interrupts(5) <= reg_b and (not reg_a);  
                                                                    
   --multiplexer pro vyber z ktereho zarizeni se budou cist data
   mux_out  <= mux_in0 when (addr = "000") else  -- RC5
               mux_in1 when (addr = "001") else  -- NEC (10 = NEC repeat)
               mux_in2; --> plati pro vsechny verze SIRC 12 / 15 / 20 !
                                  
                        
   --demultiplexer pro vyber do ktereho zarizeni poslat signal zapisu
   dmx_in <= write_en_sl & addr_r;
   with (dmx_in) select
      dmx_out <= "000001" when "1000",
                 "000010" when "1001",
                 "000100" when "1010",
                 "001000" when "1011",
                 "010000" when "1100",
                 "100000" when "1101",
                 "000000" when others;
   rc5_wr <= dmx_out(0);
   nec_wr <= dmx_out(1);
   nec_rpt <= dmx_out(2);
   sirc20_wr <= dmx_out(3);
   sirc15_wr <= dmx_out(4);
   sirc12_wr <= dmx_out(5); 
               

   -- ****************************************
   -- **    pripojeni pripravku k FITkitu   **
   -- ****************************************
   -- VERZE 1.2:
   --      ir_recv <= '1' when ((rc5_busy or nec_busy or sirc_busy)='1') else
   --           X(14);
   --      X(8) <= rc5_ir or nec_ir or sirc_ir; 
   -------------------------------------------
   -- VERZE 2.0:
   --      ir_recv <= '1' when ((rc5_busy or nec_busy or sirc_busy)='1') else
   --           X(12);
   --      X(6) <= rc5_ir or nec_ir or sirc_ir; 
   -------------------------------------------
    
   --halfduplex - prijem povolen pouze pokud se nevysila - kvuli odrazum
   ir_recv <= '1' when ((rc5_busy or nec_busy or sirc_busy)='1') else
              X(12);  
    
   --slouceni vysilaciho signalu do jednoho pomoci hradla OR
   X(6) <= rc5_ir or nec_ir or sirc_ir;        

end main;
