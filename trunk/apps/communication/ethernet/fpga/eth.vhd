-- eth.vhd : Ethernet transmitter (with CRC support)
-- Copyright (C) 2009 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
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
-- This software is provided ``as is'', and any express or implied
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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity ethernet is
   port( 
      CLK      : in std_logic; -- synchronizace (20 MHz)
      RESET    : in std_logic; -- asynchronnni reset

      -- zapis dat
      DATA_WE  : in std_logic;
      DATA_LEN : in std_logic_vector(10 downto 0);

      -- interface do BRAM
      RAM_ADDR : out std_logic_vector(10 downto 0);
      RAM_DATA : in std_logic_vector(7 downto 0);

      BUSY : out std_logic;

      -- diferencialni interface   
      TxN : out std_logic;
      TxP : out std_logic
   ); 
end ethernet;


architecture behavioral of ethernet is

signal len_reg : std_logic_vector(10 downto 0);
signal len_reg_en : std_logic;
signal len_cmp : std_logic;
signal datastart_cmp : std_logic;

signal idle_cntr : std_logic_vector(16 downto 0);
signal idle_cntr_en : std_logic;
signal idle_cntr_ld : std_logic;
signal nlp_cmp : std_logic;

signal addr_cntr : std_logic_vector(10 downto 0);
signal addr_cntr_en : std_logic;
signal addr_cntr_ld : std_logic;

signal tx_en   : std_logic;
signal tx_data : std_logic;
signal tx_data_mx : std_logic_vector(2 downto 0);

signal crc_reg   : std_logic_vector(31 downto 0);
signal crc_flush : std_logic;
signal crc_ld    : std_logic;
signal crc_en    : std_logic;
signal crc_din   : std_logic;
signal crclast_cmp : std_logic;

signal dout_shreg_ld : std_logic;
signal dout_shreg_en : std_logic;
signal dout_shreg : std_logic_vector(7 downto 0);

signal bit_cntr : std_logic_vector(2 downto 0);
signal bit_cntr_en : std_logic;
signal bit_cntr_last : std_logic;

type FSMstate is (SInit, SNLP, SSendInit, SSendPreamble1, SSendPreamble2, SSendData1, SSendData2, SSendCRC1, SSendCRC2, SCRCc, SCRCd);
signal pstate : FSMstate; -- actual state
signal nstate : FSMstate; -- next state 

begin
   --NLP counter
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         idle_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (idle_cntr_ld = '1') then
            idle_cntr <= (others => '0');
         elsif (idle_cntr_en = '1') then
            idle_cntr <= idle_cntr + 1;
         end if;
      end if;
   end process;

   nlp_cmp <= '1' when conv_integer(idle_cntr) = 0 else '0';

   --LENGTH register
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (DATA_WE = '1') then --nacteni obsahu citace
            len_reg <= DATA_LEN;
         end if;
      end if;
   end process;

   --ADDR/byte counter
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         addr_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (addr_cntr_ld = '1') then
            addr_cntr <= (others => '0');
         elsif (addr_cntr_en = '1') then
            addr_cntr <= addr_cntr + 1;
         end if;
      end if;
   end process;

   RAM_ADDR <= addr_cntr;
   len_cmp  <= '1' when (addr_cntr = len_reg) else '0';
   datastart_cmp <= '1' when addr_cntr(3) = '1' else '0'; -- bude '1' v okamziku, kdy addr=8 (konec preamble a zacatek dat)
   crclast_cmp <= '1' when addr_cntr(1 downto 0) = "11" else '0'; -- bude '1' v okamziku, kdy addr=3 (odesila se posledni 4. byte crc)

   --Data shift register
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (dout_shreg_ld = '1') then
            dout_shreg <= RAM_DATA;
         elsif (dout_shreg_en = '1') then
            dout_shreg <= '0' & dout_shreg(7 downto 1);
         end if;
      end if;
   end process;

   --Bit counter
   process (RESET, CLK)
   begin
      if (RESET = '1') then
         bit_cntr <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (bit_cntr_en = '1') then
            bit_cntr <= bit_cntr + 1;
         end if;
      end if;
   end process;

   bit_cntr_last <= '1' when bit_cntr = "111" else '0';

   --CRC calculator
   process (RESET, CLK)
   begin
      if (CLK'event) and (CLK = '1') then
         if (crc_ld = '1') then
            crc_reg <= (others => '1');
         elsif (crc_en = '1') then
            if (crc_din = '1') then
               crc_reg <= (crc_reg(30 downto 0) & '0') xor conv_std_logic_vector(16#04C11DB7#, 32);
            else
               crc_reg <= (crc_reg(30 downto 0) & '0');
            end if;
         end if;
      end if;
   end process;

   crc_din <= dout_shreg(0) xor crc_reg(31) when crc_flush='0' else '0';
             
   --Present State registr
   pstatereg: process(RESET, CLK)
   begin
      if (RESET='1') then
         pstate <= SInit;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;

   --Next State logic, Output logic
   nstate_logic: process(pstate, nlp_cmp, bit_cntr_last, len_cmp, datastart_cmp, crclast_cmp, DATA_WE)
   begin
      BUSY <= '0';
      idle_cntr_en <= '0';
      idle_cntr_ld <= '0';
      addr_cntr_en <= '0';
      addr_cntr_ld <= '0';
      dout_shreg_ld <= '0';
      dout_shreg_en <= '0';
      bit_cntr_en <= '0';
      tx_data_mx <= "000";
      tx_en <= '0';
      crc_en <= '0';
      crc_flush <= '0';
      crc_ld <= '0';

      nstate <= SInit;
      case pstate is
         when SInit =>
            idle_cntr_en <= '1';

            nstate <= SInit;
            if (DATA_WE = '1') then
               --pozadavek na odeslani ethernetoveho paketu
               nstate <= SSendInit;
            elsif (nlp_cmp = '1') then
               --vyprsel timeout, je zapotrebi poslat NLP (normal link pulse)
               nstate <= SNLP;
            end if;

         when SNLP =>
            --Vygenerovani NLP
            tx_en <= '1';
            tx_data_mx <= "100";
            idle_cntr_en <= '1';

            nstate <= SInit;
            if (DATA_WE = '1') then
               --je-li pozadavek na odeslani paketu, zpracovat jej
               nstate <= SSendInit;
            end if;
            
         --Priprava na odeslani ethernetoveho ramce
         when SSendInit =>
            BUSY <= '1';

            addr_cntr_en <= '1';
            dout_shreg_ld <= '1';
            crc_ld <= '1';

            --zakaz generovani NLP, reset pocitadla
            idle_cntr_en <= '0';
            idle_cntr_ld <= '1';

            nstate <= SSendPreamble1;

         -- Odeslani synch. hlavicky - preamble (8B), faze 1
         when SSendPreamble1 => 
            BUSY <= '1';
            tx_en <= '1';
            tx_data_mx <= "000";
            nstate <= SSendPreamble2;

         -- Odeslani synch. hlavicky - preamble (8B), faze 2
         when SSendPreamble2 => 
            BUSY <= '1';
            tx_en <= '1';
            tx_data_mx <= "001";
            dout_shreg_en <= '1';
            bit_cntr_en <= '1';

            nstate <= SSendPreamble1;
            if (bit_cntr_last = '1') then
               addr_cntr_en <= '1';
               dout_shreg_ld <= '1';
               dout_shreg_en <= '0';
               if (datastart_cmp = '1') then
                  nstate <= SSendData1;
               end if;
            end if;

         -- Odeslani dat, faze 1
         when SSendData1 => 
            BUSY <= '1';
            crc_en <= '1';
            tx_en <= '1';
            tx_data_mx <= "000";
            nstate <= SSendData2;

         -- Odeslani dat, faze 2
         when SSendData2 => 
            BUSY <= '1';
            tx_en <= '1';
            tx_data_mx <= "001";
            dout_shreg_en <= '1';
            bit_cntr_en <= '1';

            nstate <= SSendData1;
            if (bit_cntr_last = '1') then
               addr_cntr_en <= '1';
               dout_shreg_ld <= '1';
               dout_shreg_en <= '0';
               if (len_cmp = '1') then
                  nstate <= SSendCRC1;
                  addr_cntr_ld <= '1';
               end if;
            end if;

         -- Odeslani CRC (4B), faze 1
         when SSendCRC1 => 
            BUSY <= '1';
            tx_en <= '1';
            tx_data_mx <= "010";
            nstate <= SSendCRC2;

         -- Odeslani CRC (4B), faze 2
         when SSendCRC2 => 
            BUSY <= '1';
            crc_en <= '1';
            crc_flush <= '1';


            tx_en <= '1';
            tx_data_mx <= "011";
            bit_cntr_en <= '1';

            nstate <= SSendCRC1;
            if (bit_cntr_last = '1') then
               addr_cntr_en <= '1';
               if (crclast_cmp = '1') then
                  nstate <= SInit;
                  addr_cntr_ld <= '1';
                  --povoleni citace, aby se v nasledujicim taktu nevygeneroval NLP
                  idle_cntr_en <= '1';
               end if;
            end if;

         when others => null;
      end case;
   end process;

   -- vystup
   tx_data <= not dout_shreg(0) when tx_data_mx="000" else
              dout_shreg(0)     when tx_data_mx="001" else
              crc_reg(31)       when tx_data_mx="010" else
              not crc_reg(31)   when tx_data_mx="011" else
              '1'; --NLP 

   -- generovani diferencialniho vystupu
   TxP <= (tx_data)     when tx_en='1' else '0';
   TxN <= (not tx_data) when tx_en='1' else '0'; 
   
end behavioral;

