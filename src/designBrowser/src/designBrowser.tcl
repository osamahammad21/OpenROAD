###############################################################################
##
## BSD 3-Clause License
##
## Copyright (c) 2019, University of California, San Diego.
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and#or other materials provided with the distribution.
##
## * Neither the name of the copyright holder nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
###############################################################################

sta::define_cmd_args "report_logical_hierarchy" {[-module module] \
                                      [-level level] \
                                      [-rpt_file rpt_file] }
proc report_logical_hierarchy { args } {
    sta::parse_key_args "report_logical_hierarchy" args keys {-module -level -rpt_file} flags { }
    if { [info exists keys(-module)] && [info exists keys(-rpt_file)] } {
        set module $keys(-module)
        set rpt_file $keys(-rpt_file)
        if { [info exists keys(-level)] } {
            set level $keys(-level)
            ord::design_browser_cmd $module $rpt_file $level
        } else {
            ord::design_browser_cmd $module $rpt_file 100000
        }
    } else {
        ord::error "report_logical_hierarchy -module module_name -rpt_file hier.rpt -level level"
    }
}
 
 
sta::define_cmd_args "report_area" {[-module module] \
                                    [-details] \
                                    [-rpt_file rpt_file] }
proc report_area { args } {
    sta::parse_key_args "report_area" args keys {-module -rpt_file -key} flags { -details }
    if { [info exists keys(-module)] && [info exists keys(-rpt_file)] } {
        set module $keys(-module)
        set rpt_file $keys(-rpt_file)
        if { [info exists flags(-details)] && [info exists keys(-key)] } {
            set key $keys(-key)
            ord::report_logic_area_cmd $module $rpt_file true $key
        } else {
            ord::report_logic_area_cmd $module $rpt_file false term_count
        }
    } else {
        ord::error "report_area -module module -rpt_file rpt_file -details -key term_count/cell_count"
    }
}


sta::define_cmd_args "report_logical_net" {[-module module] \
                                      [-rpt_file rpt_file] }
proc report_logical_net { args } {
    sta::parse_key_args "report_logical_net" args keys {-module -rpt_file} flags { }
    if { [info exists keys(-module)] && [info exists keys(-rpt_file)] } {
        set module $keys(-module)
        set rpt_file $keys(-rpt_file)
        ord::report_logic_net_cmd $module $rpt_file
    } else {
        ord::error "report_logical_net -module module -rpt_file rpt_file"
    }
}


sta::define_cmd_args "report_logical_connection" {[-module module] \
                                      [-rpt_file rpt_file] }
proc report_logical_connection { args } {
    sta::parse_key_args "report_logical_connection" args keys {-module -rpt_file} flags { }
    if { [info exists keys(-module)] && [info exists keys(-rpt_file)] } {
        set module $keys(-module)
        set rpt_file $keys(-rpt_file)
        ord::report_logic_connection_cmd $module $rpt_file
    } else {
        ord::error "report_logical_connection -module module -rpt_file rpt_file"
    }
}

sta::define_cmd_args "report_macro" {[-module module] \
                                      [-rpt_file rpt_file] }
proc report_macro { args } {
    sta::parse_key_args "report_macro" args keys {-module -rpt_file} flags { }
    if { [info exists keys(-module)] && [info exists keys(-rpt_file)] } {
        set module $keys(-module)
        set rpt_file $keys(-rpt_file)
        ord::report_macro_cmd  $module $rpt_file
    } else {
        ord::error "report_macro -module module -rpt_dir rpt_file"
    }
}

sta::define_cmd_args "report_design_file" {[-logical_hierarchy file_name]}
proc report_design_file { args } {
    sta::parse_key_args "report_design_file" args keys {-logical_hierarchy } flags { }
    if { [info exists keys(-logical_hierarchy)] } {
        set file_name $keys(-logical_hierarchy)
        ord::report_design_file $file_name
    } else {
        ord::error "report_design_file -logical_hierarchy file_name"
    }
}

