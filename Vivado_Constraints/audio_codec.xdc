## audio_codec.xdc
## Zybo Z7 SSM2603 playback-only constraints

## I2S playback signals
set_property -dict { PACKAGE_PIN R17 IOSTANDARD LVCMOS33 } [get_ports { audio_mclk }]   ; # AC_MCLK
set_property -dict { PACKAGE_PIN R19 IOSTANDARD LVCMOS33 } [get_ports { audio_bclk }]   ; # AC_BCLK
set_property -dict { PACKAGE_PIN R18 IOSTANDARD LVCMOS33 } [get_ports { audio_pbdat }]  ; # AC_PBDAT
set_property -dict { PACKAGE_PIN T19 IOSTANDARD LVCMOS33 } [get_ports { audio_pblrc }]  ; # AC_PBLRC

## Codec mute control
set_property -dict { PACKAGE_PIN P18 IOSTANDARD LVCMOS33 } [get_ports { audio_mute_tri_o[0] }]
## Codec I2C control bus
## These are the usual flattened names when the external interface is named "audio_I2C".
set_property -dict { PACKAGE_PIN N18 IOSTANDARD LVCMOS33 } [get_ports { audio_I2C_scl_io }] ; # AC_SCL
set_property -dict { PACKAGE_PIN N17 IOSTANDARD LVCMOS33 } [get_ports { audio_I2C_sda_io }] ; # AC_SDA