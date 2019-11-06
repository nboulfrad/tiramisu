"""
Execute the programs and report execution times. For this script to work,
the wrappers must already be compiled.

This script accepts three parameters : start end job_id
It processes the programs in the range progs_list[start, end)
job_id is used to specify which job generated which list of execution times

The execution times are stored in a list where each entry has the following format :

(function_id, schedule_id, list of execution times)
"""

import sys, pickle, subprocess
from pathlib import Path

start = int(sys.argv[1])
end = int(sys.argv[2])
job_id = int(sys.argv[3])

# Path to the wrappers (the ones generated by the wrappers job).
data_path = Path("wrappers")

# Path to the list of programs
progs_list_path = Path("progs_list.pickle")

# Path to where to store the execution times
dst_path = "results/parts/final_exec_times_%s_%s_%s_{}_{}.pickle" % (job_id, start, end) # This replaces the %s

# Number of times to execute a single program
nb_tests = 15

# The progress is printed at regular intervals
print_every = 1000

# The execution times are saved at regular intervals, in case the script crashes
save_every = 1000

with open(progs_list_path, "rb") as f:
    progs_list = pickle.load(f)
    
progs_list = progs_list[start:end]
final_exec_times = []

# Start execution
start = 0

for i, prog in enumerate(progs_list):
    func_id, sched_id = prog
    
    # Call run with capture_output=True to get the execution times
    # Execution times are printed to stdout by the wrapper, and capture_output=True uses a pipe for communication
    # With this we can avoid using a disk file for communication
    proc = subprocess.run([str(data_path / func_id / sched_id / sched_id), str(nb_tests + 1)], capture_output=True)
    times_list = proc.stdout.decode("utf-8").split(" ")
    times_list = [float(s) for s in times_list[1:-1]]

    final_exec_times.append((func_id, sched_id, times_list))
    
    # Print progress to stdout
    # stdout is redirected to the log file by the command sbatch
    if (i + 1)%print_every == 0:
        print("%s/%s" % (i + 1, len(progs_list)), flush=True)
       
    # Save the results to disk
    if (i + 1)%save_every == 0:
        with open(dst_path.format(start, i + 1), "wb") as f:
            pickle.dump(final_exec_times, f)
            
        final_exec_times = []
        start = i + 1

# Save remaining results
if final_exec_times != []:
    with open(dst_path.format(start, i + 1), "wb") as f:
        pickle.dump(final_exec_times, f)