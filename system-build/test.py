import archiveexport as ae

# channels = ae.list("/mnt/archiver/indexconfig.xml", pattern="ARIDI.*BPM1")

try:
  channels = ae.list("/mnt/archiver/indexx")
except:
    print("exception cought")
# print(channels)

import datetime
now = datetime.datetime.now()
end = now-datetime.timedelta(minutes=60)
start = end-datetime.timedelta(seconds=0.1)


# get 100 random channels

import random
my_randoms = random.sample(range(len(channels)), 100)
#for i in range(0, len(channels), 10):
for i in my_randoms:
    data = ae.get_data("/mnt/archiver/index", channels=channels[i:i+1], start=now)
    print(data)

# ARIMA-CS-04LA:PS-ERRORLST
# X05LA-ID1-CHU2:I-SRDIFF
# ALBMA-CV-2:I-NOISE

# data = ae.get_data("/mnt/archiver/indexconfig.xml", channels=["X05LA-ID1-CHU2:I-SRDIFF"], start=start, end=end)
# print(data)