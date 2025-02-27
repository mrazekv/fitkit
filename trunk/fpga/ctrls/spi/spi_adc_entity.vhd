-- spi_adc_entity.vhd : SPI Decoder Entity
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <xvasic11 AT stud.fit.vutbr.cz>
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

library ieee;
use ieee.std_logic_1164.all;

entity SPI_adc is
   generic (
      DELAY      : integer := 0;  -- zpozdeni (pocet taktu) mezi READ_EN a vystavenim dat na DATA_IN
      ADDR_WIDTH : integer := 8;  -- sirka adresy
      DATA_WIDTH : integer := 8;  -- sirka dat
      ADDR_OUT_WIDTH : integer := 4; -- sirka adresy na vystupu (0 <= ADR_OUT_WIDTH <= ADDR_WIDTH)
      BASE_ADDR  : integer := 16#10# -- adresovy prostor 10-1F
   );
   port (
      -- Synchronizace
      CLK      : in  std_logic;

      -- Rozhrani SPI (z SPI_control)
      CS       : in  std_logic;
      DO       : in  std_logic;
      DO_VLD   : in  std_logic;
      DI       : out std_logic;
      DI_REQ   : in  std_logic;

      --Datove rozhrani
      ADDR     : out  std_logic_vector (ADDR_OUT_WIDTH-1 downto 0);
      DATA_OUT : out  std_logic_vector (DATA_WIDTH-1 downto 0);
      DATA_IN  : in   std_logic_vector (DATA_WIDTH-1 downto 0);

      WRITE_EN : out  std_logic; --write to FPGA through DATA_OUT
      READ_EN  : out  std_logic  --read from FPGA through DATA_IN
   );
end SPI_adc;

