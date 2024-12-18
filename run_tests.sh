#!/bin/bash

# Number of runs to compute the average
NUM_RUNS=3

# Problem size
SIZE=500

# Processes to test
PROCESSES=(2 4 8 16)

# Output file
OUTPUT_FILE="results.log"

# Clear output file
echo "Logging results to $OUTPUT_FILE"
echo "Experiment Results for n=$SIZE" > "$OUTPUT_FILE"
echo "==============================" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Function to run and average times
run_and_average() {
    local cmd="$1"
    local total=0.0

    echo "Running: $cmd"
    for (( i=1; i<=$NUM_RUNS; i++ )); do
        # Capture the real execution time from time command
        result=$( { /usr/bin/time -f "%e" $cmd > /dev/null; } 2>&1 )
        total=$(echo "$total + $result" | bc -l)
        echo "Run $i: $result s"
    done

    # Compute average
    avg=$(echo "$total / $NUM_RUNS" | bc -l)
    echo "$avg"
}

# Run tests
for p in "${PROCESSES[@]}"; do
    echo "Running with $p processes..." | tee -a "$OUTPUT_FILE"
    avg_time=$(run_and_average "mpirun -n $p ./1D_parallel_julia $SIZE")
    echo "Average time for $p processes: $avg_time s" | tee -a "$OUTPUT_FILE"
    echo "" | tee -a "$OUTPUT_FILE"
done

echo "All tests completed. Results logged in $OUTPUT_FILE."
