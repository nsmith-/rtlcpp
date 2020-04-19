CXX=g++ -Wall --std=c++11 -O2
all: rtl

rtl: rtl.o rtldevice.o nco.o
	$(CXX) -lrtlsdr -lpthread -o $@ $^

rtl.o: rtl.cc rtldevice.h nco.h buffer.h
	$(CXX) -c $<

rtldevice.o: rtldevice.cc rtldevice.h buffer.h
	$(CXX) -c $<

nco.o: nco.cc nco.h
	$(CXX) -c $<
