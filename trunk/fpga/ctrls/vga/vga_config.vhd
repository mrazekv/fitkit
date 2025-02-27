-- vga_config.vhd: VGA constants & definitions
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

package vga_controller_cfg is
   type modes is (rUnknown,
                  r640x480x60, r640x480x66, --r640x480x72, r640x480x75,
                  --r720x400x70, r720x350x70,
                  --r800x600x56, r800x600x60,
                  r800x600x72);
   
   type vga_hfsm_state is (hstDisabled, hstPrepare, hstRequestOnly, hstDraw,
                       hstDrawOnly, hstWaitSync, hstSync, hstWaitPeriod);
   type vga_vfsm_state is (vstPrepare, vstDraw, vstWaitSync, vstSync, vstWaitPeriod);
                                            
   subtype resolution is std_logic_vector(11 downto 0);
   subtype color is integer range 0 to 7;

   procedure setrgb(signal r, g, b : out std_logic_vector(2 downto 0); constant val_r, val_g, val_b : color);
   procedure setmode(constant mode : modes; signal settings : out std_logic_vector(60 downto 0));
end vga_controller_cfg;

package body vga_controller_cfg is
   procedure setrgb(signal r, g, b : out std_logic_vector(2 downto 0); constant val_r, val_g, val_b : color) is
   begin
      r <= conv_std_logic_vector(val_r, 3);
      g <= conv_std_logic_vector(val_g, 3);
      b <= conv_std_logic_vector(val_b, 3);
   end setrgb;

   procedure setmode(constant mode : modes; signal settings : out std_logic_vector(60 downto 0)) is
   begin
       case mode is
           when r640x480x60 =>
               settings(60 downto 49) <= conv_std_logic_vector(640 -1, 12); -- h_pixels    12 bit (0-4095)
               settings(48 downto 37) <= conv_std_logic_vector(480 -0, 12); -- v_lines     12 bit (0-4095)
               settings(36 downto 30) <= conv_std_logic_vector(16  -1,  7); -- h_syncstart  7 bit (0-127)
               settings(29 downto 23) <= conv_std_logic_vector(96  -1,  7); -- h_syncend    7 bit (0-127)
               settings(22 downto 16) <= conv_std_logic_vector(48  -1,  7); -- h_period     7 bit (0-127)
               settings(15 downto 10) <= conv_std_logic_vector(10  -0,  6); -- v_syncstart  6 bit (0-63)
               settings(9 downto 6)   <= conv_std_logic_vector(2   -0,  4); -- v_syncend    4 bit (0-63)
               settings(5 downto 0)   <= conv_std_logic_vector(33  -0,  6); -- v_period     6 bit (0-63)

           when r640x480x66 =>
               settings(60 downto 49) <= conv_std_logic_vector(640 -1, 12);
               settings(48 downto 37) <= conv_std_logic_vector(480 -0, 12);
               settings(36 downto 30) <= conv_std_logic_vector(61  -1,  7);
               settings(29 downto 23) <= conv_std_logic_vector(64  -1,  7);
               settings(22 downto 16) <= conv_std_logic_vector(99  -1,  7);
               settings(15 downto 10) <= conv_std_logic_vector(1   -0,  6);
               settings(9 downto 6)   <= conv_std_logic_vector(3   -0,  4);
               settings(5 downto 0)   <= conv_std_logic_vector(41  -0,  6);

           when r800x600x72 =>
               settings(60 downto 49) <= conv_std_logic_vector(800 -1, 12);
               settings(48 downto 37) <= conv_std_logic_vector(600 -0, 12);
               settings(36 downto 30) <= conv_std_logic_vector(53  -1,  7);
               settings(29 downto 23) <= conv_std_logic_vector(120 -1,  7);
               settings(22 downto 16) <= conv_std_logic_vector(67  -1,  7);
               settings(15 downto 10) <= conv_std_logic_vector(35  -0,  6);
               settings(9 downto 6)   <= conv_std_logic_vector(6   -0,  4);
               settings(5 downto 0)   <= conv_std_logic_vector(25  -0,  6);

           when others =>
               settings(60 downto 49) <= conv_std_logic_vector(0, 12);
               settings(48 downto 37) <= conv_std_logic_vector(0, 12);
               settings(36 downto 30) <= conv_std_logic_vector(0, 7);
               settings(29 downto 23) <= conv_std_logic_vector(0, 7);
               settings(22 downto 16) <= conv_std_logic_vector(0, 7);
               settings(15 downto 10) <= conv_std_logic_vector(0, 6);
               settings(9 downto 6)   <= conv_std_logic_vector(0, 4);
               settings(5 downto 0)   <= conv_std_logic_vector(0, 6);
       end case;
   end setmode;
end vga_controller_cfg;
