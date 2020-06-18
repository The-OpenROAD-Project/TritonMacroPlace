
sta::define_cmd_args "macro_placement" {
  [-global_config global_config_file]\
  [-local_config local_config_file]}

proc macro_placement { args } {
  sta::parse_key_args "macro_placement" args \
    keys {-global_config -local_config} flags {-die_area -no_timing}

  if { ![info exists keys(-global_config)] } {
    puts "Error: -global_config must exist."
    return
  } else {
    set global_config_file $keys(-global_config)
    if { [file readable $global_config_file] } {
      set_macro_place_global_config_cmd $global_config_file
    } else {
      puts "Warning: cannot read $global_config_file"
    }
  }

  if { [info exists keys(-local_config)] } {
    set local_config_file $keys(-local_config)
    if { [file readable $local_config_file] } {
      set_macro_place_local_config_cmd $local_config_file
    } else {
      puts "Warning: cannot read $local_config_file"
    }
  }

  if { [info exists flags(-die_area)] } {
    set_macro_place_die_area_mode_cmd true
  } else {
    set_macro_place_die_area_mode_cmd false
  }
 
  # disable sequential graph / timing 
  if { [info exists flags(-no_timing)] } {
    set_macro_place_timing_mode_cmd false 
  } 
  
  if { [ord::db_has_rows] } {
    place_macros_cmd
  } else {
    puts "Error: no rows defined in design. Use initialize_floorplan to add rows."
  }
}
