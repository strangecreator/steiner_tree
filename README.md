# Steiner Tree 2-Approximation

## Algorithms

Exact:
* Dreyfus-Wagner algorithm
* Optimization algorithm using `pyscipopt`

Approximate:
* 2-approximation algorithm from [Algorithms Lab, Approximations algorithms for the Steiner Tree Problem and the Traveling Salesperson Problem (TSP), 2023](https://www.youtube.com/watch?v=ZQbq3MAiy6Y)

## Analysis

All algorithms above were tested, and their time complexity, along with approximation coefficient, has been analyzed in `src/analysis.ipynb`. Measurement data was saved into `data` folder.

## Notes

Executables were generated using this command:
```sh
g++ -g -O2 -Wall -Wextra -Werror -o ../bin/<name-of-algo> <name-of-algo>.cpp
```