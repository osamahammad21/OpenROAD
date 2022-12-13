source "helpers.tcl"

set OR $argv0
set server1 [exec $OR server1.tcl > results/server1.log &]
set server2 [exec $OR server2.tcl > results/server2.log &]
set balancer [exec $OR balancer.tcl > results/balancer.log &]

read_lef Nangate45/Nangate45_tech.lef
read_lef Nangate45/Nangate45_stdcell.lef
read_def gcd_nangate45_preroute.def
exec sleep 5
set_thread_count [expr [exec getconf _NPROCESSORS_ONLN] / 4]
frankenstein_test -remote_host 127.0.0.1 \
                  -remote_port 1234 \
                  -cloud_size 2 \
                  -shared_volume results
exec kill $server1
exec kill $server2
exec kill $balancer