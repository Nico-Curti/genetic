| **Authors**  | **Project** |
|:------------:|:-----------:|
|   N. Curti   |   Genetic   |

# Genetic algorithm

Examples of genetic algorithm applications in parallel and distributed enironment.

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Authors](#authors)
4. [License](#license)

## Prerequisites

This project collects examples of genetic algorithm applications to very simple problems.
The codes are written in c++ language with the support of **c++14 standard**.
Before install it, please upgrade your c++ compiler version to a compatible one (ex. version >= 5 for g++ compiler).

For the multi-threading version of the algorithm **OpenMP** is required so please check if your compiler support it.

A more sophisticated version of the algorithm is written with **MPI**. Before install this project, please verify if **boost mpi** libraries are already installed.

For the MPI compiler must be set the variable OMPI_CXX.

## Installation

To build the executables, clone the repo and the type

```
make omp
```

for the multi-threading version, and

```
make mpi
```

for the message-passing version.

If in the Makefile the variable OMP is set to **true** you can enable multi-threading also for the mpi code (**hybrid version**).


## Authors

* **Nico Curti** [git](https://github.com/Nico-Curti), [unibo](https://www.unibo.it/sitoweb/nico.curti2)

## License

This project is released under GPL license. [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://github.com/Nico-Curti/genetic/blob/master/LICENSE)
