# xilinxisim.tcl : XILINX ISIM script - simulation environment
# Copyright (C) 2011 Brno University of Technology,
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
if {[info exists TESTBENCH_SCRIPT] == 0} {
   set TESTBENCH_SCRIPT "../../fpga/sim/isim.tcl"
}

if {[info exists TESTBENCH_ENTITY] == 0} {
   set TESTBENCH_ENTITY "testbench"
}

if {[info exists ISIM_PRECISION] == 0} {
   set ISIM_PRECISION "1ps"
}

if {[info exists SIM_ENVIRONMENT] == 0} {
   set SIM_ENVIRONMENT "isim"
}

# remove spaces
set ISIM_PRECISION [string map {" " ""} "$ISIM_PRECISION"]

if {$SIM_ENVIRONMENT == "isim"} {
   #================================================
   #Skript spusten pomoci gmake isim
   #================================================
       
   #puts "TB FILES: $TESTBENCH_ENTITY"
   #puts "TB PRECISION: $ISIM_PRECISION"
   #puts "ISIM SCRIPT: $ISIM_SCRIPT"

   if {[catch {source "$TESTBENCH_SCRIPT"}]} { 
       puts "Failed to load ISIM testbench script"
   } else {
       puts "ISIM testbench script loaded"
   } 
	
   #==================================================
   # create ISIM script
   #==================================================
   set fo [open "$ISIM_SCRIPT" "w"] 
   puts $fo "onerror {resume}
   puts \"==========================================================\nInitialize simulation\n==========================================================\ntype isim_restart to restart simulation\"
   set FIRSTRUN 1
   
   proc add_divider {TEXT} {
    divider add \"\$TEXT\"
   }
   
   #Add wave flags, label and signal name
   proc add_wave_label { FLAGS LABEL NAME } {
      set cmd \"wave add \$FLAGS\"
      if { \$LABEL != \"\" } {
         append cmd \" -name \\\"\$LABEL\\\"\"
      }
      append cmd \" \$NAME\"
      if {\[catch {eval \$cmd } stderr\] != 0} {
         puts \"  WARNING : Signal \$NAME doesn't exist\"
      }
   }
   
   #Add wave flags and signal name
   proc add_wave {FLAGS NAME} {
    add_wave_label \"\$FLAGS\" \"\" \"\$NAME\"
   }
         	
   proc isim_restart {} {
      global FIRSTRUN
      #set TESTBENCH_SCRIPT \"[pwd]/$TESTBENCH_SCRIPT\"
      if {\$FIRSTRUN == 0} { wcfg new }
      set FIRSTRUN 0
      #puts \"$TESTBENCH_SCRIPT\" 
      #puts \[pwd\]
      if {\[catch {source \"$TESTBENCH_SCRIPT\"}\]} { 
         puts \"isim source failed\"
         flush stdout
      } else {
         puts \"isim found, tb entity: $TESTBENCH_ENTITY\"
         flush stdout
         if {\[catch {isim_script} stderr\] != 0} {
            puts \"ERROR: \$stderr\"
         }
      } 
   }
   
   isim_restart	
   "
   close $fo
   
   #==================================================
   #compile design
   #==================================================
   set fh [open "|fuse -intstyle ise --timeprecision_vhdl $ISIM_PRECISION -incremental -o \"$ISIM_SIMULATOR\" -prj \"$ISIM_PRJ\" $TESTBENCH_ENTITY" r]
   while {[gets $fh line] >= 0} { puts $line }
   
   set exit_status 0
   if {[catch {close $fh} err] != 0} {
       global errorCode
       if {"CHILDSTATUS" == [lindex $errorCode 0]} {
           set exit_status [lindex $errorCode 2]
       }
       puts "\n$err"
   }
   #puts "exit status is $exit_status"
   
   #==================================================
   #execute simulator
   #==================================================
   if {$exit_status == 0} {
   
      if {[catch {exec "./$ISIM_SIMULATOR" -intstyle ise -gui -tclbatch "$ISIM_SCRIPT"} stderr] != 0} {
         puts "\n$stderr"
      }
   
   }

} 	
if {$SIM_ENVIRONMENT == "ise"} {
   #================================================
   #Skript spusten pomoci ISE
   #================================================

   proc add_divider {TEXT} {
    divider add "$TEXT"
   }
   
   #Add wave flags, label and signal name
   proc add_wave_label { FLAGS LABEL NAME } {
      set cmd "wave add $FLAGS"
      if { $LABEL != "" } {
         append cmd " -name \"$LABEL\""
      }
      append cmd " $NAME"
      if {[catch {eval $cmd } stderr] != 0} {
         puts "  WARNING : Signal $NAME doesn't exist"
      }
   }
   
   #Add wave flags and signal name
   proc add_wave {FLAGS NAME} {
    add_wave_label "$FLAGS" "" "$NAME"
   }

   if {[catch {source "$TESTBENCH_SCRIPT"}]} { 
      puts "Failed to load ISIM testbench script"
   } else {
      puts "ISIM testbench script loaded"

      if {[catch {isim_script} stderr] != 0} {
         puts "ERROR: $stderr"
      }

   } 	
}

