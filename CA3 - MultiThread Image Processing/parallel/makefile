COMPILE_WITH := g++ -std=c++11

all: ImageFilters.out

ImageFilters.out: object/main.o object/bmp.o object/timer.o
	$(COMPILE_WITH) object/timer.o object/bmp.o object/main.o -o ImageFilters.out

object/timer.o:source/timer.cpp source/timer.hpp source/defs.hpp
	$(COMPILE_WITH) -c source/timer.cpp -o object/timer.o

object/bmp.o: source/bmp.hpp source/bmp.cpp
	$(COMPILE_WITH) -c source/bmp.cpp -o object/bmp.o

object/main.o: main.cpp object/bmp.o object/timer.o
	$(COMPILE_WITH) -c main.cpp -o object/main.o

.PHONY: clean
clean:
	rm -f *.o ImageFilters.out object/*.o output.bmp
