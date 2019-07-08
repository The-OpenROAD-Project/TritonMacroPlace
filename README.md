# TritonMacroPlacer

ParquetFP based macro cell placer for OpenROAD.

## Flows
* Input: Initial placed DEF of a mixed-size (macros + cells) netlist. Such a DEF is produced by RePlAce (timing-driven, mixed-size mode, aka "TD-MS-RePlAce").
* Output: A placed DEF with macro placements honoring halos, channels and cell row "snapping".  Approximately ceil((#macros/3)^(3/2)) "sets" corresponding to quadrisections of the initial placed mixed-size DEF are explored and packed using ParquetFP-based annealing. The best resulting floorplan according to a heuristic evaluation function is returned.

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

