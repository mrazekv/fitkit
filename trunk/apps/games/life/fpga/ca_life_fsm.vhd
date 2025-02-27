-- Author:      Michal Kajan
-- Project:     BIN 2007/2008 Game of Life for FITkit
-- Date:        2.5.2008  
-- File:        ca_life_fsm.vhd
-- Description: FSM for controlling Block RAM read/write operations
--


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_misc.all;

-- ----------------------------------------------------------------------------
--  Entity declaration: CA_LIFE_FSM
-- ----------------------------------------------------------------------------

entity CA_LIFE_FSM is 
   port (
      -- INPUT PORTS ----------------------------------------------------------
      -- general signals
      CLK         : in std_logic;
      RESET       : in std_logic;

      -- data from given Block RAM
      INPUT_DATA  : in std_logic_vector(1 downto 0);

      -- write enable signal - request to write data to Block RAM
      -- after the WE signal has been deactivated, FSM will switch
      -- to its initial state
      WE_REQUEST  : in std_logic;

      -- enable input
      FSM_EN      : in std_logic;

      -- OUTPUT PORTS ---------------------------------------------------------
      -- address to read data from selected Block RAM
      READ_ADDR   : out std_logic_vector(12 downto 0);

      -- address to write data to selected Block RAM
      WRITE_ADDR  : out std_logic_vector(12 downto 0);

      -- selection of Block RAM, used as select signal for multiplexors
      BRAM_SELECT : out std_logic;

      -- write enable signal for writing in selected Block RAM
      WRITE_EN    : out std_logic;

      -- evaluating of matrix was finished
      DONE        : out std_logic;

      -- data to be sent to selected Block RAM
      OUTPUT_DATA : out std_logic_vector(1 downto 0)
   );
end entity CA_LIFE_FSM;


-- ----------------------------------------------------------------------------
--  Architecture declaration: CA_LIFE_FSM_ARCH
-- ----------------------------------------------------------------------------
architecture CA_LIFE_FSM_ARCH of CA_LIFE_FSM is 
   -- constant declarations
   constant MAX_ROW_ADDR     : integer := 59;
   constant MAX_COL_ADDR     : integer := 79;
   constant POINTS_PROCESSED : integer := 4800; -- (80x60)
   ----------------------------------------------------------------------------

   -- signal declarations 
   -- control FSM declaration
   -- determines which row is currently being processed
   type t_states is (row_1, row_2, row_3, wait_state); 
   signal present_state, next_state : t_states;
   ----------------------------------------------------------------------------

   -- 4800 points (80x60), how many points in certain BRAM was processed
   signal cnt_points_processed   : std_logic_vector(12 downto 0) := (others => '0');
   -- enable signal
   signal cnt_points_processed_ce : std_logic;
   signal cnt_points_processed_ld : std_logic;

   -- select signal for Block RAM, counts every 4800 cycles
   signal cnt_bram_select      : std_logic_vector(0 downto 0) := "0";
   -- enable signal
   signal cnt_bram_select_ce   : std_logic;
   signal cnt_bram_select_ld   : std_logic;

   -- we must know how many points in a row has been processed
   signal cnt_row_points       : std_logic_vector(1 downto 0) := "00";
   -- enable signal
   signal cnt_row_points_ce    : std_logic;
   signal cnt_row_points_ld    : std_logic;

   -- row address for currently processed point
   signal cnt_row_addr         : std_logic_vector(5 downto 0) := "000000";
   -- enable signal
   signal cnt_row_addr_ce      : std_logic;
   signal cnt_row_addr_ld      : std_logic;

   -- column address for currently processed point
   signal cnt_col_addr         : std_logic_vector(6 downto 0) := "0000000";
   -- enable signal
   signal cnt_col_addr_ce      : std_logic;
   signal cnt_col_addr_ld      : std_logic;

   signal cnt_cells_alive_1    : std_logic_vector(3 downto 0);
   signal cnt_cells_alive_1_ce : std_logic;
   signal cnt_cells_alive_1_ld : std_logic;

   -- status of observed cell
   signal reg_cell_status      : std_logic_vector(1 downto 0) := "00";
   signal reg_cell_status_ce   : std_logic := '0';

   signal store_bram_select    : std_logic_vector(0 downto 0) := "0";

   signal iscelllive : std_logic;
begin

   -- FSM control process
   fsm_ctrlp : process(CLK, RESET, next_state, FSM_EN, WE_REQUEST)
   begin
      if (CLK'event AND CLK='1') then
         if (RESET='1' OR WE_REQUEST = '1') then 
            present_state <= wait_state; --row_3;--row_1; 
         elsif (FSM_EN = '1') then
            present_state <= next_state;
         end if;
      end if;
   end process fsm_ctrlp;

   -- FSM next state logic process
   fsm_nslp : process(present_state, cnt_row_points, FSM_EN, WE_REQUEST,
                      cnt_points_processed)
   begin
      case present_state is

         -- ROW_1
         when row_1 => 
            -- default option
            next_state <= present_state;

            -- only if all 3 points in a row has been processed
            -- we can change a state
            if (FSM_EN = '1') then
               if (cnt_row_points="10") then
                  next_state <= row_2;
               end if;
            end if;

         -- ROW_2
         when row_2 =>
            -- default option
            next_state <= present_state;

            -- only if all 3 points in a row has been processed
            -- we can change a state
            if (FSM_EN = '1') then
               if (cnt_row_points="10") then
                  next_state <= row_3;
               end if;
            end if;

         -- ROW_3
         when row_3 =>
            -- default option
            next_state <= present_state;

            -- only if all 3 points in a row has been processed
            -- we can change a state
            if (FSM_EN = '1') then
               if (cnt_row_points="10") then
                  -- was whole matrix processed?
                  --if (conv_integer(cnt_points_processed) = POINTS_PROCESSED-1)then
                  --   next_state <= wait_state;
                  --else 
                     next_state <= row_1;
                  --end if;
               end if;
            end if;

         when wait_state =>
            -- default option
            next_state <= present_state;

            -- only if FSM was externally enabled we can change the state
            if (FSM_EN = '1') then
               next_state <= row_1;
            end if;

         when others =>
            null;

      end case;
   end process fsm_nslp;

   iscelllive <= '1' when (INPUT_DATA(1) = '1') else --10,11
                 '0'; --00,00

   -- FSM output logic process
   fsm_olp : process(cnt_row_points, iscelllive,
                     cnt_col_addr, cnt_row_addr,
                     cnt_points_processed,
                     cnt_cells_alive_1, present_state,
                     WE_REQUEST, FSM_EN, cnt_bram_select, store_bram_select)
   variable livecells : integer;
   begin
      -- default values
      cnt_bram_select_ce      <= '0';
      cnt_bram_select_ld      <= '0';

      cnt_points_processed_ce <= '0';
      cnt_points_processed_ld <= '0';

      cnt_row_addr_ce         <= '0';
      cnt_row_addr_ld         <= '0';

      cnt_col_addr_ce         <= '0';
      cnt_col_addr_ld         <= '0';

      cnt_row_points_ce       <= '0';
      cnt_row_points_ld       <= '0';

      cnt_cells_alive_1_ce    <= '0';
      cnt_cells_alive_1_ld    <= '0';
  
      reg_cell_status_ce      <= '0';

      WRITE_EN                <= '0';
      WRITE_ADDR              <= (others => '0');
      READ_ADDR               <= cnt_row_addr & cnt_col_addr;
      DONE                    <= '0';

      -- generating Block RAM select signal to output
      BRAM_SELECT <= cnt_bram_select(0);

      if (FSM_EN = '1') then

      case present_state is

         -- ROW 1
         when row_1 =>

            cnt_row_points_ce  <= '1';

            -- first cell in a row is being processed,
            if (cnt_row_points = "00") then
               -- SETTING ADDRESS

               -- setting row address, row address will be -1 of current point
               if (conv_integer(cnt_row_addr) = 0) then
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(MAX_ROW_ADDR, 6);
               else 
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)-1,6);
               end if; 

               -- setting column address
               -- current value will be used
               READ_ADDR(6 downto 0) <= cnt_col_addr;
 
               -- counting number of live cells in row 1
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;

            -- secod cell in row is being processed
            elsif (cnt_row_points = "01") then

               -- SETTING ADDRESS
               -- setting row address, row address will be +1 of current point
               if (conv_integer(cnt_row_addr) = 0) then
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(MAX_ROW_ADDR, 6);
               else 
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)-1,6);
               end if; 

               -- setting column address
               if (conv_integer(cnt_col_addr) + 1 > MAX_COL_ADDR) then
                  READ_ADDR(6 downto 0) <= (others => '0');
               else
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(conv_integer(cnt_col_addr)+1,7);
               end if;

               -- counting number of live cells in row 1
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;

            -- third cell in a row is being processed
            elsif (cnt_row_points = "10") then
               
               -- SETTING ADDRESS
               -- new column address will be used
               -- in this case only this value is problematic
               if (conv_integer(cnt_col_addr) = 0) then
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(MAX_COL_ADDR, 7);
               else
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(conv_integer(cnt_col_addr)-1,7);
               end if;                                      

               -- row address is unchanged
               READ_ADDR(12 downto 7) <= cnt_row_addr;

               -- counting number of live cells in row 1
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;
            end if;            

         -- ROW 2
         when row_2 =>

            cnt_row_points_ce  <= '1';

            -- first cell in a row is being processed,
            if (cnt_row_points = "00") then

               -- SETTING ADDRESS
               -- column address will be current address of cell
               READ_ADDR(6 downto 0) <= cnt_col_addr;

               -- row address will be current address of cell
               READ_ADDR(12 downto 7) <= cnt_row_addr;

               -- counting number of live cells in row 2
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;

            -- second cell in a row is being processed,
            elsif (cnt_row_points = "01") then

               -- NOTE this is the case when there is no observing of living
               -- cell since in this case it is the cell in the middle
               -- of neighbourhood itself 
               -- only storing its current status
               reg_cell_status_ce <= '1';

               -- row address will be current address of cell
               READ_ADDR(12 downto 7) <= cnt_row_addr;

               -- column address must be changed
               if (conv_integer(cnt_col_addr)+1 > MAX_COL_ADDR) then
                  READ_ADDR(6 downto 0) <= (others => '0');
               else 
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(conv_integer(cnt_col_addr)+1,7);
               end if;

            -- third cell in a row is being processed,
            elsif (cnt_row_points = "10") then

               -- SETTING ADDRESS

               -- new column address must be used
               -- in this case only this situation is problematic
               if (conv_integer(cnt_col_addr) = 0) then
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(MAX_COL_ADDR, 7);
               else
                  READ_ADDR(6 downto 0) <= conv_std_logic_vector(conv_integer(cnt_col_addr)-1, 7);
               end if;

               -- new row address must be used
               -- this situation is problematic
               if (conv_integer(cnt_row_addr) + 1 > MAX_ROW_ADDR) then
                  READ_ADDR(12 downto 7) <= (others => '0');
               else
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)+1, 6);                    
               end if;

               -- counting number of live cells in row 2
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;
            end if;

         when row_3 =>

            cnt_row_points_ce  <= '1';

            -- first cell in a row is being processed 
            if (cnt_row_points = "00") then

               -- SETTING ADDRESS
               -- row address must be changed
               -- this situation causes potential problem
               if (conv_integer(cnt_row_addr) + 1 > MAX_ROW_ADDR) then
                  READ_ADDR(12 downto 7) <= (others => '0');
               else
                  READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)+1, 6);
               end if;

               -- column address will be used current for middle cell
               READ_ADDR(6 downto 0) <= cnt_col_addr;

               -- counting number of live cells in row 3
               if (iscelllive = '1') then
                  -- incrementing number of live cells
                  cnt_cells_alive_1_ce <= '1';
               end if;

            -- second cell in row is being processed
            elsif (cnt_row_points = "01") then

                  -- row address must be changed
                  -- this situation causes potential problem
                  if (conv_integer(cnt_row_addr) + 1 > MAX_ROW_ADDR) then
                     READ_ADDR(12 downto 7) <= (others => '0');
                  else
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)+1, 6);
                  end if;                  

                  -- column address must be changed
                  if (conv_integer(cnt_col_addr) + 1 > MAX_COL_ADDR) then
                     READ_ADDR(6 downto 0) <= (others => '0');
                  else
                     READ_ADDR(6 downto 0) <= conv_std_logic_vector((conv_integer(cnt_col_addr)+1),7);
                  end if;

                  -- counting number of live cells in row 3
                  if (iscelllive = '1') then
                     -- incrementing number of live cells
                     cnt_cells_alive_1_ce <= '1';
                  end if;

            -- third cell in a row is being processed
            elsif (cnt_row_points = "10") then

               -- enabling cnt_points_processed
               cnt_points_processed_ce <= '1';

               -- enabling write signal
               WRITE_EN <= '1';

               -- SETTING WRITE ADDRESS (current address for cell)
               WRITE_ADDR <= cnt_row_addr & cnt_col_addr;

               if (iscelllive = '1') then         
                   livecells := conv_integer(cnt_cells_alive_1) + 1;
               else
                   livecells := conv_integer(cnt_cells_alive_1);
               end if;

               --new state computation
               if (reg_cell_status(1) = '1') then -- cell alive (10,11)
                  if (livecells = 3) or (livecells = 2) then -- cell is unchanged
                     OUTPUT_DATA <= "11";
                  else -- cell will die
                     OUTPUT_DATA <= "01";
                  end if;
               else --cell death (00,01)
                  if (livecells = 3) then --cell will come to life
                     OUTPUT_DATA <= "10";
                  else
                     OUTPUT_DATA <= "00"; -- cell is unchanged
                  end if;
               end if;

               -- new row address
               -- this situation is problematic
               if (conv_integer(cnt_row_addr) = 0) then
                  -- we reached last column in the row
                  if (conv_integer(cnt_col_addr) = MAX_COL_ADDR) then 
                     READ_ADDR(12 downto 7) <= (others => '0');
                  else
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(MAX_ROW_ADDR),6);
                  end if;
               -- what if row is last possible row...
               elsif (conv_integer(cnt_row_addr) = MAX_ROW_ADDR) then
                  -- was last column reached
                  if (conv_integer(cnt_col_addr) = MAX_COL_ADDR) then
                     -- MAX_ROW_ADDR will be used - we will cycle again
                     -- through whole matrix
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(MAX_ROW_ADDR),6);
                  else
                     -- it is decrement here!!!
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)-1,6);
                  end if;
               else
                  -- was last column reached
                  if (conv_integer(cnt_col_addr) = MAX_COL_ADDR) then
                     -- there is no decrement here!!! cell at new row will 
                     -- be processed
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr),6);
                  else
                     READ_ADDR(12 downto 7) <= conv_std_logic_vector(conv_integer(cnt_row_addr)-1,6);
                  end if;
               end if;

               -- column adress remains unchanged
               READ_ADDR(6 downto 0) <= cnt_col_addr;

               -- enabling counting address counters                                     
               cnt_col_addr_ce <= '1';
               -- row address counter can enabled only if column counter reached
               -- its maximum value
               if (cnt_col_addr = MAX_COL_ADDR) then
                  cnt_row_addr_ce <= '1';
               else
                  cnt_row_addr_ce <= '0';
               end if;

                  -- comparator, enables counting of Block RAM select counter
                  -- this signal is used to select correct Block RAM
                  -- for several operations (Read data, write data)
               if (cnt_points_processed =
                    conv_std_logic_vector(POINTS_PROCESSED-1, cnt_points_processed'length)
                   AND cnt_col_addr = MAX_COL_ADDR) then
                   cnt_bram_select_ce <= '1';

                  -- whole matrix processed, we're done
                  DONE <= '1';
               else
                  cnt_bram_select_ce <= '0';
               end if;

               -- loading counters of active cells to default value
               cnt_cells_alive_1_ld <= '1';

               -- enabling counting of active cells 
               cnt_cells_alive_1_ce <= '1';

            end if;


         -- waiting for enable signal
         when wait_state =>
            -- generating correct address for first point of neighbourhood
            -- this very important when state is changed to row1 - we need
            -- ready data from Block RAM at our component's input!
            
            -- address for row
            READ_ADDR(12 downto 7) <= conv_std_logic_vector(MAX_ROW_ADDR, 6);
            
            -- address for column
            READ_ADDR(6 downto 0) <= conv_std_logic_vector(MAX_COL_ADDR, 7);
         
            -- enable signal set, we should read from Block RAM defined by
            -- cnt_bram_select counter
            if (FSM_EN = '1') then
            
               -- loading all counters with their default value
               cnt_bram_select_ce      <= '1';
               cnt_bram_select_ld      <= '1';

               cnt_points_processed_ce <= '1';
               cnt_points_processed_ld <= '1';

               cnt_row_addr_ce         <= '1';
               cnt_row_addr_ld         <= '1';

               cnt_col_addr_ce         <= '1';
               cnt_col_addr_ld         <= '1';

               cnt_row_points_ce       <= '1';
               cnt_row_points_ld       <= '1';

               cnt_cells_alive_1_ce    <= '1';
               cnt_cells_alive_1_ld    <= '1';

      
               -- first Block RAM will be selected
               BRAM_SELECT <= '0';
            -- FSM is disabled, we just keep reading from old Block RAM
            -- (important for VGA controller)
            else 
               BRAM_SELECT <= cnt_bram_select(0); --store_bram_select(0);
            end if;

         when others =>
            null;
            

      end case;
      
      -- FSM is disabled
      else
         case present_state is
            -- we must take care of situation when FSM is done and waits for
            -- enable signal
            when row_1 =>
               if (cnt_row_points = "00" AND cnt_col_addr = "0000000" AND
                   cnt_row_addr = "000000" AND cnt_points_processed = X"000") then
                  BRAM_SELECT <= NOT(cnt_bram_select(0));
                  READ_ADDR   <= conv_std_logic_vector(MAX_ROW_ADDR, 6) &
                                 conv_std_logic_vector(MAX_COL_ADDR, 7);
                  
                  -- since Block RAMs will be switched address must be at the second
                  -- Block RAM before FSM is enabled, this "trick" allows data
                  -- to be ready when FSM is enabled and in current state this data
                  -- can be processed
                  WRITE_ADDR  <= conv_std_logic_vector(MAX_ROW_ADDR, 6) &
                                 conv_std_logic_vector(MAX_COL_ADDR, 7);                                                                
               end if;
            when others =>
               null;
         end case;      

      
      end if; -- if (FSM = '1')
   end process fsm_olp;

   -- COUNTERS SECTION --------------------------------------------------------
   -- counter for counting number of processed points
   cnt_points_p : process(RESET, CLK, cnt_points_processed_ce, cnt_points_processed_ld)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            cnt_points_processed <= (others => '0'); --conv_std_logic_vector(POINTS_PROCESSED-1,13);
         elsif (cnt_points_processed_ld = '1' AND cnt_points_processed_ce = '1') then
            cnt_points_processed <= (others => '0');
         elsif (cnt_points_processed =
                   conv_std_logic_vector(POINTS_PROCESSED-1,13) AND
                   cnt_points_processed_ce = '1') then
            cnt_points_processed <= (others => '0');            
         elsif (cnt_points_processed_ce = '1') then
            cnt_points_processed <= cnt_points_processed + '1';
         end if;
      end if;
   end process cnt_points_p; 

   -- counter for generating signal for selecting Block RAM
   cnt_bram_select_p : process(RESET, CLK, cnt_bram_select_ce, cnt_bram_select_ld)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            cnt_bram_select <= "0"; --"1"; 
         elsif (cnt_bram_select_ld = '1' AND cnt_bram_select_ce = '1') then
            cnt_bram_select <= "0";
         elsif (cnt_bram_select_ce = '1') then
            cnt_bram_select <= cnt_bram_select + '1';
         end if;
      end if;
   end process cnt_bram_select_p; 

   -- counts number of processed points per row (it is important to know whether
   -- 1st, 2nd or 3rd point in a row is being processed)
   cnt_rowpoints_p : process(RESET, CLK, cnt_row_points_ce, cnt_row_points_ld)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            cnt_row_points <=  (others => '0');
         elsif (cnt_row_points_ld = '1' AND cnt_row_points_ce = '1') then
            cnt_row_points <= (others => '0');
         elsif (cnt_row_points = "10" AND
                cnt_row_points_ce = '1') then  -- it counts only for 3 values
            cnt_row_points <= (others => '0');
         elsif (cnt_row_points_ce = '1') then
            cnt_row_points <= cnt_row_points + '1';
         end if;
      end if;
   end process cnt_rowpoints_p;

   -- counts addresses for columns
   cnt_col_addr_p : process(RESET, CLK, cnt_col_addr_ce, cnt_col_addr_ld)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            cnt_col_addr <= (others => '0'); --conv_std_logic_vector(MAX_COL_ADDR, 7);
         elsif (cnt_col_addr_ld = '1' AND cnt_col_addr_ce = '1') then
            cnt_col_addr <= (others => '0'); --conv_std_logic_vector(MAX_COL_ADDR, 7);
         elsif (cnt_col_addr = conv_std_logic_vector(MAX_COL_ADDR, 7) AND
             cnt_col_addr_ce = '1') then
            cnt_col_addr <= (others => '0');
         elsif (cnt_col_addr_ce = '1') then
            cnt_col_addr <= cnt_col_addr + '1'; 
         end if;
      end if;
   end process cnt_col_addr_p;    

   --counts addresses for rows
   cnt_row_addr_p : process(RESET, CLK, cnt_row_addr_ce, cnt_row_addr_ld)
   begin
      if (CLK'event AND CLK = '1') then
         --if (RESET = '1' conv_std_logic_vector(MAX_ROW_ADDR, 6)) then
         if (RESET = '1') then
            cnt_row_addr <= (others => '0'); --conv_std_logic_vector(MAX_ROW_ADDR, 6); --(others => '0');
         elsif (cnt_row_addr = conv_std_logic_vector(MAX_ROW_ADDR, 6) AND
             cnt_row_addr_ce = '1') then
            cnt_row_addr <= (others => '0');
         elsif (cnt_row_addr_ld = '1' AND cnt_row_addr_ce = '1') then
            cnt_row_addr <= (others => '0'); --conv_std_logic_vector(MAX_ROW_ADDR, 6);
         elsif (cnt_row_addr_ce = '1') then
            cnt_row_addr <= cnt_row_addr + '1';
         end if;
      end if;
   end process cnt_row_addr_p; 


   cnt_cells_alive_1_p : process(RESET, CLK, cnt_cells_alive_1_ce,
                                 cnt_cells_alive_1_ld)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            cnt_cells_alive_1 <= (others => '0');
         elsif (cnt_cells_alive_1_ld = '1' AND
                cnt_cells_alive_1_ce = '1') then
            cnt_cells_alive_1 <= (others => '0');
         elsif (conv_integer(cnt_cells_alive_1) = 8 AND
                cnt_cells_alive_1_ce = '1') then
            cnt_cells_alive_1 <= (others => '0');
         elsif (cnt_cells_alive_1_ce = '1') then
            cnt_cells_alive_1 <= cnt_cells_alive_1 + '1';
         end if;
      end if;
   end process cnt_cells_alive_1_p;


   -- status register of observed cell
   reg_cell_status_p : process(RESET, CLK, reg_cell_status_ce, INPUT_DATA)
   begin
      if (CLK'event AND CLK = '1') then
         if (RESET = '1') then
            reg_cell_status <= (others => '0');
         elsif (reg_cell_status_ce = '1') then
            reg_cell_status <= INPUT_DATA;
         end if;
      end if;
   end process reg_cell_status_p;

end architecture CA_LIFE_FSM_ARCH;

