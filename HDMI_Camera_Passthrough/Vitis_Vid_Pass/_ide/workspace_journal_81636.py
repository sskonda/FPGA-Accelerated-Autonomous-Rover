# 2026-02-11T15:12:32.475666300
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis_Vid_Pass")

platform = client.create_platform_component(name = "vitis_vid_pass",hw_design = "$COMPONENT_LOCATION/../../Vivado_Project2_CV/Zybo-Z7-HW/video_passthrough.xsa",os = "standalone",cpu = "ps7_cortexa9_0",domain_name = "standalone_ps7_cortexa9_0")

platform = client.get_component(name="vitis_vid_pass")
status = platform.build()

comp = client.create_app_component(name="pcam_app",platform = "$COMPONENT_LOCATION/../vitis_vid_pass/export/vitis_vid_pass/vitis_vid_pass.xpfm",domain = "standalone_ps7_cortexa9_0")

status = platform.build()

comp = client.get_component(name="pcam_app")
comp.build()

client.delete_component(name="pcam_app")

vitis.dispose()

