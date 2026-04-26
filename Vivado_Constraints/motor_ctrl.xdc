## motor_ctrl.xdc
## Zybo Z7-20 motor control outputs on PMOD JE
## Using JE avoids the JA/XADC PMOD, whose routing/filtering can limit digital performance.

## Motor 0 -> TB67 #0
set_property PACKAGE_PIN V12 [get_ports {motor_in1_0[0]}] ; # JE1
set_property PACKAGE_PIN W16 [get_ports {motor_in2_0[0]}] ; # JE2

## Motor 1 -> TB67 #1
set_property PACKAGE_PIN J15 [get_ports {motor_in1_0[1]}] ; # JE3
set_property PACKAGE_PIN H15 [get_ports {motor_in2_0[1]}] ; # JE4

## Motor 2 -> TB67 #2
set_property PACKAGE_PIN V13 [get_ports {motor_in1_0[2]}] ; # JE7
set_property PACKAGE_PIN U17 [get_ports {motor_in2_0[2]}] ; # JE8

## Motor 3 -> TB67 #3
set_property PACKAGE_PIN T17 [get_ports {motor_in1_0[3]}] ; # JE9
set_property PACKAGE_PIN Y17 [get_ports {motor_in2_0[3]}] ; # JE10

## All PMOD JE signals are 3.3 V CMOS
set_property IOSTANDARD LVCMOS33 [get_ports {motor_in1_0[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {motor_in2_0[*]}]