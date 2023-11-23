# ProcessorTracer

This is a implementation of trace IntelPT using perf.

You should run this code on Linux desktop, NOT virtual environment ( except Hyper-V in Windows, check this link [Enable Intel Performance Monitoring Hardware in a Hyper-V virtual machine](https://learn.microsoft.com/en-us/windows-server/virtualization/hyper-v/manage/performance-monitoring-hardware) ).

## Requirements

Tools
``` shell
sudo apt install cmake g++ make
```

[libipt v2.1](https://github.com/intel/libipt.git) ( for decoding )

+ optional : If you want to use tools in libipt ptdump, ptxed and other things, you should modify CMakeLists.txt in libipt before run `cmake`.
    
    ex) `option(PTDUMP "Enable ptdump, a packet dumper")` -> `option(PTDUMP "Enable ptdump, a packet dumper" ON)`
``` shell
git clone https://github.com/intel/libipt.git -b v2.1
cd libipt
mkdir build && cd build
cmake ..
make
sudo make install
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
sudo ./bin/trace_test <executable file>
```

Then, you can get files `traced_data.bin` and `maps_data`.

Also, you can dump `traced_data.bin` with `ptdump` in libipt.
```shell
./ptdump traced_data.bin
```

Decoding traced data file will be implemented.
