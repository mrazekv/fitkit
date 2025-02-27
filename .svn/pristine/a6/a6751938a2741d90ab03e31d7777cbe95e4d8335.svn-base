-- Author:      Michal Kajan
-- Project:     BIN 2007/2008 Game of Life for FITkit
-- Date:        2.5.2008  
-- File:        fsm_en_gen.vhd
-- Description: FSM for generating enable for FSM that controls
--              memory operations.


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;


-- ----------------------------------------------------------------------------
--  Entity declaration: FSM_EN_GEN
-- ----------------------------------------------------------------------------

entity FSM_EN_GEN is
   port(
      -- general input
      CLK       : in std_logic;
      RESET     : in std_logic;

      -- signal from FSM controlling memory operations that whole matrix
      -- has been processed
      DONE_IN   : in std_logic;

      -- global enable input from toplevel component
      GLOBAL_EN : in std_logic;

      -- signalizes elapsing 1/4 of second
      TIME_FLAG : in std_logic;

      -- enable output for FSM controlling memory operations
      EN_FSM    : out std_logic

   );
end entity FSM_EN_GEN;

-- ----------------------------------------------------------------------------
--  Architecture declaration: FSM_EN_GEN_ARCH
-- ----------------------------------------------------------------------------
architecture FSM_EN_GEN_ARCH of FSM_EN_GEN is
   ----------------------------------------------------------------------------

   -- signal declarations 
   -- control FSM declaration
   -- determines which row is currently being processed
   type t_states is (wait_for_done, wait_for_time_flag); 
   signal present_state, next_state : t_states;

begin

   -- FSM control process
   fsm_ctrlp : process(CLK, RESET, GLOBAL_EN, next_state)
   begin
      if (CLK'event AND CLK='1') then
         if (RESET='1') then 
            present_state <= wait_for_done;
         elsif (GLOBAL_EN = '1') then
            present_state <= next_state;
         end if;
      end if;
   end process fsm_ctrlp;

   -- FSM next state logic process
   fsm_nslp : process(present_state, GLOBAL_EN, TIME_FLAG, DONE_IN)
   begin
      case present_state is

         -- wait_for_done
         when wait_for_done => 
            -- default option
            next_state <= present_state;

            -- only if we have set flag DONE, we can change the state
            if (GLOBAL_EN = '1') then
               if (DONE_IN = '1') then
                  next_state <= wait_for_time_flag;
               end if;
            end if;

         when wait_for_time_flag =>
            -- default option
            next_state <= present_state;

            -- only if we have set flag signalizing that 1/4 of second 
            -- has elapsed, we can change the state
            if (GLOBAL_EN = '1') then
               if (TIME_FLAG = '1') then
                  next_state <= wait_for_done;
               end if;
            end if;

         when others =>
            null;
      end case;
   end process fsm_nslp;

   fsm_olp : process(GLOBAL_EN, TIME_FLAG, DONE_IN, present_state)
   begin
      -- default values
      EN_FSM <= '0';

      case present_state is
         -- waiting for done
         when wait_for_done =>
            -- if FSM for memory operations is externally enabled
            -- we will enable it with this FSM
            if (GLOBAL_EN = '1') then
                  EN_FSM <= '1';
            end if;

         -- waiting for time flag signalizing that 1/4 of second has elapsed
         when wait_for_time_flag =>
            EN_FSM <= '0';
            if (GLOBAL_EN = '1') then        
               if (TIME_FLAG = '1') then
                  EN_FSM <= '1'; 
               end if;
            end if;
  
         when others =>
            null;
      end case;
   end process fsm_olp;
end architecture FSM_EN_GEN_ARCH;



