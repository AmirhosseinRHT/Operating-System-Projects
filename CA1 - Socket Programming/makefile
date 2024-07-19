CC = g++
JSON = cJSON.c
.PHONY: all clean

all: restaurant.out customer.out supplier.out

restaurant.out: restaurant.c
	$(CC) restaurant.c $(JSON) -o restaurant.out

customer.out: customer.c
	$(CC) customer.c $(JSON) -o customer.out

supplier.out: supplier.c
	$(CC) supplier.c $(JSON) -o supplier.out

clean:
	rm -f *.out logs/*.txt