import archiveexport as ae

#channels = ae.list("/mnt/archiver/indexconfig.xml", pattern="ARIDI.*BPM1")

import datetime
now = datetime.datetime.now()
end = now-datetime.timedelta(days=1)
start = end-datetime.timedelta(days=1)


# get all channels starting with AR
# channels = ae.list("/mnt/archiver/index", pattern="AR.*")
# print(channels)

channels = ae.list("/mnt/archiver/index")

# query 100 random channels out of those starting with AR
import random
my_randoms = random.sample(range(len(channels)), 100)
random_channels = [channels[i] for i in my_randoms]
data = ae.get_data("/mnt/archiver/index", channels=random_channels, start=now, get_units=True, get_status=True, get_info=True)
print(data)

channels = ["ARIMA-CS-04LA:PS-ERRORLST", 
             "X05LA-ID1-CHU2:I-SRDIFF",
             "ALBMA-CV-2:I-NOISE",
             "ILUZL-2100-EDSPS:W01_L_D-WA",
             "ARIMA-QMG-06:PS-ERRORSTR",
             "ARIMA-CV-05LD:LOAD-ERR", "ABOMA-QD:ERF-DSBR"]
 
data = ae.get_data("/mnt/archiver/index", channels=channels, start=now, get_units=True, get_status=True, get_info=True)
print(data)

#rminate called after throwing an instance of 'GenericException'
#  what():  ../RawDataReader.cpp (80): Channel 'ARIMA-SE:PS-STERRSTR':
#../RTree.cpp (46): Datablock filename read error @ 0x19FCF92F
#Aborted (core dumped)
