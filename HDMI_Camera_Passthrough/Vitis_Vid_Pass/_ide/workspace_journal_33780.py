# 2026-02-16T17:38:00.576462400
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis_Vid_Pass")

platform = client.get_component(name="vitis_vid_pass")
status = platform.build()

comp = client.get_component(name="pcam_passthrough")
comp.build()

vitis.dispose()

