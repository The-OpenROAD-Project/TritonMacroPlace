source helpers.tcl
set design gcd_mem5 
set techDir nangate45-bench/tech
set designDir nangate45-bench/design/${design}

read_liberty ${techDir}/NangateOpenCellLibrary_typical.lib
read_liberty ${techDir}/fakeram45_64x7.lib

read_lef ${techDir}/NangateOpenCellLibrary.lef
read_lef ${techDir}/fakeram45_64x7.lef

read_def ${designDir}/${design}.def
read_sdc ${designDir}/${design}.sdc

macro_placement -global_config ${designDir}/halo_0.5.cfg

set def_file [make_result_file "gcd-mem5-test-01-mplace.def"]
write_def $def_file
report_file $def_file
