# Assignment 1, due October 19th 2020
Handl Philip, Buchauer Manuel, De Sclavis Davide


## Exercise 1

### Tasks
#### Study how to submit jobs in SGE, how to check their state and how to cancel them.

- <code>qsub name_of_script</code> is used to submit jobs where name_of_script represents the path to  a simple shell script containing the commands to be run on the remote cluster nodes.
- <code>qstat</code> to get information about running or waiting jobs 
- <code>qdel job-id</code> to delete a job (terminates application, frees up resources)


#### Prepare a submission script that starts an arbitrary executable, e.g. `/bin/hostname`
<pre><code>#!/bin/bash
# Execute job in the queue "std.q" unless you have special requirements. 
#$ -q std.q
# The batch system should use the current directory as working directory. 
#$ -cwd
# Name your job. Unless you use the -o and -e options, output will 
# go to a unique file name.o<job_id> for each job.
#$ -N my_test_job
# Redirect output stream to this file.
#$ -o output.dat
# Join the error stream to the output stream.
#$ -j yes
# Specify parallel environment (list available ones with qconf –spl) 
#$ -pe openmpi-2perhost 8

module load openmpi/3.1.1
mpiexec –n 8 /bin/hostname
</code></pre>

#### In your opionion, what are the 5 most important parameters available when submitting a job and why? What are possible settings of these parameters, and what effect do they have?
1. <code>-q queuename</code> to submit job to specefic queue
2. <code>-pe</code> to specify a parallel environment and the number of processes/threads (= SGE slots) on which your parallel (MPI/OpenMP) application should run
3. <code>-N</code> to name a job.. default is the filename of the script.
4. <code>-w</code> to check whether the syntax of the job is okay
5. <code>-cwd</code> to execute job in current working directory (If you omit this option, your job will execute in $HOME, which is usually a bad idea. Input/output file names are relative to this directory)
#### How do you run your program in parallel? What environment setup is required?
With the line  <code>#$ -pe openmpi-2perhost 8 </code>in the job script a parallel environment is set up.  
This line means: Set up a parallel environment with 2 CPU/cores per node and in total 8 cores. This means in total 4 nodes with 2 cores each are needed to get to the 8 cores in total.

To actually start the program in parallel, the  `mpiexec -n 8 /path/to/application` command is needed. 
The `-n` flag is set to 8, which executes the command/program 8 times (i.e. starts 8 processes).
