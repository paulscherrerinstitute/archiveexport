import archiveexport as ae

channels = ae.list("/mnt/archiver/index", pattern="ARIDI.*BPM1")

print(channels)

# print(ae.get_data("/mnt/archiver/index", channels=["ARIDI-VME-BPM12:LOAD"]))

print(ae.get_data("/mnt/archiver/index", channels=channels))
