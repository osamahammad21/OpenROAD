read_lef Nangate45/Nangate45_tech.lef
read_lef Nangate45/Nangate45_stdcell.lef
read_def gcd_nangate45_preroute.def
read_guides gcd_nangate45.route_guide
set_thread_count [exec getconf _NPROCESSORS_ONLN]

detailed_route_debug -dump_dr -dump_dir results -worker " 109201 50401 " -iter 1

detailed_route -verbose 1