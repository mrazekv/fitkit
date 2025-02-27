-- sdram.vhd: SDRAM second level controller
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
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
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.sdram_controller_cfg.all;

-- SYNTH-ISE-9.2: slices=91, slicesFF=107, 4luts=149
entity sdram_controller is
   port (
      -- Hodiny, reset, ...
      CLK     : in std_logic; -- 50 MHz default (for autorefresh)
      RST     : in std_logic;
      ENABLE  : in std_logic;
      BUSY    : out std_logic;
      
      -- Adresa/data
      ADDR        : in std_logic_vector(22 downto 0);

      DATA_OUT    : out std_logic_vector(7 downto 0);
      DATA_VLD    : out std_logic; -- DATA_OUT valid
      READ_EN     : in std_logic; -- begin read

      DATA_IN     : in std_logic_vector(7 downto 0);
      WRITE_EN    : in std_logic; -- begin write

      -- SDRAM
      RAM_A      : out std_logic_vector(13 downto 0);
      RAM_D      : inout std_logic_vector(7 downto 0);
      RAM_DQM    : out std_logic;
      RAM_CS     : out std_logic;
      RAM_RAS    : out std_logic;
      RAM_CAS    : out std_logic;
      RAM_WE     : out std_logic;
      RAM_CLK    : out std_logic;
      RAM_CKE    : out std_logic
      );
end sdram_controller;

architecture arch_sdram_controller of sdram_controller is

   component sdram_raw_controller
      generic (
         GEN_AUTO_REFRESH : boolean := true;
         OPTIMIZE_REFRESH : sdram_optimize := oAlone
         );
      port (
         -- Hodiny, reset, ...
         CLK     : in std_logic;
         RST     : in std_logic;
         ENABLE  : in std_logic;
         BUSY    : out std_logic;
   
         -- Adresa/data
         ADDR_ROW    : in std_logic_vector(11 downto 0);
         ADDR_COLUMN : in std_logic_vector(8 downto 0);
         BANK        : in std_logic_vector(1 downto 0);
         DATA_IN     : in std_logic_vector(7 downto 0);
         DATA_OUT    : out std_logic_vector(7 downto 0);
         DATA_VLD    : out std_logic; -- Output data valid
   
         -- Pozadavek + jeho potvrzeni
         CMD     : in sdram_func;
         CMD_WE  : in std_logic;
   
         -- SDRAM
         RAM_A      : out std_logic_vector(13 downto 0);
         RAM_D      : inout std_logic_vector(7 downto 0);
         RAM_DQM    : out std_logic;
         RAM_CS     : out std_logic;
         RAM_RAS    : out std_logic;
         RAM_CAS    : out std_logic;
         RAM_WE     : out std_logic;
         RAM_CLK    : out std_logic;
         RAM_CKE    : out std_logic
      );
   end component;
   
   signal cmd            : sdram_func;
   signal cmd_set        : std_logic;
   signal we_x           : std_logic;
   signal re_x           : std_logic;
   signal we_x2          : std_logic;
   signal re_x2          : std_logic;
   signal we_mx          : std_logic;
   signal re_mx          : std_logic;
   signal aCol           : std_logic_vector(8 downto 0);
   signal aRow           : std_logic_vector(11 downto 0);
   signal aBank          : std_logic_vector(1 downto 0);

begin

   aCol <= ADDR(8 downto 0);
   aRow <= ADDR(20 downto 9);
   aBank <= ADDR(22 downto 21);
   
   -- SDRAM
   raw_sdram: sdram_raw_controller
      generic map (
         GEN_AUTO_REFRESH => true,
         OPTIMIZE_REFRESH => oAlone
         )
      port map (
         -- Hodiny, reset, ...
         CLK         => CLK,
         RST         => RST,
         ENABLE      => ENABLE,
         BUSY        => BUSY,
   
         -- Adresa/data
         ADDR_COLUMN => aCol,
         ADDR_ROW    => aRow,
         BANK        => aBank,
         DATA_IN     => DATA_IN,
         DATA_OUT    => DATA_OUT,
         DATA_VLD    => DATA_VLD,
            
         -- Pozadavek + jeho potvrzeni
         CMD      => cmd,
         CMD_WE   => cmd_set,
   
         -- SDRAM
         RAM_A       => RAM_A,
         RAM_D       => RAM_D,
         RAM_DQM     => RAM_DQM,
         RAM_CS      => RAM_CS,
         RAM_RAS     => RAM_RAS,
         RAM_CAS     => RAM_CAS,
         RAM_WE      => RAM_WE,
         RAM_CLK     => RAM_CLK,
         RAM_CKE     => RAM_CKE
      );
   
   -- SPI_WE/RE
   detect: process(CLK, RST)
   begin
     if RST = '1' then
         we_x <= '0';
         re_x <= '0';
         we_x2 <= '0';
         re_x2 <= '0';
     elsif CLK'event and CLK = '1' then
         we_x2 <= we_x;
         re_x2 <= re_x;
         we_x <= WRITE_EN;
         re_x <= READ_EN;
     end if;
   end process;

   -- Detekce R/W
   we_mx <= (not we_x2 and we_x);
   re_mx <= (not re_x2 and re_x);
   
   -- Odesilani prikazu R/W do radice RAM
   ram_send_cmd: process(CLK, RST)
   begin
     if RST = '1' then
        cmd <= fNop;
        cmd_set <= '0';
     elsif CLK'event and CLK = '1' then
         cmd_set <= '0';
         if re_mx = '1' then
            -- Read
            cmd <= fRead;
            cmd_set <= '1';
         elsif we_mx = '1' then
            -- Write
            cmd <= fWrite;
            cmd_set <= '1';
         end if;
     end if;
   end process;

end arch_sdram_controller;
