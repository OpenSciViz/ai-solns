all: clean data_io cppdata_io test

data_io:
	gcc -g -std=c11 -fPIC -c $@.c
	ld -g -shared data_io.o -o $@.so 
#	ld -g --entry=main data_io.o -o $@ -lm -lc 
	gcc -g -std=c11 data_io.c -o $@ -lm -lc 

cppdata_io:
	g++ -g -std=c++11 data_io.cc -o $@ -lm -lc 

clean:
	-rm -rf *.o *.so data_io cppdata_io

test:
	-@echo test data_io
	-./data_io
	-@echo test c++ compiled data_io
	-./cppdata_io

