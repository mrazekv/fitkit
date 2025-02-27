-- serial.vhd : Serial transceiver
-- Copyright (C) 2007 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek (xvasic11 AT stud.fit.vutbr.cz)
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
use work.serial_cfg.all;

entity serial_transceiver is
   generic (
      SPEED     : serial_speed   := s460800Bd;
      DATAWIDTH : serial_databit := 8;
      STOPBITS  : serial_stopbit := 1;
      PARITY    : serial_parity  := sParityEven
   );
   port (
      CLK      : in std_logic;
      RESET    : in std_logic;

      DATA_IN  : in std_logic_vector(DATAWIDTH-1 downto 0);
      WRITE_EN : in std_logic;

      DATA_OUT : out std_logic_vector(DATAWIDTH-1 downto 0);
      DATA_VLD : out std_logic;

      BUSY     : out std_logic; 
      ERR      : out std_logic;
      ERR_RST  : in  std_logic;

      -- RS232
      RXD      : in  std_logic;
      TXD      : out std_logic;
      RTS      : in  std_logic;
      CTS      : out std_logic
   );
end serial_transceiver;


architecture struct of serial_transceiver is
   
   component serial_transmitter 
      generic (
         SPEED     : serial_speed := SPEED;
         DATAWIDTH : serial_databit := DATAWIDTH;
         STOPBITS  : serial_stopbit := STOPBITS;
         PARITY    : serial_parity := PARITY
      );
      port (
         -- Common interface
         CLK      : in std_logic;
         RESET    : in std_logic;
   
         DATA_IN  : in std_logic_vector(DATAWIDTH-1 downto 0);
         WRITE_EN : in std_logic;
   
         BUSY     : out std_logic; 
   
         -- Serial interface
         TXD      : out std_logic
      );
   end component;

   component serial_receiver
      generic (
         SPEED     : serial_speed := SPEED;
         DATAWIDTH : serial_databit := DATAWIDTH;
         STOPBITS  : serial_stopbit := STOPBITS;
         PARITY    : serial_parity := PARITY
      );
      port (
         CLK      : in std_logic;
         RESET    : in std_logic;
   
         DATA_OUT : out std_logic_vector(DATAWIDTH-1 downto 0);
         DATA_VLD : out std_logic;
   
         BUSY     : out std_logic; 
         ERR      : out std_logic;
         ERR_RST  : in  std_logic;
   
         -- Serial interface
         RXD      : in  std_logic
      );
   end component;
      
begin
   CTS <= '1';

   rx: serial_receiver
      generic map (
         SPEED => SPEED,
         DATAWIDTH => DATAWIDTH,      
         STOPBITS => STOPBITS,
         PARITY => PARITY
      )
      port map (
         CLK => CLK,
         RESET => RESET,
         DATA_OUT => DATA_OUT,
         DATA_VLD => DATA_VLD,
         BUSY => open,
         ERR => ERR,
         ERR_RST => ERR_RST,
         RXD => RXD
      );

   tx: serial_transmitter
      generic map (
         SPEED => SPEED,
         DATAWIDTH => DATAWIDTH,      
         STOPBITS => STOPBITS,
         PARITY => PARITY
      )
      port map (
         CLK => CLK,
         RESET => RESET,
         DATA_IN => DATA_IN,
         WRITE_EN => WRITE_EN,
         BUSY => BUSY,
         TXD => TXD
      );

end struct;

