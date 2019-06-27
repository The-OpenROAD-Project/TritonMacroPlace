# Global IP configuration parameters. 
#### By default, these values are used for all IPs, unless specified in local configuration file. 
* All Units in um (micrometer)
* The values in this file do not correspond to any foundry enablement
#### Global Fin Pitch value. 
* This is fin pitch value for FINFET nodes. 

      set ::FIN_PITCH 0.040

#### Global Row Height. This is the height of a single height standard cell. Also, the height of SITE. 
* Unless otherwise specified in local configuration file, this value is used for all IPs

      set ::ROW_HEIGHT 0.400

#### Global SITE width. This is the width of a SITE defined in tech LEF.
* Unless otherwise specified in local configuration file, this value is used for all IPs.

      set ::SITE_WIDTH 0.010
  
#### Global halo Width for vertical edge of macros. 
* This is halo width value to be used for each IP along the vertical edge. This is essential to ensure proper routing to macro pins.

      set ::HALO_WIDTH_V 2

#### Global halo Width for horizontal edge of macros. 
* This is halo width value to be used for each IP along the horizontal edge. This is essential to ensure proper routing to macro pins.

      set ::HALO_WIDTH_H 2
  
#### Global channel width for vertical edge of macros. 
* This is channel width value to be used for each IP along the vertical edge. This is essential for buffer placement and PDN connectivity of these cells.

      set ::CHANNEL_WIDTH_V 25
  
#### Global channel width for horizontal edge of macros.
* This is channel width value to be used for each IP along the horizontal edge. This is essential for buffer placement and PDN connectivity of these cells.

      set ::CHANNEL_WIDTH_H 25

#### Example
* [example_IP_global.cfg](example_IP_global.cfg)

