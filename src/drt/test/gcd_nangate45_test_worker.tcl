set OR $argv0
set server1 [$OR server1.tcl > results/server1.log &]
set server2 [$OR server2.tcl > results/server2.log &]
set balancer [$OR balancer.tcl > results/balancer.log &]
after 2000
set_debug_level DRT autotuner 1
detailed_route_debug -dr
detailed_route_run_worker -dump_dir results/worker138600x_79800y/ \
                          -distributed \
                          -remote_host 127.0.0.1 \
                          -remote_port 1234 \
                          -local_host 127.0.0.1 \
                          -local_port 9134 \
                          -cloud_size 2 \
                          -shared_volume results/shared_volume/
exec kill $server1
exec kill $server2
exec kill $balancer