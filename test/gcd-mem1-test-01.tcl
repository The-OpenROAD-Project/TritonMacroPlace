source helpers.tcl
set design gcd_mem1 
set techDir nangate45-bench/tech/ 
set designDir nangate45-bench/design/${design}

mplace_external mp

# mp import_lef $techDir/NangateOpenCellLibrary.lef
set lefList [glob -directory ${techDir} *.lef]
foreach lef $lefList {
  mp import_lef $lef
}

mp import_def ${designDir}/${design}.def
mp import_verilog ${designDir}/${design}.v
mp import_sdc ${designDir}/${design}.sdc

set libList [glob -directory ${techDir} *.lib]
foreach lib $libList {
  mp import_lib $lib
}

mp import_global_config ${designDir}/halo_0.5.cfg
mp place_macros

set def_file [make_result_file "gcd-mem1-test-01-mplace.def"]
mp export_def $def_file

puts "PossibleSol : [mp get_solution_count]"
report_file $def_file

exit
