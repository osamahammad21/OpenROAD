set_thread_count [expr [exec getconf _NPROCESSORS_ONLN] / 4]
set_debug_level DRT autotuner 1
run_worker -host 127.0.0.1 -port 1112
