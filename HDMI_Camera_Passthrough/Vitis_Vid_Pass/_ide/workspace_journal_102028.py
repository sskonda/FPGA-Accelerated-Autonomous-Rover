# 2026-02-15T21:12:58.795806
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis_Vid_Pass")

comp = client.create_app_component(name="pcam_passthrough",platform = "$COMPONENT_LOCATION/../vitis_vid_pass/export/vitis_vid_pass/vitis_vid_pass.xpfm",domain = "standalone_ps7_cortexa9_0")

comp = client.get_component(name="pcam_passthrough")
status = comp.import_files(from_loc="", files=["C:\Users\sanat\FPGA_Car_Project\Vitis_Vid_Pass\Zybo-Z7-20-Pcam-5C-sw.ide\Zybo-Z7-20-pcam-5c\src"])

platform = client.get_component(name="vitis_vid_pass")
status = platform.build()

status = platform.build()

comp = client.get_component(name="pcam_passthrough")
comp.build()

comp = client.get_component(name="pcam_passthrough")
status = comp.import_files(from_loc="$COMPONENT_LOCATION/../vitis_vid_pass/hw/sdt/drivers", files=["MIPI_CSI_2_RX_v1_0", "MIPI_D_PHY_RX_v1_0"], dest_dir_in_cmp = "src")

comp = client.get_component(name="pcam_passthrough")
status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

status = platform.build()

comp.build()

status = platform.build()

status = comp.clean()

status = platform.build()

comp.build()

vitis.dispose()

