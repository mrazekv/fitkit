-- interrupt.vhd:
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
use IEEE.std_logic_ARITH.ALL;
use IEEE.std_logic_UNSIGNED.ALL;


-- SYNTH-ISE-8.2: slices=9, slicesFF=16, 4luts=11
entity interrupt_controller is
   generic (
      WIDTH : integer := 8 -- pocet vstupu preruseni
   );
   port (
      CLK    : in std_logic;
      RST    : in std_logic;

      -- preruseni zarizeni
      IRQ_IN  : in std_logic_vector(WIDTH-1 downto 0);
      IRQ_OUT : out std_logic;

      -- vektor preruseni
      DATA_OUT : out std_logic_vector(WIDTH-1 downto 0);
      READ_EN  : in std_logic;
      
      -- maska preruseni
      DATA_IN  : in std_logic_vector(WIDTH-1 downto 0);
      WRITE_EN : in std_logic
   );
end interrupt_controller;

architecture arch_interrupt of interrupt_controller is

   signal irqmask_reg   : std_logic_vector(WIDTH-1 downto 0);
   signal irqvector_reg : std_logic_vector(WIDTH-1 downto 0);

begin
   -- IRQ vector
   DATA_OUT <= irqvector_reg;

   -- IRQ request output
   request:process(irqvector_reg)
   variable irqset : std_logic;
   begin
      irqset := '0';
      for i in 0 to (WIDTH-1) loop
         irqset := irqset or irqvector_reg(i);
      end loop;
      IRQ_OUT <= irqset;
   end process;

   -- Interrupt mask register
   mask: process(CLK, RST)
   begin
      if (RST = '1') then
         -- all interrupts are disabled default
         irqmask_reg  <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         if (WRITE_EN = '1') then
            irqmask_reg <= DATA_IN;  
         end if;
      end if;
   end process mask;

   --Interrupt vector register
   interrupt: process(CLK, RST)
   begin
      if (RST = '1') then
         irqvector_reg <= (others => '0');
      elsif (CLK'event) and (CLK = '1') then
         for i in 0 to WIDTH-1 loop
            if (READ_EN='1') then
               -- IRQ served
               irqvector_reg(i) <= '0';
            else
               -- IRQ request
               irqvector_reg(i) <= irqmask_reg(i) and (irqvector_reg(i) or IRQ_IN(i));
            end if;
         end loop;
      end if;
   end process interrupt;

end arch_interrupt;

