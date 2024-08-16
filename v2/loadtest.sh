#!/bin/bash

> client_performance_avg_response.txt
> client_performance_throughput.txt
> virtual_memory_stats.txt
> NLWP_stats.txt
> client_performance_timeout_rate.txt
> client_performance_request_rate.txt
> client_performance_cpu_utilization.txt
> client_performance_error_rate.txt
> client_performance_total_threads.txt


if [ $# != 4 ]; then
    echo "Invalid Usage: loadtest.sh <numClients> <loopNum> <sleepTimeInSeconds> <TimeoutInSeconds>"
    exit
fi

gcc simple-server.c -o server
gcc simple-client.c -o submit


./server 8085 2>server_error.txt &
server_id=$!

sleep 2


for ((j=1; j<=$1; ))
do


> virtual_memory_stats.txt
> NLWP_stats.txt

echo -e "\nload: $j" >> virtual_memory_stats.txt 
vmstat 1 >> virtual_memory_stats.txt & 
vmstat_process_id=$!

echo -e "\nload: $j" >> NLWP_stats.txt
watch -n 1 "ps -eLf | grep $server_id | wc -l  >> NLWP_stats.txt" >/dev/null &
watch_process_id=$!

pids=()
for ((i=0; i<$j; i++))
do
    ./submit 127.0.0.0:8085 test1.c $2 $3 $4 > client_performance$i.txt &
    pids[i]=$!    
done

for ((i=0; i<$j; i++))
do
    wait $((pids[i]))
done


kill $vmstat_process_id
> cpu_uti.txt
awk '{print $15}' < virtual_memory_stats.txt | sed 's/us/0/g' > cpu_uti.txt
cpu_utilization=0
count=0
for val in $(cat cpu_uti.txt)
do
    cpu_utilization=$(echo "$cpu_utilization + $val" | bc -l)
    count=$((count+1))
done
cpu_utilization=$(echo "$cpu_utilization / $count" | bc -l)
ut=$(echo "100 - $cpu_utilization" | bc -l)
echo $j $ut >> client_performance_cpu_utilization.txt
rm -r cpu_uti.txt 2>/dev/null


kill $watch_process_id
> nlwp.txt
sed 's/load: //g' < NLWP_stats.txt > nlwp.txt
nlps=0
count=0
for val in $(cat nlwp.txt)
do
    nlps=$(echo "$nlps + $val - 1" | bc -l)
    count=$((count+1))
done
threads=$(echo "$nlps / $count" | bc -l)
echo $j $threads >> client_performance_total_threads.txt
rm -r nlwp.txt 2>/dev/null 


thr=0
total_res_time=0
total_timeouts=0
total_errors=0
for ((i=0; i<$j; i++))
do 
    if [ -s client_performance$i.txt ]; then
        x=$(cat client_performance$i.txt | cut -d' ' -f 2)
        y=$(cat client_performance$i.txt | cut -d' ' -f 1)
        y=$(echo "$y * $2" | bc -l)
        z=$(cat client_performance$i.txt | cut -d' ' -f 3)
        a=$(cat client_performance$i.txt | cut -d' ' -f 4)
        e=$(cat client_performance$i.txt | cut -d' ' -f 5)
        total_errors=$(echo "$total_errors + $e" | bc -l)
        total_timeouts=$(echo "$total_timeouts + $a" | bc -l)
        total_res_time=$(echo "$total_res_time + $y" | bc -l)
        thr=$(echo "$thr + $x / $z" | bc -l)
    fi
done

if [ $total_res_time != 0 ]; then
    
    echo $j $thr >> client_performance_throughput.txt
    timeout_rate=$(echo "$total_timeouts / $total_res_time" | bc -l)
    echo $j $timeout_rate >> client_performance_timeout_rate.txt
    error_rate=$(echo "$total_errors / $total_res_time" | bc -l)
    echo $j $error_rate >> client_performance_error_rate.txt
    request_rate=$(echo "$thr + $timeout_rate + $error_rate" | bc -l)
    echo $j $request_rate >> client_performance_request_rate.txt
else
    echo $j 0.0 >> client_performance_throughput.txt
    timeout_rate=$(echo "$total_timeouts / $j" | bc -l)
    echo $j $timeout_rate >> client_performance_timeout_rate.txt
    error_rate=$(echo "$total_errors / $j" | bc -l)
    echo $j $error_rate >> client_performance_error_rate.txt
    request_rate=$(echo "$timeout_rate + $error_rate" | bc -l)
    echo $j $request_rate >> client_performance_request_rate.txt
fi

avg_res=0
loop_time=0
for ((i=0; i<$j; i++))
do 
    if [ -s client_performance$i.txt ]; then
        x=$(cat client_performance$i.txt | cut -d' ' -f 1)
        y=$(cat client_performance$i.txt | cut -d' ' -f 3)
        loop_time=$(echo "$loop_time + $y" | bc -l)
        avg_res=$(echo "$avg_res + $x" | bc -l)
    fi
done

if [ $avg_res != 0 ]; then 
    avg_res=$(echo "$avg_res / $j" | bc -l)
    echo $j $avg_res >> client_performance_avg_response.txt
else
    loop_time=$(echo "$loop_time / $j" | bc -l)
    echo $j $loop_time >> client_performance_avg_response.txt 
fi

for ((i=0; i<$j; i++))
do
    rm -r client_performance$i.txt 2>/dev/null    
done

j=$((j+1))
done

kill $server_id
python3 plot.py client_performance_throughput.txt 0
python3 plot.py client_performance_avg_response.txt 1
python3 plot.py client_performance_cpu_utilization.txt 2

