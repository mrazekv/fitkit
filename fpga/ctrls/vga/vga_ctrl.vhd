-- vga_ctrl.vhd: VGA base controller
-- Copyright (C) 2006 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ladislav Capka <xcapka01 AT @stud.fit.vutbr.cz>
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

-- SYNTH-ISE-8.1: slices=87, slicesFF=34, 4luts=159
-- SYNTH-ISE-8.2: slices=81, slicesFF=31, 4luts=152
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.conv_std_logic_vector;
use ieee.std_logic_unsigned.all;
use work.vga_controller_cfg.all;

entity vga_controller is
   generic (
      REQ_DELAY : natural := 1
      );
   port (
      -- Hodiny + reset
      CLK     : in std_logic;
      RST     : in std_logic;
      ENABLE  : in std_logic;

      -- Nastaveni rozliseni (popis v VGA_CONFIG.VHD)
      -- Pozor: Hodnota musi zustat stale platna!
      MODE    : in std_logic_vector(60 downto 0);

      -- Vstup / pozadavek
      DATA_RED    : in  std_logic_vector(2 downto 0);
      DATA_GREEN  : in  std_logic_vector(2 downto 0);
      DATA_BLUE   : in  std_logic_vector(2 downto 0);
      ADDR_COLUMN : out std_logic_vector(11 downto 0);
      ADDR_ROW    : out std_logic_vector(11 downto 0);

      -- Vystup na VGA
      VGA_RED   : out std_logic_vector(2 downto 0);
      VGA_GREEN : out std_logic_vector(2 downto 0);
      VGA_BLUE  : out std_logic_vector(2 downto 0);
      VGA_HSYNC : out std_logic;
      VGA_VSYNC : out std_logic;
      
      -- H/V Status
      STATUS_H  : out vga_hfsm_state;
      STATUS_V  : out vga_vfsm_state     
      );
end vga_controller;

architecture arch_vga_controller of vga_controller is

   -- Aktivace radice
   signal ienable     : std_logic := '1';

   -- Vnitrni reset radice
   signal ireset      : std_logic := '1';

   -- Horizontalni hodnoty
   signal h_pixels    : std_logic_vector(11 downto 0);
   signal h_syncstart : std_logic_vector(6 downto 0);
   signal h_syncend   : std_logic_vector(6 downto 0);
   signal h_period    : std_logic_vector(6 downto 0);

   -- Vertikalni hodnoty
   signal v_lines     : std_logic_vector(11 downto 0);
   signal v_syncstart : std_logic_vector(5 downto 0);
   signal v_syncend   : std_logic_vector(3 downto 0);
   signal v_period    : std_logic_vector(5 downto 0);

   -- MX pro citace
   signal req_delay_mx   : std_logic;
   signal h_pixels_mx    : std_logic;
   signal v_lines_mx     : std_logic;
   signal h_syncstart_mx : std_logic;
   signal h_syncend_mx   : std_logic;
   signal h_period_mx    : std_logic;
   signal v_syncstart_mx : std_logic;
   signal v_syncend_mx   : std_logic;
   signal v_period_mx    : std_logic;

   -- H/V citac
   signal hcnt        : std_logic_vector(11 downto 0) := (others => '0');
   signal vcnt        : std_logic_vector(11 downto 0) := (others => '0');

   -- Reset H/V citace
   signal hcnt_reset  : std_logic := '1';
   signal vcnt_reset  : std_logic := '1';

   -- Aktivace V citace
   signal vcnt_en     : std_logic := '0';

   -- Aktivace vstupu/vystupu VGA radice
   signal out_en      : std_logic_vector(1 downto 0);
   signal req_en      : std_logic_vector(1 downto 0);
   signal use_rqdelay : std_logic := '0';

   -- H/V synchronizacni puls
   signal hsync_en    : std_logic := '0';
   signal vsync_en    : std_logic := '0';

   -- Stavy FSMs
   signal hfsm_cst    : vga_hfsm_state;
   signal hfsm_nst    : vga_hfsm_state;
   signal vfsm_cst    : vga_vfsm_state;
   signal vfsm_nst    : vga_vfsm_state;
   
begin

   -- Nastaveni
   h_pixels    <= MODE(60 downto 49);
   v_lines     <= MODE(48 downto 37);
   h_syncstart <= MODE(36 downto 30);
   h_syncend   <= MODE(29 downto 23);
   h_period    <= MODE(22 downto 16);
   v_syncstart <= MODE(15 downto 10);
   v_syncend   <= MODE(9 downto 6);
   v_period    <= MODE(5 downto 0);

   STATUS_H  <= hfsm_cst;
   STATUS_V  <= vfsm_cst;

   VGA_RED   <= DATA_RED   when out_en = "11" else (others => '0');
   VGA_GREEN <= DATA_GREEN when out_en = "11" else (others => '0');
   VGA_BLUE  <= DATA_BLUE  when out_en = "11" else (others => '0');
   VGA_HSYNC <= not hsync_en;
   VGA_VSYNC <= not vsync_en;

   ADDR_COLUMN <= hcnt when req_en = "11" else (others => '0');
   ADDR_ROW    <= vcnt when req_en(1) = '1' else (others => '0');

   ireset <= '1' when (RST = '1') or (ienable = '0') else '0';
   use_rqdelay <= '0' when REQ_DELAY = 0 else '1';

   enable_sync: process (CLK)
   begin
      if CLK'event and CLK = '1' then
         ienable <= ENABLE;
      end if;
   end process;

   fsm_control: process(CLK, ireset)
   begin
      if (ireset = '1') then
         hfsm_cst <= hstDisabled;
         vfsm_cst <= vstPrepare;
      elsif CLK'event and CLK = '1' then
         hfsm_cst <= hfsm_nst; -- Horizontal FSM
         vfsm_cst <= vfsm_nst; -- Vertical FSM
      end if;
   end process;

   line_fsm: process(hfsm_cst, hcnt, ireset, req_delay_mx, h_pixels_mx, h_syncstart_mx, h_syncend_mx, h_period_mx)
   begin
      -- Default
      hfsm_nst   <= hstDisabled;  -- Dalsi stav = radic neaktivni
      vcnt_en    <= '0';          -- V citac zastaven
      out_en(0)  <= '0';          -- VGA vystup neaktivni
      req_en(0)  <= '0';          -- Vstupni pozadavky neaktivni
      hsync_en   <= '0';          -- H sync neaktivni
      hcnt_reset <= '0';          -- Neresetujeme citac

      case hfsm_cst is
         when hstDisabled => -- Radic neaktivni
              hcnt_reset <= '1';
              if ireset = '0' then
                 hfsm_nst <= hstPrepare;
              else
                 hfsm_nst <= hstDisabled;
              end if;

         when hstPrepare => -- Citace online
              hcnt_reset <= '1';
              if use_rqdelay = '0' then
                 hfsm_nst  <= hstDraw;
              else
                 hfsm_nst  <= hstRequestOnly;
              end if;

         when hstRequestOnly => -- Cekani REQ_DELAY pred kreslenim
              req_en(0) <= '1';
              if req_delay_mx = '1' then
                 hfsm_nst <= hstDraw;
              else
                 hfsm_nst <= hstRequestOnly;
              end if;

         when hstDraw => -- Kresleni
              out_en(0) <= '1';
              req_en(0) <= '1';
              if h_pixels_mx = '1' then
                 hcnt_reset <= '1';
                 if use_rqdelay = '0' then
                    hfsm_nst  <= hstWaitSync;
                 else
                    hfsm_nst  <= hstDrawOnly;
                 end if;
              else
                 hfsm_nst <= hstDraw;
              end if;

         when hstDrawOnly => -- Konec kresleni se spozdenim REQ_DELAY
              out_en(0) <= '1';
              if req_delay_mx = '1' then
                 hcnt_reset <= '1';
                 hfsm_nst <= hstWaitSync;
              else
                 hfsm_nst <= hstDrawOnly;
              end if;

         when hstWaitSync => -- Cekani pred H sync
              if h_syncstart_mx = '1' then
                 hcnt_reset <= '1';
                 hfsm_nst <= hstSync;
              else
                 hfsm_nst <= hstWaitSync;
              end if;

         when hstSync => -- H synchronizacni pulz
              hsync_en <= '1';
              if h_syncend_mx = '1' then
                 hcnt_reset <= '1';
                 vcnt_en  <= '1';
                 hfsm_nst <= hstWaitPeriod;
              else
                 hfsm_nst <= hstSync;
              end if;

         when hstWaitPeriod => -- Cekani po H sync
              if h_period_mx = '1' then
                 hcnt_reset <= '1';
                 if use_rqdelay = '0' then
                    hfsm_nst  <= hstDraw;
                 else
                    hfsm_nst  <= hstRequestOnly;
                 end if;
              else
                 hfsm_nst <= hstWaitPeriod;
              end if;
      end case;
   end process;

   frame_fsm: process(vfsm_cst, vcnt, ireset, v_lines_mx, v_syncstart_mx, v_syncend_mx, v_period_mx)
   begin
      vfsm_nst   <= vstPrepare;
      out_en(1)  <= '0';
      req_en(1)  <= '0';
      vcnt_reset <= '0';
      vsync_en   <= '0';

      case vfsm_cst is
         when vstPrepare => -- Start
              vcnt_reset <= '1';
              req_en(1)  <= '1';
              out_en(1)  <= '1';
              vfsm_nst   <= vstDraw;

         when vstDraw => -- Kresleni
              if v_lines_mx = '1' then
                 vcnt_reset <= '1';
                 vfsm_nst   <= vstWaitSync;
              else
                 req_en(1)  <= '1';
                 out_en(1)  <= '1';
                 vfsm_nst   <= vstDraw;
              end if;

         when vstWaitSync => -- Cekani pred V sync
              if v_syncstart_mx = '1' then
                 vcnt_reset <= '1';
                 vsync_en   <= '1';
                 vfsm_nst   <= vstSync;
              else
                 vfsm_nst <= vstWaitSync;
              end if;

         when vstSync => -- V synchronizacni pulz
              if v_syncend_mx = '1' then
                 vcnt_reset <= '1';
                 vsync_en <= '0';
                 vfsm_nst <= vstWaitPeriod;
              else
                 vsync_en <= '1';
                 vfsm_nst <= vstSync;
              end if;

         when vstWaitPeriod => -- Cekani po V sync
              if v_period_mx = '1' then
                 vcnt_reset <= '1';
                 req_en(1) <= '1';
                 out_en(1) <= '1';
                 vfsm_nst <= vstDraw;
              else
                 vfsm_nst <= vstWaitPeriod;
              end if;
      end case;
   end process;

   -- Comparators
   req_delay_mx   <= use_rqdelay when hcnt(2 downto 0) = conv_std_logic_vector(REQ_DELAY-1, 3) else '0';
   h_pixels_mx    <= '1' when hcnt(11 downto 0) = h_pixels else '0';
   v_lines_mx     <= '1' when vcnt(11 downto 0) = v_lines else '0';
   h_syncstart_mx <= '1' when hcnt(6 downto 0)  = h_syncstart else '0';
   h_syncend_mx   <= '1' when hcnt(6 downto 0)  = h_syncend else '0';
   h_period_mx    <= '1' when hcnt(6 downto 0)  = h_period else '0';
   v_syncstart_mx <= '1' when vcnt(5 downto 0)  = v_syncstart else '0';
   v_syncend_mx   <= '1' when vcnt(3 downto 0)  = v_syncend else '0';
   v_period_mx    <= '1' when vcnt(5 downto 0)  = v_period else '0';

   -- Horizontal/vertical counter
   hcnt_ctrl: process(CLK, ireset, hcnt_reset)
   begin
      if (ireset = '1') then
         hcnt <= (others => '0');
      elsif (CLK'event and CLK = '1') then
         if hcnt_reset = '1' then
            hcnt <= (others => '0');
         else
            hcnt <= hcnt + 1;
         end if;
      end if;
   end process;

   vcnt_ctrl: process(CLK, ireset, vcnt_reset, vcnt_en)
   begin
      if (ireset = '1') then
         vcnt <= (others => '0');
      elsif (CLK'event and CLK = '1') then
         if (vcnt_reset = '1') then
            vcnt <= (others => '0');
         elsif vcnt_en = '1' then
            vcnt <= vcnt + 1;
         end if;
      end if;
   end process;

end arch_vga_controller;
