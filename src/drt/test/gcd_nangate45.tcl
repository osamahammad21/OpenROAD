read_lef Nangate45/Nangate45_tech.lef
read_lef Nangate45/Nangate45_stdcell.lef
read_def gcd_nangate45_preroute.def
set_thread_count [expr [exec getconf _NPROCESSORS_ONLN] / 4]
read_guides  gcd_nangate45.route_guide
detailed_route -output_drc results/gcd_nangate45.output.drc.rpt \
               -output_maze results/gcd_nangate45.output.maze.log \
               -verbose 1 
set def_file results/gcd_nangate45.defok
write_def $def_file
