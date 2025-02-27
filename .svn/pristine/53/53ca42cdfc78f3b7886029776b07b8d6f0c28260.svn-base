-- keyboard.vhd: Keyboard simulation file
-- Author(s): Tomas Martinek <martinto at fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity lcd_ctrl_high is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- user interface
   DATA        : in  std_logic_vector(7 downto 0);
   WRITE       : in  std_logic;
   CLEAR       : in  std_logic;

   -- lcd interface
   LRS         : out std_logic;
   LRW         : out std_logic;
   LE          : out std_logic;
   LD          : inout std_logic_vector(7 downto 0)
);
end entity lcd_ctrl_high;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of lcd_ctrl_high is

   constant ITEMS          : integer := 32;
   type   t_fifo is array(9 downto 0) of std_logic_vector(ITEMS-1 downto 0);
   type   t_state is (INIT, CLEAR_DISPLAY, CLEAR_CURSOR, SET_FUNCTION, 
                      CURSOR_OFF, SET_CURSOR_DIR, WAIT_FOR_REQ, WRITE_ADDR, 
                      WRITE_DATA);

   signal cnt_addr         : std_logic_vector(3 downto 0);
   signal cnt_addr_ce      : std_logic;
   signal cnt_addr_ld      : std_logic;
   signal lcd_busy         : std_logic;
   signal change_addr      : std_logic;
   signal write_req        : std_logic;
   signal clear_req        : std_logic;

   signal present_state, next_state : t_state;
   signal fsm_cnt_ce       : std_logic;
   signal fsm_cnt_clr      : std_logic;
   signal fsm_lcd_rs       : std_logic;
   signal fsm_fifo_rd      : std_logic;
   signal fsm_lcd_write    : std_logic;
   signal fsm_lcd_data     : std_logic_vector(7 downto 0);
   signal mx_lcd_addr      : std_logic_vector(7 downto 0);
   signal lcd_data         : std_logic_vector(7 downto 0);
   signal busy             : std_logic;

   signal fifo             : t_fifo;
   signal fifo_din         : std_logic_vector(9 downto 0);
   signal fifo_dout        : std_logic_vector(9 downto 0);
   signal fifo_rd          : std_logic;
   signal fifo_wr          : std_logic;
   signal fifo_empty       : std_logic;
   signal fifo_full        : std_logic;
   signal cnt_fifo_addr    : std_logic_vector(4 downto 0);
   signal cnt_fifo_count   : std_logic_vector(5 downto 0);
   signal cnt_fifo_addr_ce : std_logic;
   signal cnt_fifo_addr_updw : std_logic;

begin

-- cnt_addr counter
cnt_addr_ld <= fsm_cnt_clr;
cnt_addr_ce <= fsm_cnt_ce or fsm_cnt_clr;
process(RESET, CLK)
begin
   if (RESET = '1') then
      cnt_addr <= (others => '0');
   elsif (CLK'event AND CLK = '1') then
      if (cnt_addr_ce = '1') then
         if (cnt_addr_ld = '1') then
            cnt_addr <= (others => '0');
         else
            cnt_addr <= cnt_addr + 1;
         end if;
      end if;
   end if;
end process;

change_addr <= '1' when (cnt_addr(2 downto 0)="000") else '0';
mx_lcd_addr <= X"80"  when (cnt_addr(3) = '0') else X"C0";

-- ------------------------------------------------------------------
--                         FIFO Block 
-- ------------------------------------------------------------------
fifo_din    <= CLEAR & WRITE & DATA;
fifo_wr     <= WRITE or CLEAR;
fifo_rd     <= fsm_fifo_rd;

cnt_fifo_addr_ce <= (fifo_wr and (not fifo_rd)) or (fifo_rd and (not fifo_wr));
cnt_fifo_addr_updw <= fifo_rd;

-- cnt_fifo_addr counter
process(RESET, CLK)
begin
   if (RESET = '1') then
      cnt_fifo_addr <= (others => '1');
      cnt_fifo_count <= (others => '0');
   elsif (CLK'event AND CLK = '1') then
      if (cnt_fifo_addr_ce = '1') then
         if (cnt_fifo_addr_updw = '1') then
            cnt_fifo_addr <= cnt_fifo_addr - 1;
            cnt_fifo_count <= cnt_fifo_count - 1;
         else
            cnt_fifo_addr <= cnt_fifo_addr + 1;
            cnt_fifo_count <= cnt_fifo_count + 1;
         end if;
      end if;
   end if;
end process;

-- full/empty flag setting
fifo_full <= '1' when (cnt_fifo_count = conv_std_logic_vector(ITEMS,
                       cnt_fifo_count'length)) else '0';
fifo_empty <= '1' when (cnt_fifo_count = conv_std_logic_vector(0,
                        cnt_fifo_count'length)) else '0';

-- fifo memory
FIFO_U : for i in 0 to 9 generate 
   process(CLK)
   begin
      if (CLK'event and CLK='1') then
         if (fifo_wr='1') then
            fifo(i) <= fifo(i)(ITEMS-2 downto 0) & fifo_din(i);
         end if;
      end if;
   end process;
   fifo_dout(i) <= fifo(i)(conv_integer(cnt_fifo_addr));
end generate;

lcd_data  <= fifo_dout(7 downto 0);
write_req <= fifo_dout(8);
clear_req <= fifo_dout(9);

-- ------------------------------------------------------------------
--                        LCD controller    
-- ------------------------------------------------------------------
lcd_u : entity work.lcd_ctrl_rw
port map (
   RESET    => RESET,
   CLK      => CLK,

   -- interni rozhrani
   READ     => '0',
   WRITE    => fsm_lcd_write,
   RS       => fsm_lcd_rs,
   DIN      => fsm_lcd_data,
   DOUT     => open,
   DV       => open,
   BUSY     => busy,

   --- rozhrani LCD displeje
   LRS      => LRS,
   LRW      => LRW,
   LE       => LE,
   LD       => LD
);

lcd_busy <= busy;
-- ------------------------------------------------------------------
--                       Finite State Machine
-- ------------------------------------------------------------------

-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= INIT;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, lcd_busy, change_addr, write_req,
                           clear_req, fifo_empty)
begin
   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when INIT => 
      next_state <= CLEAR_CURSOR;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CLEAR_CURSOR => 
      next_state <= CLEAR_CURSOR;
      if (lcd_busy = '0') then
         next_state <= SET_FUNCTION;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when SET_FUNCTION => 
      next_state <= SET_FUNCTION;
      if (lcd_busy = '0') then
         next_state <= CURSOR_OFF;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CURSOR_OFF => 
      next_state <= CURSOR_OFF;
      if (lcd_busy = '0') then
         next_state <= SET_CURSOR_DIR;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when SET_CURSOR_DIR => 
      next_state <= SET_CURSOR_DIR;
      if (lcd_busy = '0') then
         next_state <= CLEAR_DISPLAY;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CLEAR_DISPLAY => 
      next_state <= CLEAR_DISPLAY;
      if (lcd_busy = '0') then
         next_state <= WAIT_FOR_REQ;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WAIT_FOR_REQ => 
      next_state <= WAIT_FOR_REQ;
      if (fifo_empty = '0') then
         if (write_req = '1') then
            if (change_addr = '1') then
               next_state <= WRITE_ADDR;
            else
               next_state <= WRITE_DATA;
            end if;
         elsif (clear_req = '1') then
            next_state <= CLEAR_DISPLAY;
         end if;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_ADDR => 
      next_state <= WRITE_ADDR;
      if (lcd_busy = '0') then
         next_state <= WRITE_DATA;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_DATA  => 
      next_state <= WRITE_DATA;
      if (lcd_busy = '0') then
         next_state <= WAIT_FOR_REQ;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, lcd_busy, lcd_data,
                       mx_lcd_addr, fifo_empty, clear_req)
begin
   fsm_cnt_ce        <= '0';
   fsm_cnt_clr       <= '0';
   fsm_lcd_rs        <= '0';
   fsm_lcd_write     <= '0';
   fsm_fifo_rd       <= '0';
   fsm_lcd_data      <= (others => '0');

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when INIT => 
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CLEAR_CURSOR => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data  <= X"02";
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when SET_FUNCTION => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data  <= X"38";
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CURSOR_OFF => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data  <= X"0C";
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when SET_CURSOR_DIR => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data  <= X"06";
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when CLEAR_DISPLAY => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data  <= X"01";
         fsm_cnt_clr   <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WAIT_FOR_REQ => 
      if (fifo_empty = '0' and clear_req = '1') then
         fsm_fifo_rd <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_ADDR => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_lcd_data <= mx_lcd_addr;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when WRITE_DATA => 
      if (lcd_busy = '0') then
         fsm_lcd_write <= '1';
         fsm_cnt_ce    <= '1';
         fsm_lcd_rs    <= '1';
         fsm_fifo_rd   <= '1';
         fsm_lcd_data  <= lcd_data;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
   end case;
end process output_logic;

end architecture behavioral;

