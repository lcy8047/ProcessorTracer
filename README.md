# ProcessorTracer

This is a implementation of trace IntelPT using perf.

You should run this code on Linux desktop, NOT virtual environment ( except Hyper-V in Windows, check this link [Enable Intel Performance Monitoring Hardware in a Hyper-V virtual machine](https://learn.microsoft.com/en-us/windows-server/virtualization/hyper-v/manage/performance-monitoring-hardware) ).

## Requirements

[libipt](https://github.com/intel/libipt.git)

``` shell
sudo apt-get install cmake gcc make
```

## Build

Make sure libipt is installed.

``` shell
    git clone https://github.com/lcy8047/ProcessorTracer
    cd ProcessorTracer
    mkdir build && cd build
    cmake ..
    make
```

## Run

``` shell
    ./bin/trace_test <executable file>
```

Then, you can get files `traced_data.bin` and `maps_data`

Decoding traced data file will be implemented.
