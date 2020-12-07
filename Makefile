BOOSTDIR='/usr/include/boost'
LEDALIB='/usr/local/LEDA'
LEDAINCL='/usr/local/LEDA/incl'

compile:
	g++ project.cpp -O3 -o run -I$(BOOSTDIR) -lrt
	g++ ledabenchmark.cpp -o ledabenchmark -O3 -I$(LEDAINCL) -L$(LEDALIB) -lleda -lrt