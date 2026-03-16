# 2026-02-16T00:35:33.728193900
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis_Vid_Pass")

platform = client.get_component(name="vitis_vid_pass")
status = platform.build()

status = platform.build()

comp = client.get_component(name="pcam_passthrough")
comp.build()

status = comp.clean()

status = platform.build()

comp.build()

comp = client.get_component(name="pcam_passthrough")
status = comp.import_files(from_loc="$COMPONENT_LOCATION/src/ov5640", files=["I2C_Client.h"], dest_dir_in_cmp = "hdmi")

comp = client.get_component(name="pcam_passthrough")
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

status = comp.clean()

status = platform.build()

comp.build()

status = comp.clean()

status = platform.build()

comp.build()

vitis.dispose()

