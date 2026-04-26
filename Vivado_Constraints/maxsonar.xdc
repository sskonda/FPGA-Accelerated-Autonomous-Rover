## maxsonar_uart.xdc
## Pmod MAXSONAR on Zybo Z7 JD

set_property PACKAGE_PIN T15 [get_ports {maxsonar_tx}] ; # JD2
set_property PACKAGE_PIN P14 [get_ports {maxsonar_rx}] ; # JD3

set_property IOSTANDARD LVCMOS33 [get_ports {maxsonar_tx}]
set_property IOSTANDARD LVCMOS33 [get_ports {maxsonar_rx}]