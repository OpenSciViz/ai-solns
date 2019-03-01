HD_COMPONENT_NAME	= heacore

HD_COMPONENT_VERS	= 

HD_TEST_CTASK		= ape_test

HD_TEST_CTASK_SRC_c	= ape_test_main.c

HD_CFLAGS		= -I. -I.. -I../../include ${HD_STD_CFLAGS}

HD_CLIBS		= ${HD_LFLAGS} -l${PIL} -l${READLINE} ${SYSLIBS}

HD_INSTALL_TASKS	= ${HD_CTASK}

HD_CLEAN		= pfiles ape_large.par ape_test.par \
			  ape_test_unix_copy.par ape_test_win32_copy.par

default: test

all: default

include ${HD_STD_MAKEFILE}

run-test:
	@if [ ! -d pfiles ]; then mkdir pfiles; fi
	export PFILES=""; cat ape_test-input | ./ape_test; echo "test status = $$?"
