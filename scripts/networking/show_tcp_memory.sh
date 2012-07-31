
function show_proc()
{
  echo ----------------------------------------------
  echo $*
  $*
}

show_proc cat /proc/sys/net/ipv4/tcp_mem
show_proc cat /proc/sys/net/core/rmem_default
show_proc cat /proc/sys/net/core/rmem_max
show_proc cat /proc/sys/net/core/wmem_default
show_proc cat /proc/sys/net/core/wmem_max
show_proc cat /proc/sys/net/core/optmem_max
show_proc cat /proc/net/sockstat
show_proc cat /proc/sys/net/ipv4/tcp_max_orphans

