all: clean data_io cppdata_io test

data_io:
	gcc -g -std=c11 -fPIC -c $@.c
#	ld -g -shared data_io.o -o $@.so 
#	ld -g --entry=main data_io.o -o $@ -lm -lc 
	gcc -g -std=c11 data_io.c -o $@ -lm -lc 
	ldd $@

cppdata_io:
	g++ -g -std=c++11 data_io.cc -o $@ -lm -lc 
	ldd $@

clean:
	-rm -rf *_wrap.* *.o *.so data_io cppdata_io

test: data_io cppdata_io
	-@echo test data_io
	-./data_io
	-@echo test c++ compiled data_io
	-./cppdata_io

ifndef SWIG
  SWIG := $(shell swig -version)
endif

ifndef PYINC
  PY2INC := /opt/conda2//include/python2.7
  PY3INC := /opt/conda3//include/python3.6
  PYINC := $(PY2INC)
endif

pyswig: clean
	@$(info $(SWIG))
	@$(info invoke swig -python on a header file that has a module declaration creates .py and _wrap.c module files)
	@$(info data_io.h and data_io.c are supplemented by generated files data_io_wrap.c and data_io.py)
	@$(info the resulting shared object swig_exampler.so provides the low-level python module shared library code)
	swig -python data_io.h
	gcc -fPIC -c data_io.c data_io_wrap.c -I$(PYINC)
	ld -shared data_io.o data_io_wrap.o -o data_io.so 

pytest:
	-python2 -c 'import data_io'
	-python3 -c 'import data_io'
