# Routing Topology and Time-Division Multiplexing Co-optimization for Multi-FPGA systems

This repository implements our DAC'20 paper: Tung-Wei Lin, Wei-Chen Tai, Yu-Cheng Lin, and Iris Hui-Ru Jiang. "Routing topology and time-division multiplexing co-optimization for multi-FPGA systems." https://dl.acm.org/doi/10.5555/3437539.3437717

## Precompiled binary 

Please find the precompiled binary in sbin/

## Build

These instructions will help you compile and run our program successfully on your local machine if our pre-compiled static binary doesn't work on your platform.

### Dependencies

- **[paal](https://paal.mimuw.edu.pl/ "paal homepage")**  
    Download and place under `src/`
    ```bash
    cd src/
    git clone https://github.com/paal/paal.git
    ```
    If the official repository is unavailable, use the mirror:
    ```bash
    cd src/
    git clone https://github.com/WireCell/paal.git
    ```

- **[oneTBB](https://github.com/oneapi-src/oneTBB "oneTBB git repo")**  
    Modern TBB uses CMake (not Make). Build and install:
    ```bash
    cd src/
    git clone git@github.com:uxlfoundation/oneTBB.git tbb
    cd tbb
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j$(nproc)
    ```

    Set environment variables (note: no spaces around `=`):
    ```bash
    export TBBROOT="/path/to/src/tbb"
    # Find the actual directory name in build/ - it will be something like gnu_X.X_cxx11_64_release
    export TBBLIB="$(find $TBBROOT/build -maxdepth 1 -type d -name '*release' | head -n1)"
    export LD_LIBRARY_PATH="$TBBLIB:$LD_LIBRARY_PATH"
    ```
    
    Alternatively, manually find and set the directory:
    ```bash
    ls $TBBROOT/build/
    # Use the directory that contains .so files (e.g., gnu_9.4_cxx11_64_release)
    export TBBLIB="$TBBROOT/build/gnu_9.4_cxx11_64_release"  # adjust to your actual directory
    ```

- **[boost >= 1.58.0](https://www.boost.org/users/download/ "boost download page")**  
    Although libraries in Boost are mostly header-only, we've also used some libraries that require compilation.  
    Tested with Boost 1.90.0 in Dec 2025. To build:
    ```bash
    wget https://archives.boost.io/release/1.90.0/source/boost_1_90_0.tar.gz
    tar -xzf boost_1_90_0.tar.gz
    cd boost_1_90_0
    ./bootstrap.sh --prefix=./
    ./b2
    ```
    Set environment variables and configure dynamic linker (note: no spaces around `=`):
    ```bash
    export BOOSTDIR="/path/to/boost_1_90_0"
    export LD_LIBRARY_PATH="$BOOSTDIR/stage/lib:$LD_LIBRARY_PATH"
    ```

### Compile

```
make all
```
A dynamic binary `router` will be compiled in `bin/`  

## Run

```
./bin/router <input file> <output file> [-t arg] [-e arg] [--ripUp arg]
```
- `-t` : number of threads, default = 8
- `-e` : convergence criterion for Lagrangian Relaxation, default = 0.27%
- `--ripUp` : number of times to perform rip up and reroute, default = 1
- For more information :
```
./bin/router -h
```

- The parameters of each test case for our experiments are set as :  

   | test case       |`-t` | `-e`  |`--ripUp`|
   | :---:           |:---:|:---:  |:---:    |
   | synopsys01      | 8   | 0.27% | 1       |  
   | synopsys02      | 8   | 0.27% | 2       |  
   | synopsys03      | 8   | 0.27% | 3       |  
   | synopsys04      | 8   | 0.27% | 2       |  
   | synopsys05      | 8   | 0.27% | 2       |  
   | synopsys06      | 8   | 0.05% | 2       |  
   | synopsyshidden01| 8   | 0.05% | 3       |  
   | synopsyshidden02| 8   | 0.05% | 2       |  
   | synopsyshidden03| 8   | 0.05% | 3       | 

- The static version is also compiled and stored in `sbin/`, see `sbin/readme.md` for more information.

## Test

- Download test cases from the official [ICCAD 2019 Contest](http://iccad-contest.org/2019/problems.html) Problem B/test cases
- Place it under `FPGA_routing/` and leave the filename unchanged  
    Namely like :
    ```
    FPGA_routing
    ├── docs
    │    └── documentation.pdf
    ├── evaluator
    │    └── ...
    ├── regression
    │    └── ...
    ├── testcases
    │    └── SampleInput
    │    └── synopsys01.txt
    │    └── ...
    │    └── ...
    │    └── ...
    .
    .
    .
    ```
    The relative path should be exactly like this for all the test cases to work

- Before running the test cases, compile the source code first :
    ```
    make all
    ```
    The object files need to be generated before testing

- To run multiple test cases consecutively, run any script you want to test :
    ```
    cd regression/
    ./regression.sh
    ```

- To run a specific test case, change to its directory and compile :
    ```
    cd regression/test/
    make
    ```

- `multiProcess.sh` runs all the test cases in parallel which requires a system with at least 32 cores

## Citation
If you find this code useful in your research, please cite our DAC 2020 paper:
```
@inproceedings{lin2020routing,
  title={Routing topology and time-division multiplexing co-optimization for multi-FPGA systems},
  author={Lin, Tung-Wei and Tai, Wei-Chen and Lin, Yu-Cheng and Jiang, Iris Hui-Ru},
  booktitle={2020 57th ACM/IEEE Design Automation Conference (DAC)},
  pages={1--6},
  year={2020},
  organization={IEEE}
}
```
## Contacts

Please direct any questions to this email address: waynelin567@gmail.com. I will reply as soon as possible.

### Maintainers
[waynelin567](https://github.com/waynelin567), [willytai](https://github.com/willytai), and [yclin7777777](https://github.com/yclin7777777)


