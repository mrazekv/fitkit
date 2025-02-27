# xilinxise.tcl : XILINX ISE script
# Copyright (C) 2010 Brno University of Technology,
#                    Faculty of Information Technology
# Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
# 
# LICENSE TERMS
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. All advertising materials mentioning features or use of this software
#    or firmware must display the following acknowledgement: 
#
#      This product includes software developed by the University of
#      Technology, Faculty of Information Technology, Brno and its 
#      contributors. 
#
# 4. Neither the name of the Company nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
# 
# This software or firmware is provided ``as is'', and any express or implied
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose are disclaimed.
# In no event shall the company or contributors be liable for any
# direct, indirect, incidental, special, exemplary, or consequential
# damages (including, but not limited to, procurement of substitute
# goods or services; loss of use, data, or profits; or business
# interruption) however caused and on any theory of liability, whether
# in contract, strict liability, or tort (including negligence or
# otherwise) arising in any way out of the use of this software, even
# if advised of the possibility of such damage.
# 
# $Id$
# 

proc project_new {NAME} {
   puts "Creating new project"

   set isever [get_ise_version]

        if {[lindex $isever 0] >= 11} {
                set EXT "xise"
        } else {
                set EXT "ise"
        }
   file delete "$NAME.$EXT"
   project new "$NAME.$EXT"
}

proc project_close {} {
   puts "Closing project"

   project close
}

proc project_set_props {} {
   puts "Setting project properties"

   project set family "Spartan3"
   project set device "xc3s50"
   project set package "pq208"
   project set speed "-4"
   catch {
     project set top_level_module_type "HDL"
   }
   catch {
     project set synthesis_tool "XST (VHDL/Verilog)"
   }
   catch {
     project set "Preferred Language" "VHDL"
   }
   catch {
     project set "Auto Implementation Top" "true"
   }
}

proc project_set_top {NAME} {
   puts "Setting top-level entity"
   
   #Try the top-level instance /NAME
   set insts [search "/$NAME" -t instance]
   if {[collection sizeof $insts] == 0} {
      #Failed - try to find instance with the same name
      set insts [search "*/$NAME*" -t instance]
   }
   
   if {[collection sizeof $insts] == 0} {
      #Failed - try the pair architecture + entity_name 
      if {[catch {project set top "behavioral" "$NAME"}]} { 
         puts "Set top-level failed"
      } else {
         puts "  entity $NAME of architecture behavioral found"
      }
   } else {
      set inst [object name [collection index $insts 0]]
      puts "  instance $inst found"
      project set top "$inst"
   }
}

proc project_set_isimscript {FILENAME} {
  set ok 0
  foreach {P} {"Simulate Behavioral Model" "Simulate Post-Map Model" "Simulate Post-Place & Route Model" "Simulate Post-Translate Model"} {
     catch {
       project set "Custom Simulation Command File" "$FILENAME" -process "$P"
       project set "Use Custom Simulation Command File" true -process "$P"
                 incr ok
     }
  }

  if {$ok != 4} {
     catch {
       project set "Custom Simulation Command File" "$FILENAME" -process all
       project set "Use Custom Simulation Command File" true -process all
     }
  }

}

proc project_set_ngcpath {NGCPATH} {
  catch {
     project set "Macro Search Path" "$NGCPATH" -process "Translate"
  }
}

proc project_get_ext {} {

   set isever [get_ise_version]

        if {[lindex $isever 0] >= 11} {
                puts "xise"
        } else {
                puts "ise"
        }

}

proc get_ise_version {} {

   set maj 0
   set min 0
   catch {
     set par_msg  [open "|par" r]
     while {[gets $par_msg cur_line] >= 0} {
       if {[regexp -nocase {^Release (.*) - .*} $cur_line match ver]} {
         set ise_ver [regexp -all -inline {\d+} $ver]
         set maj [lindex $ise_ver 0]
         set min [lindex $ise_ver 1]
       } 
     }
   }
   return [list $maj $min]
}
