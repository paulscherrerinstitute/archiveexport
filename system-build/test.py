import archiveexport as ae

# channels = ae.list("/mnt/archiver/indexconfig.xml", pattern="ARIDI.*BPM1")

import datetime
now = datetime.datetime.now()
end = now-datetime.timedelta(minutes=60)
start = end-datetime.timedelta(seconds=0.1)


# get 100 random channels
channels = ae.list("/mnt/archiver/index", pattern="AR.*")
print(channels)
import random
my_randoms = random.sample(range(len(channels)), 100)
for i in my_randoms:
    data = ae.get_data("/mnt/archiver/index", channels=channels[i:i+1], start=now, get_units=True, get_status=True, get_info=True)
    print(data)

# channels = ["ARIMA-CS-04LA:PS-ERRORLST", 
#             "X05LA-ID1-CHU2:I-SRDIFF",
#             "ALBMA-CV-2:I-NOISE",
#             "ILUZL-2100-EDSPS:W01_L_D-WA",
#             "ARIMA-QMG-06:PS-ERRORSTR"]
# 
# data = ae.get_data("/mnt/archiver/indexconfig.xml", channels=channels, start=now, get_units=True, get_status=True, get_info=True)
# print(data)