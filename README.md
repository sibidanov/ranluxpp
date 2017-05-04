# Introduction

This program is an implementation of Linear Congruential Generator
(LCG) defined as:

x_{i+1} = A * x_{i} mod m

in long integer arithmetic with the modulus m = 2^576 - 2^240 + 1 and
the multiplier A = a^p, a = m-(m-1)/2^24 and p = 24..thousands,
where m and a are derived from the subtract-with-borrow random number
generator aka RANLUX. It is a well known high statistical quality
random number generator included in the modern C++ standard.

One step of the subtract-with-borrow generator uses only one
subtraction of 24 bit numbers and carry bit propagation and is
equivalent to one modular multiplication (a * x mod m) so the generator
presents a clever way to avoid large integer arithmetic. Despite
RANLUX uses only simple arithmetic operations it is one of the slowest
generators on the market since internally large portion of generated
numbers are wasted or skipped to break correlation between nearby
numbers in the sequence to achieve high statistical quality. Whereas
in the suggested approach the skipping is equivalent again to only one
modular multiplication with the multiplier A = (a^p mod m), where p-24
is the number of skipped elements in the RANLUX sequence.

The core of the generator is the fast multiplication of two 576 bit
numbers which is implemented in assembler for AMD64 architecture using
the simple school-book method with 81 64x64 bit multiplication. Recent
progress in CPU development allows to achieve speed 119 (CPU clock) per
576x576 bit multiplication and in conjunction with modular reduction
achieved speed is 155 (CPU clock) per modular multiplication at
Skylake CPU. At the moment there are three versions suitable for
generic AMD64 CPUs, with mulx and adcx/adox instructions available.

Finally, generation speed achieved (20 clock/float) is order of
magnitude faster than the usual approach, for example, GCC C++ RANLUX
implementation where 198 clock/float is measured at Haswell CPU.

The conventional RANLUX algorithm is also optimized to get profit from
modern CPU improvements. Scalar as well as SIMD optimized versions are
implemented. The achieved speed of 40 clock/float is only two times
less than in the LCG approach and those optimizations can be easily
applied to RANLUX implementations in other packages. The SSE2 and AVX2
versions is approximately 4 and 8 times faster correspondingly.


# File descriptions

src/mul9x9mod.asm  -- modular multiplication code.
src/mulmod.cxx     -- C interface with GCC function multiversioning to the modular multiplication code.
src/ranluxpp.cxx   -- generator itself using modular multiplication.
src/ranlux.cxx     -- optimized version of the conventional RANLUX algorithm.
src/skipstates.asm -- asm optimization for hardware carry bit propagation in the conventional RANLUX algorithm.

tests/ranluxpp_test.cxx   -- usage example and benchmarks of the generator with modular multiplication.
tests/ranlux_test.cxx     -- usage example and benchmarks of the generators based on the conventional RANLUX algorithm.
tests/std_random_test.cxx -- benchmarks of the standard C++ random number generators.


# Compilation

Type "make" in this directory to build the generator library and test executables.


# Tests and benchmarks

Type "./ranluxpp_test", "./ranlux_test" or "./std_random_test" to see the command help and the command options.


# Contact

Alexei Sibidanov <sibid@uvic.ca>
