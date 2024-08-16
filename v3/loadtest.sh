#!/bin/bash
# Initializing output files
> client_performance_avg_response.txt
> client_performance_throughput.txt
> virtual_memory_stats.txt
> client_performance_timeout_rate.txt
> client_performance_error_rate.txt

# Checking command line arguments
if [ $# != 4 ]; then
    echo "Invalid Usage: loadtest.sh <numClients> <loopNum> <sleepTimeInSeconds> <TimeoutInSeconds>"
    exit
fi

# Start the server in the background
./server 8085 12 2>server_error.txt &
server_id=$!

# Wait for server to start
sleep 1

# Loop for different numbers of clients
for ((j=1; j<=$1; ))
do
    # Initializing output files for each iteration
    > virtual_memory_stats.txt

    echo -e "\nload: $j" >> virtual_memory_stats.txt
    vmstat 1 >> virtual_memory_stats.txt &
    vmstat_process_id=$!

    # Array to store process IDs of client executions
    pids=()

    # Launching clients
    for ((i=0; i<$j; i++))
    do
        ./submit 127.0.0.0:8085 test1.c $2 $3 $4 > client_performance$i.txt &
        pids[i]=$!
    done

    # Waiting for all client processes to finish
    for ((i=0; i<$j; i++))
    do
        wait $((pids[i]))
    done

    # Killing the vmstat process
    kill $vmstat_process_id

    # Extracting CPU utilization data
    > cpu_uti.txt
    awk '{print $15}' < virtual_memory_stats.txt | sed 's/us/0/g' > cpu_uti.txt

    cpu_utilization=0
    count=0

    # Calculating average CPU utilization
    for val in $(cat cpu_uti.txt)
    do
        cpu_utilization=$(echo "$cpu_utilization + $val" | bc -l)
        count=$((count+1))
    done

    if [ $count == 0 ]; then
        ut=0
    else    
        cpu_utilization=$(echo "$cpu_utilization / $count" | bc -l)
        ut=$(echo "100 - $cpu_utilization" | bc -l)
    fi

    # Appending CPU utilization data to the output file
    echo $j $ut >> client_performance_cpu_utilization.txt
    rm -r cpu_uti.txt 2>/dev/null

    # Initializing variables for statistics
    succ_req=0
    total_res_time=0
    total_timeouts=0
    total_errors=0
    throughput=0

    # Processing output files from clients
    for ((i=0; i<$j; i++))
    do 
        if [ -s client_performance$i.txt ]; then
            x=$(cat client_performance$i.txt | cut -d' ' -f 2)
            y=$(cat client_performance$i.txt | cut -d' ' -f 1)
            y=$(echo "$y * $2" | bc -l)
            a=$(cat client_performance$i.txt | cut -d' ' -f 4)
            e=$(cat client_performance$i.txt | cut -d' ' -f 5)
            total_errors=$(echo "$total_errors + $e" | bc -l)
            total_timeouts=$(echo "$total_timeouts + $a" | bc -l)
            total_res_time=$(echo "$total_res_time + $y" | bc -l)
            succ_req=$(echo "$succ_req + $x" | bc -l)
            v=$(cat client_performance$i.txt | cut -d' ' -f 3)
            z=$(echo "$v" | bc -l)
            throughput=$(echo "$throughput + $x/$v" | bc -l)
        fi
    done

    # Calculating throughput
    if [ $total_res_time != 0 ]; then
        thr=$(echo "$succ_req / $total_res_time" | bc -l)
        echo $j $throughput >> client_performance_throughput.txt
    else
        echo $j 0.0 >> client_performance_throughput.txt
        echo $j $timeout_rate >> client_performance_timeout_rate.txt
    fi

    # Calculating average response time or loop time
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

    # Appending average response time or loop time to the output file
    if [ $avg_res != 0 ]; then 
        avg_res=$(echo "$avg_res / $j" | bc -l)
        echo $j $avg_res >> client_performance_avg_response.txt
    else
        loop_time=$(echo "$loop_time / $j" | bc -l)
        echo $j $loop_time >> client_performance_avg_response.txt 
    fi

    # Cleaning up client performance files
    for ((i=0; i<$j; i++))
    do
        rm -r client_performance$i.txt 2>/dev/null    
    done

    # Incrementing the number of clients for the next iteration
    j=$((j+1))
done

# Removing temporary queueSize.txt
rm -r queueSize.txt 2>/dev/null

# Killing the server
kill $server_id

# Plotting the results
python3 plot.py client_performance_throughput.txt 0
python3 plot.py client_performance_avg_response.txt 1
python3 plot.py client_performance_cpu_utilization.txt 2

