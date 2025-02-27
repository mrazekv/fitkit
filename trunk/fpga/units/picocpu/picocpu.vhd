-- top_level.vhd: PicoBlaze CPU - FITkit base
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Èapka Ladislav <xcapka01 AT stud.fit.vutbr.cz>
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
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity picocpu is
   port (
      -- Hodinovy signal + reset
      CLK           : in std_logic;
      RST           : in std_logic;
      -- CPU enable, pri zmene programu v RAM musi byt v 0
      CORE_EN       : in std_logic;
      -- Pozadavek na cteni/zapis dat na port
      WRITE_STROBE  : out std_logic;
      READ_STROBE   : out std_logic;
      -- Identifikator portu      
      PORT_ID       : out std_logic_vector(7 downto 0);
      -- Vstupni/vystupni data portu
      OUT_PORT      : out std_logic_vector(7 downto 0);
      IN_PORT       : in std_logic_vector(7 downto 0);
      -- Signal pro preruseni + zpetne potvrzeni
      INTERRUPT     : in std_logic;
      INTERRUPT_ACK : out std_logic;
      -- Pristup k pameti instrukci
      RAM_ADDR      : in std_logic_vector(9 downto 0);
      RAM_DATA_IN   : in std_logic_vector(17 downto 0);
      RAM_DATA_OUT  : out std_logic_vector(17 downto 0);
      RAM_DATA_WE   : in std_logic
   );
end picocpu;

architecture arch_picocpu of picocpu is

   -- PicoBlaze CPU
   component kcpsm3 
      port (
         CLK           : in std_logic;
         RESET         : in std_logic;
         ADDRESS       : out std_logic_vector(9 downto 0); 
         INSTRUCTION   : in std_logic_vector(17 downto 0);
         PORT_ID       : out std_logic_vector(7 downto 0);
         WRITE_STROBE  : out std_logic;
         READ_STROBE   : out std_logic;
         OUT_PORT      : out std_logic_vector(7 downto 0);
         IN_PORT       : in std_logic_vector(7 downto 0);
         INTERRUPT     : in std_logic;
         INTERRUPT_ACK : out std_logic
      );
   end component;
   
   -- pamet pro instrukce
   component RAMB16_S18_S18
      port (
         DOA   : out std_logic_vector(15 downto 0);
         DOB   : out std_logic_vector(15 downto 0);
         DOPA  : out std_logic_vector(1 downto 0);
         DOPB  : out std_logic_vector(1 downto 0);
         ADDRA : in std_logic_vector(9 downto 0);
         ADDRB : in std_logic_vector(9 downto 0);
         CLKA  : in std_ulogic;
         CLKB  : in std_ulogic;
         DIA   : in std_logic_vector(15 downto 0);
         DIB   : in std_logic_vector(15 downto 0);
         DIPA  : in std_logic_vector(1 downto 0);
         DIPB  : in std_logic_vector(1 downto 0);
         ENA   : in std_ulogic;
         ENB   : in std_ulogic;
         SSRA  : in std_ulogic;
         SSRB  : in std_ulogic;
         WEA   : in std_ulogic;
         WEB   : in std_ulogic
      );
   end component;

   signal cpu_reset      : std_logic;

   signal cpu_addr       : std_logic_vector(9 downto 0);
   signal cpu_isnt       : std_logic_vector(17 downto 0);

   signal iram_data_out   : std_logic_vector(17 downto 0);
   signal iram_data_in    : std_logic_vector(17 downto 0);
   signal iram_data_we    : std_logic;
  
begin

   -- reset CPU
   cpu_reset <= RST or not CORE_EN;
   
   -- instance procesoru PicoBlaze
   processor: kcpsm3
      port map (
         CLK           => CLK,
         RESET         => cpu_reset,
         ADDRESS       => cpu_addr,
         INSTRUCTION   => cpu_isnt,
         
         PORT_ID       => PORT_ID,

         WRITE_STROBE  => WRITE_STROBE,
         OUT_PORT      => OUT_PORT,

         READ_STROBE   => READ_STROBE,
         IN_PORT       => IN_PORT,

         INTERRUPT     => INTERRUPT,
         INTERRUPT_ACK => INTERRUPT_ACK
      );

   -- PortA -> CPU, PortB -> SPI
   instr_ram: RAMB16_S18_S18
      port map (
         DOA   => cpu_isnt(15 downto 0),
         DOB   => iram_data_out(15 downto 0),
         DOPA  => cpu_isnt(17 downto 16),
         DOPB  => iram_data_out(17 downto 16),
         ADDRA => cpu_addr,
         ADDRB => RAM_ADDR,
         CLKA  => CLK,
         CLKB  => CLK,
         DIA   => (others => 'Z'),
         DIB   => iram_data_in(15 downto 0),
         DIPA  => (others => 'Z'),
         DIPB  => iram_data_in(17 downto 16),
         ENA   => '1',
         ENB   => '1',
         SSRA  => '0',
         SSRB  => '0',
         WEA   => '0',
         WEB   => iram_data_we
      );
   
   -- Data INPUT <-> RAM
   iram_data_in  <= RAM_DATA_IN;
   RAM_DATA_OUT  <= iram_data_out;
   iram_data_we  <= RAM_DATA_WE and not CORE_EN;

end arch_picocpu;
