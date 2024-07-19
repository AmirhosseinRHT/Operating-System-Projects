COMPILE_WITH := g++ -std=c++17

all: resource.out building.out bill.out main

main: object/main.o object/handler.o
	$(COMPILE_WITH) object/handler.o object/main.o -o main

resource.out: source/resource.cpp source/resource.hpp
	$(COMPILE_WITH) source/resource.cpp -o program/resource.out
	
building.out: source/building.cpp source/building.hpp
	$(COMPILE_WITH) source/building.cpp -o program/building.out

bill.out: source/bill.cpp source/bill.hpp
	$(COMPILE_WITH) source/bill.cpp -o program/bill.out

object/handler.o: source/handler.cpp source/handler.hpp
	$(COMPILE_WITH) -c source/handler.cpp -o object/handler.o

object/main.o: main.cpp
	$(COMPILE_WITH) -c main.cpp -o object/main.o

.PHONY: clean
clean:
	rm -f *.o main logs/*.txt program/*.out object/*.o