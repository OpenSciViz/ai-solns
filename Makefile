$(info TBD compile make rules)
SHELL := /bin/bash

all: clean data_io flow cppflow cppdata_io test

data_io:
	gcc -g -std=c11 -fPIC -c $@.c
#	ld -g -shared data_io.o -o $@.so 
#	ld -g --entry=main data_io.o -o $@ -lm -lc 
	gcc -g -std=c11 $@.c -o $@ -lm -lc 

flow:
	gcc -g -std=c11 -fPIC -c $@.c
	gcc -g -std=c11 $@.c -o $@ -lm -lc 
#	gcc -g -std=c11 $@.c -o $@ data_io.o -lm -lc 
	ldd $@

cppdata_io:
	g++ -g -std=c++11 data_io.cc -o $@ -lm -lc 

cppflow:
	g++ -g -std=c++11 flow.cc -o $@ -lm -lc 
#	g++ -g -std=c11 $@.c -o $@ data_io.o -lm -lc 
	ldd $@

test: data_io cppdata_io
	-@echo test data_io
	-./data_io
	-@echo test c++ compiled data_io
	-./cppdata_io
	-@echo test flow
	-./flow
	-@echo test cppflow
	-./cppflow

ifndef SWIG
  SWIG := $(shell swig -version)
endif

ifndef PYINC
  PY2 := $(shell python -c 'import sys ; vvv=str(sys.version_info.major)+"."+str(sys.version_info.minor)+"."+str(sys.version_info.micro);print vvv')
  PY3 := $(shell python3 -c 'import sys ; vvv=str(sys.version_info.major)+"."+str(sys.version_info.minor)+"."+str(sys.version_info.micro);print(vvv)')
  PY2INC := /opt/conda2//include/python2.7
  PY3INC := /opt/conda3//include/python3.6
endif

py2swig: clean
	@$(info $(SWIG) python2 is the default)
	@$(info invoke swig -python on a header file that has a module declaration creates .py and _wrap.c module files)
	@$(info data_io.h and data_io.c are supplemented by generated files data_io_wrap.c and data_io.py)
	@$(info the resulting shared object swig_exampler.so provides the low-level python module shared library code)
	swig -python data_io.h
	gcc -fPIC -c data_io.c data_io_wrap.c -I$(PY2INC)
	ld -shared data_io.o data_io_wrap.o -o data_io.so.$(PY2) 

py3swig: clean
	@$(info $(SWIG) python3)
	@$(info invoke swig -python3 on a header file that has a module declaration creates .py and _wrap.c module files)
	@$(info data_io.h and data_io.c are supplemented by generated files data_io_wrap.c and data_io.py)
	@$(info the resulting shared object swig_exampler.so provides the low-level python module shared library code)
	swig -python data_io.h
	gcc -fPIC -c data_io.c data_io_wrap.c -I$(PY3INC) 
	ld -shared data_io.o data_io_wrap.o -o data_io.so.$(PY3) 

py2test: py2swig
	-unlink -f data_io.so && ln -s data_io.so data_io.so.$(PY2)
	-python -c 'import data_io'

py3test: py3swig
	-unlink -f data_io.so && ln -s date_io.so data_io.so.$(PY3)
	-python3 -c 'import data_io'

pytest: py2test py3test
	-echo tested all python

clean:
	@$(info swig module cdata_io.py should be regenerated) 
	-rm -rf *_wrap.* *.o *.so *.so.* *.pyc cdata_io.py data_io cppdata_io flow cppflow

realclean: clean
	-@echo $(PY2) $(PY3)
	-@echo reset python virtualenv if in use \(TBD\)

