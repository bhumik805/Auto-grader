#!/bin/bash

# Clear existing result files
> cliavgres.txt
> throughput.txt
> looptimes.txt
> client_performance_cpu_utilization.txt

# Check if the correct number of command-line arguments are provided
if [ $# -ne 5 ]; then
    echo "Usage: ./loadtest.sh <no. of clients> <no. of requests> <thinktime/sleeptime> <ip> <server port>"
    exit 1
fi

# Start the server in the background and capture its process ID
./server $5 2>server_error.txt &
server_id=$!
sleep 2

# Loop through the specified number of clients
for ((i=1;i<=$1; i++ )); do

    # Clear virtual memory statistics file
    > virtual_memory_stats.txt
    echo -e "\nload: $i" >> virtual_memory_stats.txt

    # Run vmstat in the background to capture virtual memory statistics
    vmstat 1 >> virtual_memory_stats.txt & 
    vmstat_process_id=$!

    cliavgres=0
    thr=0
    temp=0
    totres=0
    pids=()

    # Loop through number of clients i
    for ((j=1; j<=$i; j++)); do
        # Run the client in the background and capture its process ID
        ./client "$4:$5" hello.c "$2" "$3" 1>out$j.txt &
        pids[j]=$!  
    done 

    # Wait for all client processes to complete
    for ((j=1; j<=$i; j++)); do
        wait "${pids[j]}"
    done

    echo "$i clients completed"

    # Process the results for each client
    for ((j=1;j<=$i;j++)); do
        op=$(cut -d " " -f 1,2,3 out$j.txt)
        res=$(echo $op | cut -d " " -f 1 | bc -l)
        cliavgres=$(echo "$cliavgres + $res" | bc -l)
        sucres=$(echo $op | cut -d " " -f 2 | bc -l)
        temp=$(echo $op | cut -d " " -f 3 | bc -l)
        thr=$(echo "$thr + $sucres / $temp" | bc -l)
        totres=$(echo "$totres + $temp" | bc -l)
    done

    # Stop the vmstat process
    kill $vmstat_process_id

    # Extract CPU utilization information from virtual_memory_stats.txt
    > cpu_uti.txt
    awk '{print $15}' < virtual_memory_stats.txt | sed 's/us/0/g' > cpu_uti.txt
    cpu_utilization=0
    count=0

    # Calculate average CPU utilization
    for val in $(cat cpu_uti.txt)
    do
        cpu_utilization=$(echo "$cpu_utilization + $val" | bc -l)
        count=$((count+1))
    done
    cpu_utilization=$(echo "$cpu_utilization / $count" | bc -l)
    ut=$(echo "100 - $cpu_utilization" | bc -l)
    
    # Append CPU utilization information to client_performance_cpu_utilization.txt
    echo $j $ut >> client_performance_cpu_utilization.txt
    rm -r cpu_uti.txt 2>/dev/null

    # Calculate and append results to respective result files
    cliavgres=$(echo "$cliavgres / $i" | bc -l)
    thr=$(echo "$thr" | bc -l)
    echo $i $totres >> looptimes.txt
    echo $i $cliavgres >> cliavgres.txt
    echo $i $thr >> throughput.txt
    
done

# Stop the server
# kill $server_id

# Run Python scripts to generate plots
python3 plot.py cliavgres.txt 1
python3 plot.py throughput.txt 0
python3 plot.py client_performance_cpu_utilization.txt 2
