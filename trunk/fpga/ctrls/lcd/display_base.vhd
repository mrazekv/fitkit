-- Display_base.vhd : <short description>
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Jan Markovic   xmarko04@stud.fit.vutbr.cz
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

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Display_base is
   generic (
      INT_EN : std_logic := '0'
   );
   Port(
      reset     : in std_logic;
      clk       : in std_logic;

      --addr      : out  STD_LOGIC_VECTOR (N-1 downto 0);
      --data_out  : out  STD_LOGIC_VECTOR (7 downto 0);
      RS        : in  STD_LOGIC;
      data_in   : in  STD_LOGIC_VECTOR (7 downto 0);
      WRITE_EN  : in  STD_LOGIC;
      --READ_EN   : in  STD_LOGIC;
      
      -- ouuput intrrupt
      INTERRUPT : out STD_LOGIC;

      -- DISPLAY 
      RS_dis    : out   std_logic;
      data_dis  : inout std_logic_vector (7 downto 0) ;
      RW_dis    : out   std_logic;
      E_dis     : out   std_logic 
   );
end Display_base;

architecture Arch_Display_base of Display_base is

--TYPI
type State_display_type is (
   idle, 
   w0, w1, w2, w3, w4, w5, w6, w7, r1, r2, r3, r4, r5, r6, r7
); 

-- SIGMALY
signal d_state : State_display_type;
signal data    : std_logic_vector(7 downto 0); 
signal dis_busy: std_logic;
signal interrupt_out : std_logic;

begin

   INTERRUPT  <= INT_EN and interrupt_out;

state_machyne_display:process(clk,reset)
begin
   if reset = '1' then
     data  <= "00000000";
     RS_dis  <= '0'; 
     data_dis <= "ZZZZZZZZ";
     RW_dis  <= '1';
     E_dis  <= '0'; 
     dis_busy <= '0';
     d_state <= idle;
     interrupt_out <= '0';
  else 
     if clk'event and clk = '1' then
        interrupt_out <= '0';
        case d_state is
           when idle => 
              if WRITE_EN = '1' then   
                 d_state <= w0;          
--                 busy    <= '0';      
                 RS_dis  <= RS;
                 data    <= data_in;
              end if;
           when w0 =>
              d_state <= w1;
              RW_dis  <= '0';
           when w1 =>  
              d_state <= w2;
--              RW_dis <= '0';
           when w2 =>
              d_state <= w3;
              E_dis <= '1';
           when w3 =>
              d_state <= w4;
              data_dis <= data;
           when w4 =>
              d_state <= w5;
           when w5 =>
              d_state <= w6;
           when w6 =>
              E_dis <= '0';
              d_state <= w7;
           when w7 =>
              data_dis <= "ZZZZZZZZ";
              RW_dis <= '1';
              if INT_EN = '0' then
                 d_state <= idle;
              else
                 d_state <= r1;
              end if;   
--            -- busy <= '1';
                  
               -- wiat for busy
           when r1 =>
              d_state <= r2;
              RS_dis  <= '0'; -- read busy flag & addr
              RW_dis  <= '1';
           when r2 =>
              d_state <= r3;
              E_dis  <= '1';
           when r3 =>
              d_state <= r4; 
           when r4 =>
              d_state <= r5;
           when r5 =>
              d_state <= r6;
           when r6 =>
              d_state <= r7;
              dis_busy <= data_dis(7);
              E_dis <= '0';
           when r7 =>
              if dis_busy = '0' then -- uvidime noyna naopak
                 d_state <= idle; 
                 interrupt_out <= '1';
              else
                 d_state <= r1;
              end if;     
        end case;  
     end if;  
  end if;  
end process state_machyne_display;

end Arch_Display_base;

