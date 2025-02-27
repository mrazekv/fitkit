-- nec_full.vhd: NEC code IR transciever
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

architecture full of ir_NEC is

--signaly prijimace
type FSMstate is (S_IDLE, S_LEAD1, S_LEAD2, S_REP, S_NORM, S_BIT0, S_BIT1, S_STORE0, S_STORE1, S_CHECK, S_COMPLETE,S_DATA_REP);

signal pstate       : FSMstate;
signal nstate       : FSMState;
signal cnt          : std_logic_vector(16 downto 0);
signal rep_tmr      : std_logic_vector(19 downto 0);
signal sh_reg       : std_logic_vector(32 downto 0);
signal dout         : std_logic_vector(23 downto 0);
signal sh_rst       : std_logic;
signal sh_wr        : std_logic;
signal sh_val       : std_logic;
signal tim_rst      : std_logic;
signal data_valid   : std_logic;
signal recv_complet : std_logic;
signal ir_reg_a     : std_logic;
signal ir_reg_b     : std_logic;
signal ir_down      : std_logic;
signal ir_up        : std_logic;
signal check_ok     : std_logic; 
signal repeat_data  : std_logic;
signal rep_tout     : std_logic;
signal last_valid   : std_logic;
signal rep_tmr_rst  : std_logic;

--signaly vysilace
type TFSMstate is (S_IDLE, S_LOAD, S_HEADER, S_NORM, S_MARK, S_BIT0, S_BIT1, S_SHIFT, S_STOP,
                   S_TMRRST1, S_TMRRST2, S_TMRRST3, S_TMRRST4, S_WAIT, S_REPEAT);

signal tr_pstate    : TFSMstate;
signal tr_nstate    : TFSMState;
signal gtics        : std_logic_vector(6 downto 0);
signal space_cnt    : std_logic_vector(13 downto 0);
signal tcnt         : std_logic_vector(9 downto 0);
signal bitcount     : std_logic_vector(4 downto 0);
signal sh_outreg    : std_logic_vector(31 downto 0);
signal din          : std_logic_vector(31 downto 0);
signal irout        : std_logic;  
signal out_en       : std_logic;
signal gen_cmp      : std_logic;
signal tx_init      : std_logic;
signal tics_rst     : std_logic;
signal dshift       : std_logic;
signal sh_load      : std_logic;
signal is_repeat    : std_logic;

begin

   --kontrola prijatych dat na spravnost (komparator)
   check_ok <= '1' when (sh_reg(24 downto 17) = not sh_reg(32 downto 25)) else 
               '0'; 


   data_valid <= recv_complet and check_ok; 
   DATA_VLD <= data_valid or repeat_data;
   
   
   --mapovani bitu na sbernici: 
   -- msb | ADR15 - ADR0 | CMD7 - CMD0 | lsb
   DATA_OUT <= dout(15 downto 0) & dout(23 downto 16);
   
   
   --klopny obvod RS drzici informaci u platosti predchozich dat
   last_valid <= '1' when (data_valid='1') else
                 '0' when (rep_tout='1') else
                 last_valid;  
                 
   --komparator pro definovani maximalni doby cekani na opakovaci pulz  (115ms = cca +5%)               
   rep_tout <= '1' when (rep_tmr="11001111000000000000") else
               '0';                 
   
   rep_tmr_rst <= data_valid or repeat_data or rep_tout;
   
   --klopny obvod D pro pomatovani prijatych dat (do prichodu dalsich...)
   process(RST,CLK)
   begin
      if (RST='1') then
         dout <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if (data_valid='1') then
            dout <= sh_reg(24 downto 1); --nezajima nas negovana podoba commandu
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
   
   
   --citac casu pro odpocet delky opakovaciho intervalu
   process(CLK, rep_tmr_rst)
   begin
     if (rep_tmr_rst='1') then
        rep_tmr <= (others => '0');
     elsif (CLK'event) and (CLK='1') then
        rep_tmr <= rep_tmr + 1;
     end if;
   end process;    
   
   
   --posuvny registr vystupu
   -- msb | CMD'7 - CMD'0 | CMD7 - CMD0 | ADR15 - ADR0 | 1 lsb   
   process(sh_rst,CLK)
   begin
      if (sh_rst='1') then
         sh_reg <= "100000000000000000000000000000000";
      elsif (CLK'event) and (CLK='1') then
         if (sh_wr='1') then
            sh_reg <= sh_val & sh_reg(32 downto 1);
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
   process(pstate,ir_down,ir_up,cnt,last_valid,sh_reg)
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
            if (cnt="01111011000111101") then --vstup do validniho pasma (8.55ms = -5%)
               nstate <= S_LEAD2;
            elsif (ir_up='1') then            --leader konci moc brzo -> chyba
               nstate <= S_IDLE; 
            end if;
            
         when S_LEAD2 =>
            nstate <= S_LEAD2;
            if (cnt="10001000000101000") then --timeout (9.45ms) (+5%)
               nstate <= S_IDLE;
            elsif (ir_up='1') then            --nacten 9ms leader code, pokracuj
               tim_rst <= '1';  
               nstate <= S_REP;               --detekci typu ramce (REP/NORM)
            end if;  
            
         when S_REP =>    
            nstate <= S_REP;
            if (cnt="00110000111101100") then --prepnuti na normalni (3.4ms)
               nstate <= S_NORM;              --(stred mezi 4.5ms a 2.25ms)
            elsif (ir_down='1') then
               if (last_valid='1') then
                  nstate <= S_DATA_REP;
               else
                  nstate <= S_IDLE;
               end if;   
            end if;                 
            
         when S_NORM =>     
            nstate <= S_NORM; 
            if (cnt="01000100000010100") then --vyprseni limitu (4.725ms = +5%)
               nstate <= S_IDLE;  
            elsif (ir_down='1') then
               tim_rst <= '1';
               nstate <= S_BIT0;              --vlastni nacitani bitu
            end if;                          
            
         when S_BIT0 =>
            nstate <= S_BIT0;
            if (cnt="00011000011110110") then --prepnuti na bit o log 1 (1.7ms)
               nstate <= S_BIT1;              --(stred mezi 1.125ms a 2.25ms) 
            elsif (ir_down='1') then
               nstate <= S_STORE0;             --nacten bit o hodnote "0" 
            end if;
            
         when S_BIT1 =>     
            nstate <= S_BIT1;
            if (cnt="00100010000001110") then --vyprseni limitu (2.363ms = +5%)
               nstate <= S_IDLE;            
            elsif (ir_down='1') then
               nstate <= S_STORE1;             --nacten bit o hodnote "1" 
            end if;                      
            
         when S_STORE0 =>
            tim_rst <= '1';   
            nstate <= S_CHECK;
            
         when S_STORE1 =>
            tim_rst <= '1';   
            nstate <= S_CHECK;            
            
         when S_CHECK =>
            tim_rst <= '1'; 
            nstate <= S_BIT0;
            if (sh_reg(0)='1') then
               nstate <= S_COMPLETE;
            end if;                      
            
         when S_COMPLETE =>
            nstate <= S_IDLE;
            
         when S_DATA_REP =>
            nstate <= S_IDLE;                              
         
         when others => null;   
      end case;    
   end process; 
   
   
   --FSM Output logic
   process(pstate)
   begin
      sh_rst <= '0'; 
      recv_complet <= '0';
      sh_wr <= '0';  
      sh_val <= '0';
      repeat_data <= '0';      

      case pstate is
         when S_IDLE =>  
            sh_rst <= '1';            

         when S_STORE0 =>  
            sh_wr <= '1';
            
         when S_STORE1 => 
            sh_val <= '1'; 
            sh_wr <= '1';              
                                                                                    
         when S_COMPLETE =>     
            recv_complet <= '1';
            
         when S_DATA_REP =>     
            repeat_data <= '1';                
            
         when others => null;
      end case;         
   end process;     
    
------------vysilac--------------------------------------------------------

   --mapovani vstupnich dat  do posuvneho registru
   -- | neg CMD7 - CMD0 | CMD7 - CMD0 | ADR15 - ADR0 |
   din <= (not DATA_IN(7 downto 0)) & DATA_IN(7 downto 0) & DATA_IN (23 downto 8);
   
   
   IR_OUT <= irout when (out_en='1') else
            '0';
   
   --generator 38kHz   (97 x SMCLK) !!!!!!!!!!!!!!!!
   gen_cmp <= '1' when (gtics="1100001") else '0';    
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
   
   
   --pocitadlo 38kHz pulzu
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
   
   
   --pocitadlo delky za jakou se bude opakovat ramec
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
   
   
   --posuvny registr s paralelnim vstupem
   process(CLK)
   begin
      if (CLK'event and CLK='1') then
         if (sh_load='1') then
            sh_outreg <= din;
         elsif (dshift='1') then   
            sh_outreg <= '0' & sh_outreg(31 downto 1);
         end if;   
      end if;
   end process; 
   

   -- registr uchovavaji informaci zda jde o normalni ci opakovaci ramec
   process(CLK)
   begin
      if (CLK'event and CLK='1') then
         if (WR_REPEAT='1') then
            is_repeat <= '1';
         elsif (WRITE_EN='1') then
            is_repeat <= '0';
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
   process(tr_pstate,WRITE_EN,WR_REPEAT,tcnt,bitcount,space_cnt,sh_outreg)
   begin
      tr_nstate <= S_IDLE;

      case tr_pstate is   
           
         when S_IDLE =>
            tr_nstate <= S_IDLE;
            if (WRITE_EN='1') then
               tr_nstate <= S_LOAD;
            elsif (WR_REPEAT='1') then
               tr_nstate <= S_HEADER;   
            end if;
                        
         when S_LOAD =>
            tr_nstate <= S_HEADER;
            
         when S_HEADER =>              -- 9ms header
            tr_nstate <= S_HEADER;
            if (tcnt="1010101100") then
               tr_nstate <= S_TMRRST1;
            end if; 
            
         when S_TMRRST1 =>
            tr_nstate <= S_NORM;
            if (is_repeat='1') then
               tr_nstate <= S_REPEAT;
            end if; 
               
         when S_REPEAT =>              -- 2.25ms space
            tr_nstate <= S_REPEAT;
            if (tcnt="0010101010") then
               tr_nstate <= S_TMRRST4;
            end if;                
            
         when S_NORM =>                -- 4.5ms space
            tr_nstate <= S_NORM;
            if (tcnt="0101010110") then
               tr_nstate <= S_TMRRST2;
            end if; 
            
         when S_TMRRST2 =>
            tr_nstate <= S_MARK;            
            
         when S_MARK =>                -- 560us mark
            tr_nstate <= S_MARK;
            if (tcnt="0000101010") then
               tr_nstate <= S_TMRRST3;
            end if;  
            
         when S_TMRRST3 =>
            if (sh_outreg(0)='1') then
               tr_nstate <= S_BIT1;
            else
               tr_nstate <= S_BIT0;
            end if;            
            
         when S_BIT0 =>                -- 565us space
            tr_nstate <= S_BIT0;
            if (tcnt="0000101010") then
               tr_nstate <= S_SHIFT;
            end if; 
            
         when S_BIT1 =>                -- 1690us space
            tr_nstate <= S_BIT1;
            if (tcnt="0010000000") then
               tr_nstate <= S_SHIFT;
            end if; 
            
         when S_SHIFT =>
            tr_nstate <= S_TMRRST2;
            if (bitcount="11111") then
               tr_nstate <= S_TMRRST4;
            end if;
            
         when S_TMRRST4 =>
            tr_nstate <= S_STOP;                
               
         when S_STOP =>                -- 560us stopbit mark
            tr_nstate <= S_STOP;
            if (tcnt="0000101010") then
               tr_nstate <= S_WAIT;      
            end if;
            
         when S_WAIT =>                --ceka 8208 tiku, pak ukonci BUSY priznak
            tr_nstate <= S_WAIT;
            if (space_cnt="10000000010000") then
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

         when S_TMRRST1 =>
            tics_rst <= '1'; 
            
         when S_TMRRST2 =>
            tics_rst <= '1';             
            
         when S_MARK =>
            out_en <= '1';
            
         when S_TMRRST3 =>
            tics_rst <= '1';             
                       
         when S_SHIFT =>
            dshift <= '1';
            
         when S_TMRRST4 =>
            tics_rst <= '1';                                                  

         when S_STOP =>
            out_en <= '1';                          
            
         when others => null;
      end case;         
   end process;      
        
end full;
