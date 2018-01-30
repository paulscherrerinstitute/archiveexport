import archiveexport as ae

print(ae.list("/mnt/archiver/index", pattern="ARIDI.*BPM1"))

print(ae.get_data("/mnt/archiver/index", channel="ARIDI-VME-BPM12:LOAD"))