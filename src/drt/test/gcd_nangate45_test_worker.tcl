set OR $argv0
set server1 [$OR server1.tcl > results/server1.log &]
set server2 [$OR server2.tcl > results/server2.log &]
set server3 [$OR server3.tcl > results/server3.log &]
set balancer [$OR balancer.tcl > results/balancer.log &]
after 5000
set_debug_level DRT autotuner 1
detailed_route_debug -dr
detailed_route_run_worker -dump_dir results/worker0x_0y/ \
                          -distributed \
                          -remote_host 127.0.0.1 \
                          -remote_port 1234 \
                          -cloud_size 3 \
                          -shared_volume results/shared_volume/
