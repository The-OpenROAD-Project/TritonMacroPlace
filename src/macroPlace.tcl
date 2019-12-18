
sta::define_cmd_args "macro_placement" {
  [-global_config global_config_file]\
  [-local_config local_config_file]}

proc macro_placement { args } {
  sta::parse_key_args "macro_placement" args \
    keys {-global_config -local_config} flags {}

  set mp [get_tritonmp]

  if { ![info exists keys(-global_config)] } {
    puts "Error: -global_config must exist."
  } else {
    set global_config_file $keys(-global_config)
    if { [file readable $global_config_file] } {
      $mp import_global_config $global_config_file
    } else {
      puts "Warning: cannot read $global_config_file"
    }
  }

  if { [info exists keys(-local_config)] } {
    set local_config_file $keys(-local_config)
    if { [file readable $local_config_file] } {
      $mp import_local_config $local_config_file
    } else {
      puts "Warning: cannot read $local_config_file"
    }
  }
  
  if { [ord::db_has_rows] } {
    $mp place_macros 
    puts "Total Extracted Solution: [$mp get_solution_count]"
  } else {
    puts "Error: no rows defined in design. Use initialize_floorplan to add rows."
  }
}
