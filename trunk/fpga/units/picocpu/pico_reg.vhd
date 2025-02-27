-- pico_reg.vhd: PicoBUS register
-- Copyright (C) 2011 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
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
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.math_pack.all; 

entity PICO_reg is
   generic (
      DATA_WIDTH : integer := 8;  
      BASE_ADDR  : integer := 16#10#;
      BYPASS     : boolean := false
   );
   port (
      -- Synchronizace
      CLK      : in  std_logic;

      -- Picoblaze port
      WRITE_STROBE  : in std_logic;
      READ_STROBE   : in std_logic;
      PORT_ID       : in std_logic_vector(7 downto 0);
      OUT_PORT      : in std_logic_vector(7 downto 0);
      IN_PORT       : out std_logic_vector(7 downto 0);

      --Datove rozhrani
      DATA_IN  : in  std_logic_vector (DATA_WIDTH-1 downto 0);
      DATA_OUT : out std_logic_vector (DATA_WIDTH-1 downto 0)
   );
end PICO_reg;
  

architecture beh of PICO_reg is
   constant ADDRBITS : integer := log2(DATA_WIDTH / 8);
 
   signal data_reg : std_logic_vector((2**ADDRBITS)*DATA_WIDTH - 1 downto 0) := (others => '0');
   signal port_out : std_logic_vector(7 downto 0);
   signal port_sel : std_logic;
   signal port_outsel : std_logic := '0';

begin

   DATA_OUT <= data_reg(DATA_WIDTH-1 downto 0);

   IN_PORT <= (others => 'Z') when port_outsel='0' else port_out;

   port_sel <= '1' when (PORT_ID(7 downto ADDRBITS) = conv_std_logic_vector(BASE_ADDR / (2**ADDRBITS), 8-ADDRBITS)) else '0';
  
   process (CLK)
     variable bnk_addr : integer;
   begin
     if (CLK'event) and (CLK = '1') then

          if (READ_STROBE = '1') then
             port_outsel <= port_sel;

             if (port_sel = '1') then
                 if (ADDRBITS > 0) then
                    bnk_addr := conv_integer(PORT_ID(ADDRBITS-1 downto 0));
                 else
                    bnk_addr := 0;
                 end if;

                 if (BYPASS) then
                    port_out <= data_reg(8*bnk_addr+7 downto 8*bnk_addr);
                 else
                    if (8*bnk_addr+7 < DATA_WIDTH) then
                       port_out <= DATA_IN(8*bnk_addr+7 downto 8*bnk_addr);
                    else
                       port_out <= (others => '1');
                    end if;
                 end if;
             end if;

          end if;

          if (WRITE_STROBE = '1') then
             if (port_sel = '1') then
                if (ADDRBITS > 0) then
                   bnk_addr := conv_integer(PORT_ID(ADDRBITS-1 downto 0));
                else
                   bnk_addr := 0;
                end if;

                data_reg(8*bnk_addr+7 downto 8*bnk_addr) <= OUT_PORT;
             end if;
          end if;

     end if;
   end process;

end architecture;
