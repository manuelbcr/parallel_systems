# Assignment 1, due October 19th 2020
Handl Philip, Buchauer Manuel, De Sclavis Davide


## Exercise 1

### Tasks
#### Study how to submit jobs in SGE, how to check their state and how to cancel them.

- <code>qsub name_of_script</code> is used to submit jobs where name_of_script represents the path to  a simple shell script containing the commands to be run on the remote cluster nodes.
- <code>qstat</code> to get information about running or waiting jobs 
- <code>qdel job-id</code> to delete a job (terminates application, frees up resources)




#### Prepare a submission script that starts an arbitrary executable, e.g. `/bin/hostname`
#### In your opionion, what are the 5 most important parameters available when submitting a job and why? What are possible settings of these parameters, and what effect do they have?
#### How do you run your program in parallel? What environment setup is required?
