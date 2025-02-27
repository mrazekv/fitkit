-- rc5_full.vhd: Philips RC5x code IR transciever
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

architecture full of ir_RC5 is

------ signaly prijimace --------
type FSMstate is (S_IDLE, S_WAIT, S_RECV, S_CNTRSET, S_SPACE1, S_SPACE2, S_COMPLETE);

signal pstate     : FSMstate;
signal nstate     : FSMState;

signal cnt        : std_logic_vector(18 downto 0);
signal cnt_rst    : std_logic;
signal cnt_en     : std_logic;

signal ir_reg_a   : std_logic;
signal ir_reg_b   : std_logic;
signal ir_down    : std_logic;
signal ir_up      : std_logic;

signal timeout_en : std_logic;
signal timeout    : std_logic;
signal reset      : std_logic;       --reset prijimace

signal sh_wr      : std_logic;
signal sh_en      : std_logic;
signal sh_rst     : std_logic;
signal sh_val     : std_logic;
signal data_shreg : std_logic_vector(13 downto 0);
signal dout       : std_logic_vector(12 downto 0);
signal data_valid : std_logic;


-------- signaly vysilace ---------
type TrFSMstate is (S_IDLE, S_LOAD, S_TRANS, S_WAIT);

signal tr_pstate    : TrFSMstate;
signal tr_nstate    : TrFSMstate;

signal tx_init      : std_logic;      --reset komponent vysilace
signal gen_cmp      : std_logic;
signal dclk         : std_logic;
signal sh_load      : std_logic;
signal data_end     : std_logic;
signal frame_end    : std_logic;
signal out_en       : std_logic;

signal din          : std_logic_vector(13 downto 0);
signal sh_reg       : std_logic_vector(13 downto 0);
signal gtics        : std_logic_vector(6 downto 0);
signal pcount       : std_logic_vector(12 downto 0);      

-------------------------------------------------------------------------------
begin

   --------- POPIS PRIJIMACE --------

   DATA_VLD <= data_valid;
   
   --mapovani 13bitu dat na 24bit sbernici: 
   -- msb | 0 0 0 0 0 0 0 TB | 0 0 0 ADR4 - ADR0 | 0 CMD6 - CMD0 | lsb
   DATA_OUT <= "0000000"&dout(11)&"000"&dout(10 downto 6)&'0'&(not dout(12))&dout(5 downto 0);
   
   --klopny obvod D pro pomatovani prijatych dat (do prichodu dalsich...)
   process(RST,CLK)
   begin
      if (RST='1') then
         dout <= (others => '0');
      elsif (CLK'event) and (CLK='1') then
         if (data_valid='1') then
            dout <= data_shreg(12 downto 0);
         end if;   
      end if;
   end process;   
   
   --detektor vzestupne a sestupne  hrany
   process(reset,CLK)
   begin
      if (reset = '1') then
         ir_reg_a  <= '1';
         ir_reg_b <= '1';
      elsif (CLK'event) and (CLK = '1') then 
         ir_reg_a  <= IR_IN;
         ir_reg_b <= ir_reg_a;
      end if;   
   end process;          
   ir_down <= ir_reg_b and (not ir_reg_a);    
   ir_up <= ( not ir_reg_b) and ir_reg_a; 
   
   
   --hodnota do shift registru dana smerem hrany (dolu=1, nahoru=0)
   --staci ir_down protoze nemuzou nikdy byt oba v "1"
   sh_val <= ir_down;
   
   
   --signal povoluje zapis do posuvneho registru, v pripade nejake hrany
   sh_wr <= (ir_down or ir_up) and sh_en;
   
       
   --po dosazeni urciteho casu generovan timeout
   --timeout za 2*889us + 1/4*889us
   timeout <= '1' when (cnt="0000011100110011011") else '0';     
                            
   --signal reset jednak vstupnim RST, dale take timeoutem
   reset <= RST or  (timeout and timeout_en);
       
   --posuvny registr vystupu / format dat v data_shreg:
   -- msb | SB | CMD6 | TB | ADR4 - ADR0 | CMD5 - CMD0 | lsb
   process(sh_rst,CLK)
   begin
      if (sh_rst='1') then
         data_shreg <= "00000000000001";
      elsif (CLK'event) and (CLK='1') then
         if (sh_wr='1') then
            data_shreg <= data_shreg(12 downto 0) & sh_val;
         end if;
      end if;
   end process; 
 
    
   --citac casu
   process(CLK,cnt_rst)
   begin
     if (cnt_rst='1') then
        cnt <= (others => '0');
     elsif (CLK'event) and (CLK='1') then
        if (cnt_en='1') then
           cnt <= cnt + 1;
        end if;   
     end if;
   end process; 
   
   
   --FSM Present State logic
   process(reset,CLK)
   begin
      if (reset='1') then
         pstate <= S_IDLE;
      elsif (CLK'event) and (CLK='1') then
         pstate <= nstate;
      end if;
   end process;

   
   --FSM Next State logic
   process(pstate,ir_down,sh_wr,cnt,data_shreg)
   begin
      nstate <= S_IDLE;
      case pstate is
         
         when S_IDLE =>
            nstate <= S_IDLE;
            if (ir_down='1') then
               nstate <= S_WAIT;
            end if;
            
         when S_WAIT =>
            nstate <= S_WAIT;
            --dalsi korektni pulz ocekavan za 889us + 3/4*889us
            if (cnt="0000010110011001110") then
               nstate <= S_RECV;
            end if;
            
         when S_RECV =>
            nstate <= S_RECV;
            if (data_shreg(13)='1') then
               nstate <= S_SPACE1;
            elsif (sh_wr='1') then
               nstate <= S_CNTRSET;
            end if;  
            
         when S_CNTRSET =>
            nstate <= S_WAIT;
            
         when S_SPACE1 =>
            nstate <= S_SPACE2;            
            
         when S_SPACE2 =>
            nstate <= S_SPACE2;
            if (ir_down='1') then
               nstate <= S_IDLE;
            --po dobu 71ms nesmi prijit zadny pulz! (113ms perioda ramcu)  
            elsif (cnt="1111111111111111111") then
               nstate <= S_COMPLETE;
            end if;         
            
         when S_COMPLETE =>
            nstate <= S_IDLE;                  
         
         when others => null;   
      end case;    
   end process;
   
   
   --FSM Output logic
   process(pstate)
   begin
      cnt_rst <= '0';
      sh_rst <= '0'; 
      data_valid <= '0';
      sh_en <= '0';
      cnt_en <= '0';
      timeout_en <= '1';
      case pstate is
         when S_IDLE =>  
            cnt_rst <= '1';
            sh_rst <= '1';    
         
         when S_WAIT =>
            cnt_en <= '1';
             
         when S_RECV =>
            sh_en <= '1';
            cnt_en <= '1';
            
         when S_CNTRSET =>
            cnt_rst <= '1';
            
         when S_SPACE1 =>     
            cnt_rst <= '1';
            
         when S_SPACE2 =>     
            cnt_en <= '1';
            timeout_en <= '0';                                         
                        
         when S_COMPLETE =>     
            data_valid <= '1';
            
         when others => null;
      end case;         
   end process;
  
   
   --------- POPIS VYSILACE --------
   
   --mapovani: msb | SB | CMD6 | TB | ADR4 - ADR0 | CMD5 - CMD0 | lsb
   din <= '1'&(not DATA_IN(6))&DATA_IN(16)&DATA_IN(12 downto 8)&DATA_IN(5 downto 0);
    
   IR_OUT <= pcount(0) when (((dclk xor sh_reg(13)) and out_en)='1') else '0'; 
     
   --generator 36kHz   (102 * SMCLK)
   gen_cmp <= '1' when (gtics="1100101") else '0';    
   process (tx_init,CLK)
   begin
      if (tx_init='1') then
         gtics <= (others => '0');
      elsif (CLK='1') and (CLK'event) then         
         if (gen_cmp='1') then
            gtics <= (others => '0'); 
         else
            gtics <= gtics + 1;
         end if;
      end if;
   end process;


   --pocitadlo 36kHz pulzu
   process (tx_init, CLK)
   begin
      if (tx_init='1') then
         pcount <= (others => '0');
      elsif (CLK='1') and (CLK'event) then
         if (gen_cmp='1') then
            pcount <= pcount+1;
         end if;
      end if;
   end process; 
   
   
   --komparator pro detekci konce prenosu posledniho bitu
   data_end <= '1' when (pcount="0011011000000") else
               '0';  
               
   --komparator pro detekci konce prenosu celeho ramce
   frame_end <= '1' when (pcount="1111111111111") else
                '0';                
     
   --datove hodiny (32*36kHz)
   dclk <= pcount(6);
     

   --posuvny registr s asynchronnim paralelnim vstupem
   process(dclk,sh_load)
   begin
      if (sh_load='1') then
         sh_reg <= din;
      elsif (dclk'event and dclk='1') then
         sh_reg <= sh_reg(12 downto 0) & '0';
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
   process(tr_pstate,WRITE_EN,data_end,frame_end)
   begin
      tr_nstate <= S_IDLE;
      case tr_pstate is   
           
         when S_IDLE =>
            tr_nstate <= S_IDLE;
            if (WRITE_EN='1') then
               tr_nstate <= S_LOAD;
            end if;
            
         when S_LOAD =>
            tr_nstate <= S_TRANS;
            
         when S_TRANS =>
            tr_nstate <= S_TRANS;
            if (data_end='1') then
               tr_nstate <= S_WAIT;   
            end if; 
            
         when S_WAIT =>
            tr_nstate <= S_WAIT;
            if (frame_end='1') then
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
            
      case tr_pstate is
         when S_IDLE =>  
            tx_init <= '1'; 
            BUSY <= '0';
         
         when S_LOAD =>  
            tx_init <= '1';     
            sh_load <= '1';
            
         when S_TRANS =>
            out_en <= '1';                                                      
            
         when others => null;
      end case;         
   end process;   
      
end full;

