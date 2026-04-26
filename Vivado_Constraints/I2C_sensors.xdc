## sensor_i2c.xdc
## I2C1 via EMIO, externalized as I2C_sensors, routed to PMOD JC

set_property PACKAGE_PIN V15 [get_ports {I2C_sensors_scl_io}] ; # JC1
set_property PACKAGE_PIN W15 [get_ports {I2C_sensors_sda_io}] ; # JC2

set_property IOSTANDARD LVCMOS33 [get_ports {I2C_sensors_scl_io}]
set_property IOSTANDARD LVCMOS33 [get_ports {I2C_sensors_sda_io}]