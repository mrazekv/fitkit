-- Author:      Michal Kajan
-- Project:     BIN 2007/2008 Game of Life for FITkit
-- Date:        2.5.2008  
-- File:        ca_life_ent.vhd
-- Description: Entity file for component.
--


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;


-- ----------------------------------------------------------------------------
--  Entity declaration: CA_LIFE
-- ----------------------------------------------------------------------------

entity CA_LIFE is
   port(
      -- general signals
      CLK         : in std_logic;
      RESET       : in std_logic;

      -- address for Block RAM, used for VGA controller
      ADDRESS_VGA : in std_logic_vector(12 downto 0);
      
      -- write enable for Block RAM, all data will be written do BRAM_1
      WRITE_EN    : in std_logic;
      
      -- enable for internal FSM
      FSM_EN      : in std_logic;
      
      -- data to be written to Block RAM
      DATA_IN     : in std_logic_vector(1 downto 0);
      
      -- address for write operations to Block RAM 1
      ADDRESS_WR  : in std_logic_vector(12 downto 0);

      -- signalizes that 1/4 of second has elapsed
      TIME_FLAG   : in std_logic;
      
      -- 2-bit data output
      DATA_OUT    : out std_logic_vector(1 downto 0)
   );

end entity CA_LIFE;

