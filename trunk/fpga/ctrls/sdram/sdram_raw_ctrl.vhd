-- sdram.vhd: SDRAM base controller
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

-- SYNTH-ISE-9.2: slices=127, slicesFF=151, 4luts=169
entity sdram_raw_controller is
   generic (
      -- Generovani prikazu refresh radicem automaticky
      GEN_AUTO_REFRESH : boolean := true;
      OPTIMIZE_REFRESH : sdram_optimize := oAlone -- deprecated
      );
   port (
      -- Clock, reset, ...
      CLK     : in std_logic;
      RST     : in std_logic;
      ENABLE  : in std_logic;
      BUSY    : out std_logic;

      -- Address/data
      ADDR_ROW    : in std_logic_vector(11 downto 0);
      ADDR_COLUMN : in std_logic_vector(8 downto 0);
      BANK        : in std_logic_vector(1 downto 0);
      DATA_IN     : in std_logic_vector(7 downto 0);
      DATA_OUT    : out std_logic_vector(7 downto 0);
      DATA_VLD    : out std_logic; -- Output data valid

      -- Command signal/set
      CMD     : in sdram_func;
      CMD_WE  : in std_logic;

      -- Signals to SDRAM
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
end sdram_raw_controller;

architecture arch_sdram_raw_controller of sdram_raw_controller is

   type tstate is (stReset,
                   stIdle,
                   stCmdWait,
                   stCmdLMR,
                   stCmdSelect,
                   stCmdRead,
                   stCmdWrite,
                   stCmdCharge,
                   stCmdRefresh);

   type tistate is (csi_Begin,
                    csi_Idle,
                    csi_Chg,
                    csi_Ref1,
                    csi_Ref2,
                    csi_Ref3,
                    csi_Ref4,
                    csi_Ref5,
                    csi_Ref6,
                    csi_Ref7,
                    csi_LMR,
                    csi_End);

   type tcmd_len is (t1clk, t2clk, t3clk, t7clk);

   -- Init FSM states
   signal cstate_init : tistate;
   signal nstate_init : tistate;

   -- RAM command (RCKE RDQM CS RAS CAS WE)
   signal ram_cmd   : std_logic_vector(5 downto 0);
   signal ram_a_sel : std_logic_vector(1 downto 0);

   -- Controler state (current/next/future)
   signal cstate    : tstate;
   signal nstate    : tstate;

   signal iready    : std_logic;

   -- State timing counter
   signal cnt       : std_logic_vector(2 downto 0);
   signal cnt_set   : std_logic_vector(2 downto 0);
   signal cnt_mx    : std_logic;
   signal cnt_load  : std_logic;
   signal cnt_value : tcmd_len;

   -- RAM controller command
   signal cmd_fifo_reg   : sdram_func;
   signal cmd_loaded     : std_logic;
   signal cmd_ld_xor1    : std_logic;
   signal cmd_ld_xor2    : std_logic;
   signal cmd_ld_en      : std_logic;
   signal cmd_as_state   : tstate;

   -- Fifo row/col/bank, data
   signal row_fifo_reg : std_logic_vector(11 downto 0);
   signal col_fifo_reg : std_logic_vector(8 downto 0);
   signal bnk_fifo_reg : std_logic_vector(1 downto 0);
   signal din_fifo_reg : std_logic_vector(7 downto 0);

   signal dout_reg     : std_logic_vector(7 downto 0);

   signal dout_ready  : std_logic;
   signal din_ready   : std_logic;

   signal row_bus_reg    : std_logic_vector(11 downto 0);
   signal column_bus_reg : std_logic_vector(8 downto 0);
   signal bank_bus_reg   : std_logic_vector(1 downto 0);
   signal data_bus_reg   : std_logic_vector(7 downto 0);

   -- Refresh request
   signal ref_mx       : std_logic;
   signal ref_cnt      : std_logic_vector(9 downto 0);

   -- Read latency 2 shift (latency 3 need 4 bit shift)
   signal lat_sh       : std_logic_vector(2 downto 0) := "000";

   -- Additional FSM state logic
   signal cs_cc        : std_logic;
   signal cs_cf        : std_logic;
   signal cs_cs        : std_logic;
   signal cs_cr        : std_logic;
   signal cs_cw        : std_logic;

   -- Generic to logic translation
   signal GENERIC_GAF  : std_logic;

   -- Init logic
   signal init_p0 : std_logic;
   signal init_p1 : std_logic;
   signal init_p2 : std_logic;
   signal inited  : std_logic;

   -- External operation
   signal cf_crw  : std_logic;
   signal cf_cw   : std_logic;
   signal cf_cr   : std_logic;
   signal cf_cf   : std_logic;

   -- Active bank's row
   signal actrows  : std_logic_vector(51 downto 0);
   signal actrow   : std_logic_vector(12 downto 0);
   signal arow     : std_logic_vector(12 downto 0);
   signal resel_mx : std_logic;
   signal bnk1     : std_logic;
   signal bnk2     : std_logic;
   signal bnk3     : std_logic;
   signal bnk4     : std_logic;

   -- State signals/State regs
   signal cha     : std_logic;
   signal chg     : std_logic;
   signal chg_reg : std_logic;
   signal ref     : std_logic;
   signal ref_reg : std_logic;
   signal sel     : std_logic;
   signal sel_reg : std_logic;
   signal fff     : std_logic;
   signal fff_reg : std_logic;
   signal out_ld  : std_logic;
   signal out_reg : std_logic;

   signal waw     : std_logic;
   signal rar     : std_logic;
   signal raw     : std_logic;

begin

   -- Generic GEN_AUTO_REFRESH true/false -> 0/1
   GENERIC_GAF <= '1' when GEN_AUTO_REFRESH = true else '0';

   RAM_CLK  <= CLK;
   RAM_CKE  <= ram_cmd(5);
   RAM_DQM  <= ram_cmd(4);
   RAM_CS   <= ram_cmd(3);
   RAM_RAS  <= ram_cmd(2);
   RAM_CAS  <= ram_cmd(1);
   RAM_WE   <= ram_cmd(0);
   BUSY     <= not iready;
   DATA_VLD <= lat_sh(2); -- Read data valid after latency logic
   DATA_OUT <= dout_reg;

   -- Set data for RAM (RD)
   RAM_D    <= data_bus_reg when din_ready = '1' else (others => 'Z');

   -- Busy logic
   iready <= inited and not fff_reg;

   -- Idle count logic
   cnt_mx  <= cnt(0) and cnt(1) and cnt(2);     -- cnt=7
   cnt_set <= "001" when (cnt_value = t7clk) else
              "101" when (cnt_value = t3clk) else
              "110" when (cnt_value = t2clk) else
              "111" when (cnt_value = t1clk) else
              "111";

   -- Cmd load logic
   cmd_loaded <= cmd_ld_xor1 xor cmd_ld_xor2;
   cmd_ld_en  <= CMD_WE and iready and ENABLE;

   -- Bank selection
   bnk1 <= chg_reg or sel_reg when bnk_fifo_reg = "00" else ref_reg;
   bnk2 <= chg_reg or sel_reg when bnk_fifo_reg = "01" else ref_reg;
   bnk3 <= chg_reg or sel_reg when bnk_fifo_reg = "10" else ref_reg;
   bnk4 <= chg_reg or sel_reg when bnk_fifo_reg = "11" else ref_reg;

   -- Row selected in next command mx
   actrow <= actrows(12 downto 0)  when bnk_fifo_reg = "00"
        else actrows(25 downto 13) when bnk_fifo_reg = "01"
        else actrows(38 downto 26) when bnk_fifo_reg = "10"
        else actrows(51 downto 39) when bnk_fifo_reg = "11";

   -- Need row reselection?
   resel_mx <= '1' when actrow(11 downto 0) /= row_fifo_reg else '0';

   -----------------------------------------------------------------------------
   -- Autorefresh gen counter limit (10 bit): 1100000000 = 15.360 us (50 MHz)
   ref_mx <= (ref_cnt(9) and ref_cnt(8) and GENERIC_GAF);

   -----------------------------------------------------------------------------
   -- Input command logic
   cmd_as_state <= stCmdRead    when cmd_fifo_reg = fRead
              else stCmdWrite   when cmd_fifo_reg = fWrite
              else stCmdRefresh when cmd_fifo_reg = fRefresh
              else stCmdWait;

   -----------------------------------------------------------------------------
   -- Next (FIFO) opration logic
   cf_cr  <= '1' when cmd_fifo_reg = fRead else '0';
   cf_cw  <= '1' when cmd_fifo_reg = fWrite else '0';
   cf_cf  <= '1' when cmd_fifo_reg = fRefresh else '0';
   cf_crw <= cf_cr or cf_cw;

   -----------------------------------------------------------------------------
   -- State flag logic

   -- Charge all rows when need refresh
   cha <= ((cf_cf and cmd_loaded) or (ref_mx and not fff_reg)) and
          (actrows(12) or actrows(25) or actrows(38) or actrows(51));

   -- Charge if ram will be refreshed OR loaded RD/WR & INCORRECT ROW SELECTED
   chg <= cha or (cf_crw and resel_mx and cmd_loaded and actrow(12));

   -- Refresh flag
   ref <= (ref_mx and not fff_reg) or (cf_cf and cmd_loaded);

   -- Select if loaded RD/WR and reselect/nothing selected
   sel <= cf_crw and (resel_mx or not actrow(12)) and cmd_loaded;

   -- Fifo full if chg OR sel flag set OR
   --     (not prepare command finished AND operation in FIFO can't be optimized
   --      by read-after-read, write-after-write or read-after-write techniques)
   --fff <= chg or sel or (not rar and not waw and not raw and cmd_loaded) or not cnt_mx;
   fff <= chg or sel or (not rar and not waw and not raw and cmd_loaded) or (not cnt_mx and cmd_loaded);

   -- Optimize flags (read-after-read, write-after-write, read-after-write)
   -- WARN: when R/W state was finished and IDLE now take latency time, flags aren't set
   rar <= cs_cr and cf_cr and not resel_mx and cmd_loaded;
   waw <= cs_cw and cf_cw and not resel_mx and cmd_loaded;
   raw <= cs_cw and cf_cr and not resel_mx and cmd_loaded;

   out_ld <= (out_reg and not fff_reg) or rar or waw or raw;

   -- row is unselected when REFRESH flasg set
   arow <= (not ref_reg) & row_bus_reg; --'1' & row_bus_reg when ref_reg = '0' else (others => '0');

   -----------------------------------------------------------------------------
   -- State flag registers
   state_registers: process(RST, CLK)
   begin
      if RST = '1' then
         chg_reg <= '0';
         ref_reg <= '0';
         sel_reg <= '0';
         fff_reg <= '0';
         out_reg <= '0';
         actrows <= (others => '0');
      elsif CLK'event and CLK = '1' then
         chg_reg <= (chg_reg and not cs_cc) or chg; -- Simple charge flag
         ref_reg <= (ref_reg and not cs_cf) or ref; -- Refresh flag
         sel_reg <= (sel_reg and not cs_cs) or sel; -- Select flag

         -- cnt_ld must be set only when can't be rar/waw/raw
         fff_reg <= (fff_reg and (sel_reg or chg_reg or not cnt_mx)) or fff;
         out_reg <= fff_reg;

         if bnk1 = '1' then
            actrows(12 downto 0) <= arow;
         end if;
         if bnk2 = '1' then
            actrows(25 downto 13) <= arow;
         end if;
         if bnk3 = '1' then
            actrows(38 downto 26) <= arow;
         end if;
         if bnk4 = '1' then
            actrows(51 downto 39) <= arow;
         end if;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- CMD_loaded control register
   cmd_ld_reg: process(RST, CLK, cmd_ld_xor1)
   begin
      if RST = '1' then
         cmd_ld_xor2 <= cmd_ld_xor1;
      elsif (CLK'event and CLK = '0') then
         cmd_ld_xor2 <= cmd_ld_xor1;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Data from FIFO to SDRAM output
   data_to_bus: process(RST, CLK)
   begin
      if RST = '1' then
         data_bus_reg   <= (others => '0');
         column_bus_reg <= (others => '0');
         bank_bus_reg   <= (others => '0');
         row_bus_reg    <= (others => '0');
      elsif (CLK'event and CLK = '0') then
         if cmd_loaded = '1' then
            data_bus_reg   <= din_fifo_reg;
            column_bus_reg <= col_fifo_reg;
            bank_bus_reg   <= bnk_fifo_reg;
            row_bus_reg    <= row_fifo_reg;
         end if;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Data from input to FIFO, load control flag
   cmd_ld_work: process(RST, CLK)
   begin
      if RST = '1' then
         cmd_fifo_reg <= fNop;
         bnk_fifo_reg <= (others => '0');
         row_fifo_reg <= (others => '0');
         col_fifo_reg <= (others => '0');
         din_fifo_reg <= (others => '0');
         cmd_ld_xor1  <= '0';
      elsif (CLK'event and CLK = '0') then
         if cmd_ld_en = '1' then
            cmd_fifo_reg <= CMD;
            bnk_fifo_reg <= BANK;
            col_fifo_reg <= ADDR_COLUMN;
            row_fifo_reg <= ADDR_ROW;
            din_fifo_reg <= DATA_IN;
            cmd_ld_xor1  <= not cmd_ld_xor1;
         end if;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Auto refresh generator
   ram_refresh_gen: process(RST, CLK)
   begin
      if RST = '1' then
         ref_cnt <= (others => '0');
      elsif CLK'event and CLK = '0' then
         if GEN_AUTO_REFRESH = true then
            if ref_reg = '1' then -- When ref flag set, reset counter
                ref_cnt <= (others => '0');
            elsif ref_mx = '0' then
                ref_cnt <= ref_cnt + 1;
            end if;
         end if;
      end if;
   end process;

   -- Command idle timer
   ram_cnt: process(RST, CLK)
   begin
      if RST = '1' then
         cnt <= (others => '1');
      elsif CLK'event and CLK = '0' then
         if cnt_load = '1' then
            cnt <= cnt_set;
         elsif cnt_mx = '0' then
            cnt <= cnt + 1;
         end if;
       end if;
   end process;

   -- Data valid shifter (latency), output reg write
   ram_read_flag: process(RST, CLK)
   begin
      if RST = '1' then
         dout_reg <= (others => '0');
      elsif CLK'event and CLK = '1' then
         -- Output reg set
         if lat_sh(1) = '1' then
            dout_reg <= RAM_D;
         end if;
         -- Latency shift
         lat_sh <= lat_sh(1 downto 0) & dout_ready;
      end if;
   end process;

   -----------------------------------------------------------------------------
   -- Init FSM
   fsm_init_ctrl: process(RST, CLK)
   begin
      if RST = '1' then
         cstate_init <= csi_Begin;
      elsif CLK'event and CLK = '0' then
         cstate_init <= nstate_init;
      end if;
   end process;

   fsm_init: process(cstate_init)
   begin
      init_p0 <= '0';
      init_p1 <= '0';
      init_p2 <= '0';
      inited  <= '0';

      case cstate_init is
         when csi_Begin => null;
         when csi_Idle  => init_p0 <= '1';
         when csi_Chg   => init_p0 <= '1';
         when csi_Ref1  => init_p1 <= '1';
         when csi_Ref2  => init_p1 <= '1';
         when csi_Ref3  => init_p1 <= '1';
         when csi_Ref4  => init_p1 <= '1';
         when csi_Ref5  => init_p1 <= '1';
         when csi_Ref6  => init_p1 <= '1';
         when csi_Ref7  => init_p1 <= '1';
         when csi_LMR   => init_p2 <= '1';
         when csi_End   => inited  <= '1';
      end case;
   end process;

   fsm_init_ns: process(cstate_init, cnt_load)
   begin
      nstate_init <= csi_Begin;

      case cstate_init is
         when csi_Begin => nstate_init <= csi_Idle;
         when csi_Idle => nstate_init <= csi_Chg;
         when csi_Chg =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref1;
            else
               nstate_init <= csi_Chg;
            end if;
         when csi_Ref1 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref2;
            else
               nstate_init <= csi_Ref1;
            end if;
         when csi_Ref2 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref3;
            else
               nstate_init <= csi_Ref2;
            end if;
         when csi_Ref3 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref4;
            else
               nstate_init <= csi_Ref3;
            end if;
         when csi_Ref4 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref5;
            else
               nstate_init <= csi_Ref4;
            end if;
         when csi_Ref5 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref6;
            else
               nstate_init <= csi_Ref5;
            end if;
         when csi_Ref6 =>
            if cnt_load = '1' then
               nstate_init <= csi_Ref7;
            else
               nstate_init <= csi_Ref6;
            end if;
         when csi_Ref7 =>
            if cnt_load = '1' then
               nstate_init <= csi_LMR;
            else
               nstate_init <= csi_Ref7;
            end if;
         when csi_LMR =>
            if cnt_load = '1' then
               nstate_init <= csi_End;
            else
               nstate_init <= csi_LMR;
            end if;
         when csi_End => nstate_init <= csi_End;
      end case;
   end process;

   -----------------------------------------------------------------------------
   -- Muxes

   -- Latency 2, Sequential, Burst Len 1B, Single Location Access
   RAM_A <= (9 => '1', 5 => '1', others => '0')               when ram_a_sel = "00"
       else bank_bus_reg & '0' & (not ref_reg) & "0000000000" when ram_a_sel = "01"
       else bank_bus_reg & row_bus_reg                        when ram_a_sel = "10"
       else bank_bus_reg & "000" & column_bus_reg             when ram_a_sel = "11";

   -----------------------------------------------------------------------------
   -- Main FSM control
   ram_FSM_ctrl: process(RST, CLK)
   begin
      if RST = '1' then
         cstate <= stReset;
      elsif CLK'event and CLK = '0' then
         cstate <= nstate;
      end if;
   end process;

   -- Main FSM logic
   ram_FSM: process(cstate, rar, waw, raw)
   begin
      -- Default
      ram_cmd    <= "111111";        -- Idle (RCKE RDQM CS RAS CAS WE)
      cnt_value  <= t1clk;           -- Counter value
      cnt_load   <= '0';             -- No counter set value
      dout_ready <= '0';             -- No data reading (from RAM)
      din_ready  <= '0';             -- No data to write (to RAM)
      ram_a_sel  <= (others => '0');
      -- CurrentState_Cmd(Charge,reFresh,Select,Idle)
      cs_cc      <= '0';
      cs_cf      <= '0';
      cs_cs      <= '0';
      cs_cr      <= '0';
      cs_cw      <= '0';

      case cstate is
         when stIdle  => null;

         when stReset => -- Start
            ram_cmd   <= "011111"; -- RCKE RDQM CS RAS CAS WE

         when stCmdLMR => -- Load mode register (OP-CODE)
            ram_cmd   <= "110000";
            ram_a_sel <= "00";
            cnt_value <= t1clk;
            cnt_load  <= '1';

         when stCmdWait => -- External command waiting
            null;

         when stCmdCharge => -- Precharge
            cs_cc     <= '1'; -- Current state charge identify
            ram_cmd   <= "110010";
            ram_a_sel <= "01";
            cnt_value <= t2clk;
            cnt_load  <= '1';

         when stCmdRefresh => -- Refresh
            cs_cf     <= '1'; -- Current state refresh identify
            ram_cmd   <= "110001";
            cnt_value <= t7clk;
            cnt_load  <= '1';

         when stCmdSelect => -- Select bank/row
            cs_cs     <= '1'; -- Current state select identify
            ram_cmd   <= "110011";
            ram_a_sel <= "10";
            cnt_value <= t3clk;
            cnt_load  <= '1';

         when stCmdRead => -- Read byte bank/column
            cs_cr      <= '1';
            ram_cmd    <= "100101";
            ram_a_sel  <= "11";
            dout_ready <= '1';
            cnt_value  <= t2clk;
            cnt_load   <= not rar;

         when stCmdWrite => -- Write byte bank/column
            cs_cw     <= '1';
            ram_cmd   <= "100100";
            ram_a_sel <= "11";
            din_ready <= '1';
            cnt_value <= t1clk;
            cnt_load  <= not (waw or raw);

      end case;
   end process;

   -- Main FSM state control
   ram_FSM_ns: process(cstate, cnt_mx, rar, waw, raw, init_p0, chg_reg, init_p1, ref_reg, init_p2, sel_reg, out_ld, cmd_as_state)
   begin
      -- Default
      nstate <= stReset; -- Future state, after actual finished

      case cstate is
         when stIdle  =>
            if (not cnt_mx and not (rar or waw or raw)) = '1' then
               nstate <= stIdle;
            elsif (init_p0 or chg_reg) = '1' then
               nstate <= stCmdCharge;
            elsif (init_p1 or ref_reg) = '1' then
               nstate <= stCmdRefresh;
            elsif init_p2 = '1' then
               nstate <= stCmdLMR;
            elsif sel_reg = '1' then
               nstate <= stCmdSelect;
            elsif out_ld = '1' then
               nstate <= cmd_as_state;
            else
               nstate <= stCmdWait;
            end if;

         when stReset =>      nstate <= stIdle;
         when stCmdLMR =>     nstate <= stIdle;
         when stCmdCharge =>  nstate <= stIdle;
         when stCmdRefresh => nstate <= stIdle;
         when stCmdSelect =>  nstate <= stIdle;

         when stCmdWait => -- External command waiting
            if chg_reg = '1' then
               nstate <= stCmdCharge;
            elsif ref_reg = '1' then
               nstate <= stCmdRefresh;
            elsif sel_reg = '1' then
               nstate <= stCmdSelect;
            elsif out_ld = '1' then
               nstate <= cmd_as_state;
            else
               nstate <= stCmdWait;
            end if;

         when stCmdRead => -- Read byte bank/column
            if rar = '1' then
               nstate <= stCmdRead; -- Read after read
            else
               nstate <= stIdle;
            end if;

         when stCmdWrite => -- Write byte bank/column
            if (waw or raw) = '1' then
               nstate <= cmd_as_state; -- Write/read after write
            else
               nstate <= stIdle;
            end if;

      end case;
   end process;

end arch_sdram_raw_controller;
