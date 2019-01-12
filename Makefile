data_io: clean
	gcc -g -std=c11 -fPIC -c data_io.c
	ld -g -shared data_io.o -o data_io.so 
#	ld -g --entry=main data_io.o -o data_io -lm -lc 
	gcc -g data_io.c -o data_io -lm -lc 

clean:
	rm -rf *.o *.so
