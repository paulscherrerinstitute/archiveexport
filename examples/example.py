import archiveexport as ae
import datetime

# find channels by pattern
index_file = "/net/slsmcarch-navf/export/archiver-data-mc/archive_ST/index"
channels = ae.list(index_name=index_file, pattern="ARIDI.*BPM1")

# calculate start and date
now = datetime.datetime.now()
end = now-datetime.timedelta(days=1)
start = end-datetime.timedelta(minutes=1)

# query data
data = ae.get_data(index_name=index_file, channels=channels, start=start, end=end, get_units=True, get_status=True, get_info=True)
print(data)

