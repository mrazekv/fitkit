-- sirc_full.vhd: Sony SIRC code IR transciever
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
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

architecture full of ir_SIRC is

--signaly prijimace
type FSMstate is (S_IDLE, S_LEAD1, S_LEAD2, S_SPACE, S_BSTART, S_BIT0, S_BIT1, S_STORE0, S_STORE1, S_12_FINISH, S_15_FINISH, S_20_FINISH);

signal pstate       : FSMstate;
signal nstate       : FSMState;
signal cnt          : std_logic_vector(14 downto 0);
signal sh_reg       : std_logic_vector(19 downto 0);
signal dout         : std_logic_vector(22 downto 0);
signal reccount     : std_logic_vector(4 downto 0);
signal format_out   : std_logic_vector(22 downto 0);
signal format       : std_logic_vector(1 downto 0);
signal sh_rst       : std_logic;
signal sh_wr        : std_logic;
signal sh_val       : std_logic;
signal tim_rst      : std_logic;
signal data12_valid : std_logic;
signal data15_valid : std_logic;
signal data20_valid : std_logic;
signal ir_reg_a     : std_logic;
signal ir_reg_b     : std_logic;
signal ir_down      : std_logic;
signal ir_up        : std_logic;

--signaly vysilace
type TFSMstate is (S_IDLE, S_LOAD, S_HEADER, S_SPACE, S_DECISION, S_BIT0, S_BIT1, S_SHIFT, S_WAIT, S_TMRRST);

signal tr_pstate    : TFSMstate;
signal tr_nstate    : TFSMState;
signal gtics        : std_logic_vector(6 downto 0);
signal space_cnt    : std_logic_vector(11 downto 0);
signal tcnt         : std_logic_vector(7 downto 0);
signal bitcount     : std_logic_vector(4 downto 0);
signal comp_val     : std_logic_vector(4 downto 0);
signal sh_outreg    : std_logic_vector(19 downto 0);
signal din          : std_logic_vector(19 downto 0);
signal tx_init      : std_logic;
signal irout        : std_logic;
signal out_en       : std_logic;
signal all_sent     : std_logic;
signal write_en     : std_logic;
signal dshift       : std_logic;
signal gen_cmp      : std_logic;
signal tics_rst     : std_logic;
signal sh_load      : std_logic;

begin

   DATA_12_VLD <= data12_valid;
   DATA_15_VLD <= data15_valid;
   DATA_20_VLD <= data20_valid;                 
   
   --mapovani bitu na sbernici: 
   -- | 00000000 | 000 ADR4- ADR0 | 0 CMD6 - CMD0 |        12 bit verze
   -- | 00000000 | ADR7  -   ADR0 | 0 CMD6 - CMD0 |        15 bit verze
   -- | EX7 - EX0| 000 ADR4- ADR0 | 0 CMD6 - CMD0 |        20 bit verze
   DATA_OUT <= dout(22 downto 7)&'0'&dout(6 downto 0);
   
    
   --multiplexer pro vyber vyrianty ulozeni (12bit / 15 / 20 bit)
   format_out  <= "00000000000"&sh_reg(19 downto 15)&sh_reg(14 downto 8) when (format = "00") else
                  "00000000"&sh_reg(19 downto 12)&sh_reg(11 downto 5) when (format = "01") else 
                  sh_reg(19 downto 12)&"000"&sh_reg(11 downto 7)&sh_reg(6 downto 0);
    
    
   --klopny obvod D pro pomatovani prijatych dat (do prichodu dalsich...)
   process(RST,CLK)
   begin
      if (RST='1') then
         dout <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if ((data12_valid or data15_valid or data20_valid)='1') then
            dout <= format_out;
         end if;   
      end if;
   end process;    


   --detektor vzestupne a sestupne  hrany
   process(RST,CLK)
   begin
      if (RST = '1') then
         ir_reg_a  <= '1';
         ir_reg_b <= '1';
      elsif (CLK'event) and (CLK = '1') then 
         ir_reg_a  <= IR_IN;
         ir_reg_b <= ir_reg_a;
      end if;   
   end process;          
   ir_down <= ir_reg_b and (not ir_reg_a);    
   ir_up <= ( not ir_reg_b) and ir_reg_a; 
   
   
   --citac casu pro mereni delky pulzu
   process(CLK,tim_rst)
   begin
     if (tim_rst='1') then
        cnt <= (others => '0');
     elsif (CLK'event) and (CLK='1') then
        cnt <= cnt + 1;
     end if;
   end process; 
   
 
   --pocitadlo prijatych bitu  (stejny reset jako posuvny registr!)
   process (sh_rst, CLK)
   begin
      if (sh_rst='1') then
         reccount <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (sh_wr='1') then
            reccount <= reccount+1;
         end if;
      end if;
   end process;
 
   
   --posuvny registr vystupu
   -- msb | ADR4 - ADR0    | CMD6 - CMD0 | 00000000 | lsb  
   -- msb | ADR7 - ADR0       | CMD6 - CMD0 | 00000 | lsb
   -- msb | EXT7 - EXT0 | ADR4 - ADR0 | CMD6 - CMD0 | lsb   
   process(sh_rst,CLK)
   begin
      if (sh_rst='1') then
         sh_reg <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if (sh_wr='1') then
            sh_reg <= sh_val & sh_reg(19 downto 1);
         end if;
      end if;
   end process; 
    
     
 --FSM Present State logic
   process(RST,CLK)
   begin
      if (RST='1') then
         pstate <= S_IDLE;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;
     
 
   --FSM Next State logic
   process(pstate,ir_down,ir_up,cnt,reccount)
   begin
      nstate <= S_IDLE;
      tim_rst <= '0';
      case pstate is  
         
         when S_IDLE =>
            tim_rst <= '1';
            nstate <= S_IDLE;
            if (ir_down='1') then            --sestupnou hranou zacni casovat
               nstate <= S_LEAD1;
            end if;
            
         when S_LEAD1 =>
            nstate <= S_LEAD1;
            if (cnt="100000110101001") then  --vstup do validniho pasma (2.28ms = -5%)
               nstate <= S_LEAD2;
            elsif (ir_up='1') then           --leader konci moc brzo -> chyba
               nstate <= S_IDLE; 
            end if;
            
         when S_LEAD2 =>
            nstate <= S_LEAD2;
            if (cnt="100100010010011") then  --timeout (2.52ms = +5%)  -> chyba
               nstate <= S_IDLE;
            elsif (ir_up='1') then           --nacten 2.4ms leader code, pokracuj
               tim_rst <= '1';  
               nstate <= S_SPACE;
            end if;  
    
         when S_SPACE =>
            nstate <= S_SPACE;
            if (ir_down='1') then
               tim_rst <= '1';
               nstate <= S_BSTART;              --zacatek dalsiho pulzu -> OK
            elsif (cnt="001001000100101") then  --timeout 630us pro mezeru:
               if (reccount="01100") then
                  nstate <= S_12_FINISH;        --nacteno 12 bitu -> OK
               elsif (reccount="01111") then
                  nstate <= S_15_FINISH;        --nacteno 15 bitu -> OK
               elsif (reccount="10100") then
                  nstate <= S_20_FINISH;        --nacteno 20 bitu -> OK
               else
                  nstate <= S_IDLE;             --pocet nesedi -> CHYBA!
               end if;
            end if;
            
            when S_BSTART =>                      --zacatek nacitani BITu
               nstate <= S_BSTART;      
               if (ir_up='1') then                --nesmi prijit konec pulzu
                  nstate <= S_IDLE;
               elsif (cnt="001000001101010") then --vstup do bitu 0 (570us = -5%)
                  nstate <= S_BIT0;
               end if;
               
            when S_BIT0 =>
               nstate <= S_BIT0;
               if (ir_up='1') then
                  nstate <= S_STORE0;             --nacten bit o hodnote '0'
               elsif (cnt="001100111101011") then -- 0.9ms = prechod do nacitani 1
                  nstate <= S_BIT1;               -- ( = stred mezi 0.6ms a 1.2ms)
               end if;  
               
            when S_BIT1 =>
               nstate <= S_BIT1;
               if (ir_up='1') then
                  nstate <= S_STORE1;             --nacten bit o hodnote '1'
               elsif (cnt="010010101010011") then -- 1.29ms (= +8%) timeout -> chyba
                  nstate <= S_IDLE;
               end if; 
               
            when S_STORE0 =>
               tim_rst <= '1';
               nstate <= S_SPACE;
               
            when S_STORE1 =>
               tim_rst <= '1';
               nstate <= S_SPACE; 
               
            when S_12_FINISH =>
               nstate <= S_IDLE;
               
            when S_15_FINISH =>
               nstate <= S_IDLE;
               
            when S_20_FINISH =>
               nstate <= S_IDLE;                              
               
         when others => null;   
      end case;    
   end process; 
   
   
   --FSM Output logic
   process(pstate)
   begin
      sh_rst <= '0'; 
      sh_wr <= '0';  
      sh_val <= '0';
      data12_valid <= '0';
      data15_valid <= '0'; 
      data20_valid <= '0';   
      format <= "00";    

      case pstate is
         when S_IDLE =>  
            sh_rst <= '1';                        

         when S_STORE0 =>  
            sh_wr <= '1';
            
         when S_STORE1 => 
            sh_val <= '1'; 
            sh_wr <= '1';              
                                                                                    
         when S_12_FINISH =>     
            data12_valid <= '1';
            
         when S_15_FINISH =>     
            data15_valid <= '1'; 
            format <= "01";
            
         when S_20_FINISH =>     
            data20_valid <= '1';
            format <= "10";                                                 
            
         when others => null;
      end case;         
   end process;     

------------------------------  vysilac ----------------------------------------

   --mapovani na vstup:
   -- kus sbernice je sdilen -> nutne osetrit v SW aby nesla zadat vetsi adresa!
   --                             | ADR4 - ADR0 | CMD6 - CMD0 |     12bit
   --               | ADR7 - ADR5 | ADR4 - ADR0 | CMD6 - CMD0 |     15bit
   -- | EXT7 - EXT3 | EXT2 - EXT0 | ADR4 - ADR0 | CMD6 - CMD0 |     20bit
   din <= DATA_IN(23 downto 19)&(DATA_IN(18 downto 16) or DATA_IN(15 downto 13))&DATA_IN(12 downto 8)&DATA_IN(6 downto 0);

   IR_OUT <= irout when (out_en='1') else
             '0';

   --generator 40kHz   (92 x SMCLK) !!!!!!!!!!!!!!!!
   gen_cmp <= '1' when (gtics="1011100") else '0';    
   process (tx_init,CLK)
   begin
      if (tx_init='1') then
         gtics <= (others => '0');
         irout <= '0';
      elsif (CLK='1') and (CLK'event) then         
         if (gen_cmp='1') then
            gtics <= (others => '0'); 
            irout <= not irout; 
         else
            gtics <= gtics + 1;
         end if;
      end if;
   end process; 
   
   
   --pocitadlo 40kHz pulzu (casovani delek)
   process (tics_rst, CLK)
   begin
      if (tics_rst='1') then
         tcnt <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (gen_cmp='1') then
            tcnt <= tcnt+1;
         end if;
      end if;
   end process;
   
   
   --posuvny registr s paralelnim vstupem
   process(CLK)
   begin
      if (CLK'event and CLK='1') then
         if (sh_load='1') then
            sh_outreg <= din;
         elsif (dshift='1') then   
            sh_outreg <= '0' & sh_outreg(19 downto 1);
         end if;   
      end if;
   end process; 
   
   
   --pocitadlo vyslanych bitu
   process (tx_init, CLK)
   begin
      if (tx_init='1') then
         bitcount <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (dshift='1') then
            bitcount <= bitcount+1;
         end if;
      end if;
   end process;       
   
   
   --generovani zapisoveho signalu
   write_en <= (WRITE_12_EN or WRITE_15_EN or WRITE_20_EN);
   
   
   --D flipflop pro pomatovani hodnoty ke komparaci (pocet bitu)
   process(CLK)
   begin
      if (CLK'event) and (CLK='1') then
         if (write_en='1') then
            if (WRITE_12_EN='1') then
               comp_val <= "01100";        --12 bitu
            elsif (WRITE_15_EN='1') then
               comp_val <= "01111";        --15 bitu
            else   
               comp_val <= "10100";        --20 bitu
            end if;   
         end if;   
      end if;
   end process;
   
   
   --komparator vraci '1' pokud se pocet vyslanych bitu shoduje s danou verzi protokolu
   all_sent <= '1' when (bitcount=comp_val) else
               '0';  
   
   
   --pocitadlo doby za jakou se bude opakovat ramec
   process (tx_init, CLK)
   begin
      if (tx_init='1') then
         space_cnt <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (gen_cmp='1') then
            space_cnt <= space_cnt+1;
         end if;
      end if;
   end process;              
   
   
   --FSM Present State logic
   process(RST,CLK)
   begin
      if (RST='1') then
         tr_pstate <= S_IDLE;
      elsif (CLK'event) and (CLK='1') then
         tr_pstate <= tr_nstate;
      end if;
   end process; 
   
   
   --FSM Next State logic
   process(tr_pstate,write_en,tcnt,all_sent,sh_outreg)
   begin
      tr_nstate <= S_IDLE;

      case tr_pstate is   
           
         when S_IDLE =>
            tr_nstate <= S_IDLE;
            if (write_en='1') then
               tr_nstate <= S_LOAD; 
            end if;
                        
         when S_LOAD =>                -- naplneni posuvneho registru
            tr_nstate <= S_HEADER;
            
         when S_HEADER =>              -- 2.4ms header
            tr_nstate <= S_HEADER;
            if (tcnt="11000000") then
               tr_nstate <= S_TMRRST;
            end if; 
            
         when S_TMRRST =>
            tr_nstate <= S_SPACE; 
               
         when S_SPACE =>              -- 0.6ms space
            tr_nstate <= S_SPACE;
            if (tcnt="00110000") then
               tr_nstate <= S_DECISION;
            end if;                
            
         when S_DECISION =>
            if (all_sent='1') then    --pokud vse preneseno, prejdi na cekani
               tr_nstate <= S_WAIT;
            elsif (sh_outreg(0)='1') then
               tr_nstate <= S_BIT1;   --bude se prenaset log 1
            else
               tr_nstate <= S_BIT0;   --jinak 0
            end if; 
                                
         when S_BIT0 =>                -- 0.6ms mark pro logickou uroven 0
            tr_nstate <= S_BIT0;
            if (tcnt="00110000") then
               tr_nstate <= S_SHIFT;
            end if; 
            
         when S_BIT1 =>                -- 1.2ms mark pro logickou uroven 1
            tr_nstate <= S_BIT1;
            if (tcnt="01100000") then
               tr_nstate <= S_SHIFT;
            end if; 
            
         when S_SHIFT =>               --posuv datoveho registru doprava 
            tr_nstate <= S_SPACE;
                                 
         when S_WAIT =>                --ceka do doby 42ms, pak ukonci BUSY priznak
            tr_nstate <= S_WAIT;
            if (space_cnt="110100100000") then
               tr_nstate <= S_IDLE;
            end if;                                                                
         
         when others => null;   
      end case;    
   end process;     
  

   --FSM Output logic
   process(tr_pstate)
   begin
      tx_init <= '0';
      BUSY <= '1';
      sh_load <= '0';
      out_en <= '0';
      dshift <= '0';
      tics_rst <= '0';
            
      case tr_pstate is
         when S_IDLE =>  
            tics_rst <= '1';
            tx_init <= '1';
            BUSY <= '0'; 
         
         when S_LOAD => 
            tics_rst <= '1';      
            sh_load <= '1';
            tx_init <= '1';
             
         when S_HEADER =>
            out_en <= '1';   
                                  
         when S_TMRRST =>
            tics_rst <= '1';
            
         when S_DECISION =>
            tics_rst <= '1';                          
                       
         when S_SHIFT =>
            dshift <= '1';
            tics_rst <= '1'; 
            
         when S_BIT1 =>
            out_en <= '1';
            
         when S_BIT0 =>
            out_en <= '1';                                                                                
            
         when others => null;
      end case;         
   end process;    
    
end full;
