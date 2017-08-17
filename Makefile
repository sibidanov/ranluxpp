CXX = g++
CXXFLAGS = -O3 -Iinc -mavx2
RLIB = libranlux++.a

# use assembly optimized version of the skipping
ASMSKIP = yes
ifeq ($(ASMSKIP),yes)
  CXXFLAGS += -DASMSKIP
  ASMOBJ = src/skipstates.o
endif

all: ranluxpp_test ranlux_test std_random_test

%.o: %.asm
	$(AS) -c -o $@ $<

%.o: %.cxx
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(RLIB): src/ranluxpp.o src/mulmod.o src/mul9x9mod.o src/divmult.o src/lcg2ranlux.o src/ranlux.o src/cpuarch.o $(ASMOBJ)
	ar cru $@ $^

ranlux_test: tests/ranlux_test.cxx $(RLIB)
	$(CXX) -o $@ $^ $(CXXFLAGS)

ranluxpp_test: tests/ranluxpp_test.cxx $(RLIB)
	$(CXX) -o $@ $^ $(CXXFLAGS)

std_random_test: tests/std_random_test.cxx
	$(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f ranlux_test ranluxpp_test std_random_test src/*.o src/*~ tests/*~ inc/*~ core *~ $(RLIB)

src/ranlux.o: inc/ranlux.h
src/ranluxpp.o: inc/ranluxpp.h
src/mulmod.o: inc/mulmod.h
src/cpuarch.o: inc/cpuarch.h
