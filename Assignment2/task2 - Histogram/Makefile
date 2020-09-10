# -*- Makefile -*-

#Target: Dependency
#	action

all: spmd worksharing

spmd: spmdhistogram.cpp
	/opt/global/gcc-7.2.0/bin/g++ -fopenmp spmdhistogram.cpp -O2 -ospmd

worksharing: forhistogram.cpp
	/opt/global/gcc-7.2.0/bin/g++ -fopenmp forhistogram.cpp -O2 -oworksharing

clean:
	rm -f spmd worksharing
