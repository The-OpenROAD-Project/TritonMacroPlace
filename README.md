# TritonMacroPlacer

ParquetFP based macro cell placer for OpenROAD.

## Flows
* Input: Initialy placed DEF from globalPlacer (e.g. RePlAce with timing-driven / mixed-height-cell)
* Output: Multiple DEF files that could be a solution. The maximum number of solutions is restricted by (#macros/3)^(3/2).   

## Getting Started
### Prerequisite
* GCC compiler and libstdc++ static library >= 4.8.5
* boost library >= 1.41
* bison (for lef/def parsers) >= 3.0.4
* tcl (for OpenSTA) >= 8.4
* Recommended OS: Centos6, Centos7 or Ubuntu 16.04

### Clone repo and submodules 
    $ git clone --recursive https://github.com/abk-openroad/TritonMacroPlace
    $ make build
    $ cd build
    $ cmake ..             // you may specify -DCMAKE_INSTALL_PREFIX to avoid installing in /usr/local/*
    $ make
    $ make install

### Manual
* [TritonMacroPlacer's arguments](doc/BinaryArguments.md)

### License
* BSD-3-clause License [[Link]](LICENSE)

### 3rd Party Module List
* ParquetFP from UMPack
* Eigen
* CImg
* [OpenSTA](https://github.com/abk-openroad/OpenSTA)
* LEF/DEF Parser (Modified by mgwoo)

