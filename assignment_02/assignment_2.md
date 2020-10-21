# Assignment 2, due November 9th 2020
Handl Philip, Buchauer Manuel, De Sclavis Davide


## Exercise 1

This exercise consists in writing a parallel application to speed up the computation of π.

### Description

There are many ways of approximating π, one being a well-known Monte Carlo method: The ratio of the areas of a square and its incircle is π/4. Since the exact area of a circle cannot be computed (we don't know the value of π yet), one can instead sample random points, check their distance from the center and compute the ratio of points inside the circle to all sampled points.

<img src="https://upload.wikimedia.org/wikipedia/commons/2/20/MonteCarloIntegrationCircle.svg" width="40%">

### Tasks

#### Write a sequential application `pi_seq` in C or C++ that computes π for a given number of samples (command line argument). Test your application for various, large sample sizes to verify the correctness of your implementation.
We know that area of the square is 1 and the area of the circle is $\frac{\pi}{4}$
Now for a very large number of randomly created generated points we have:
$$\frac{area of the circle}{area of the square} = \frac{number of points generated inside the circle}{total number of points generated or number of points generated inside the square }$$

and therefore we get: 

$$\Pi = 4* \frac{number of points generated inside the circle}{number of points generated inside the square}$$

#### Consider a parallelization strategy using MPI. Which communication pattern(s) would you choose and why?

#### Implement your chosen parallelization strategy as a second application `pi_mpi`. Run it with varying numbers of ranks and sample sizes and verify its correctness by comparing the output to `pi_seq`.

#### Discuss the effects and implications of your parallelization.
