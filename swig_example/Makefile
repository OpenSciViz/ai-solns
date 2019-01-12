
ifndef SWIG
  SWIG := $(shell swig -version)
endif

$(info $(SWIG))
$(info invoke swig -python on a header file that has a module declaration creates .py and _wrap.c module files)
$(info swig_example.h and swig_wxample.c are supplemented by generated files swig_example_wrap.c and swig_example.py)

$(info the resulting shared object swig_exampler.so provides the low-level python module share library code)

pyswig: clean
	swig -python swig_example.h
	gcc -fPIC -c swig_example.c swig_example_wrap.c -I/opt/conda2//include/python2.7
	ld -shared swig_example.o swig_example_wrap.o -o swig_example.so 

clean:
	-\rm *.o *.so

