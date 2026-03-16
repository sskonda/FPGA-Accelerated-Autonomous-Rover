# 2026-02-15T20:14:48.692212400
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis_Vid_Pass")

platform = client.get_component(name="vitis_vid_pass")
status = platform.build()

comp = client.get_component(name="pcam_passthrough")
comp.build()

status = platform.build()

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

status = platform.build()

comp.build()

vitis.dispose()

