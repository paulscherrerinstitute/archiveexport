#		-I/opt/anaconda3/lib/python3.6/site-packages/numpy/core/include/ 

install:
	# g++ main.cpp -o ArchiveExportPy -I/opt/epics/base/include/ -I/opt/epics/base/include/os/Linux/  -I/opt/epics/extensions/src/ChannelArchiver/Storage -I/opt/epics/extensions/src/ChannelArchiver/Tools  -L/opt/epics/extensions/lib/linux-x86_64/ -lStorage -lTools -D ARCH_VERSION_TXT=\"0.9.0\" -L/opt/epics/base/lib/linux-x86_64/ -lca -lCom
	g++ -c -fPIC archiveexport.cpp -o archiveexport.o \
		-I/opt/anaconda3/include/python3.6m/ \
		-I/opt/epics/extensions/src/ChannelArchiver/Storage \
		-I/opt/epics/extensions/src/ChannelArchiver/Tools \
		-I/opt/epics/base/include/ \
		-I/opt/epics/base/include/os/Linux/ \
		-L/opt/epics/extensions/lib/linux-x86_64/ -lStorage -lTools \
		-L/opt/epics/base/lib/linux-x86_64/ -lCom -lca 
	g++ -shared archiveexport.o -o archiveexport.so \
		-I/opt/anaconda3/include/python3.6m/ \
		-I/opt/epics/extensions/src/ChannelArchiver/Storage \
		-I/opt/epics/extensions/src/ChannelArchiver/Tools \
		-I/opt/epics/base/include/ \
		-I/opt/epics/base/include/os/Linux/ \
		-L/opt/epics/extensions/lib/linux-x86_64/ -lStorage -lTools \
		-L/opt/epics/base/lib/linux-x86_64/ -lCom -lca

clean:
	rm archiveexport.o archiveexport.so