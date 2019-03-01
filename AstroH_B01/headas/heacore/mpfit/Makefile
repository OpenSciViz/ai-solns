HD_COMPONENT_NAME	= heacore

HD_COMPONENT_VERS	=

HD_LIBRARY_ROOT         = mpfit

HD_CFLAGS		= ${HD_STD_CFLAGS}

HD_INSTALL_LIBRARIES	= ${HD_LIBRARY_ROOT}

HD_INSTALL_HEADERS	= mpfit.h

HD_LIBRARY_SRC_c	= mpfit.c

include ${HD_STD_MAKEFILE}

testmpfit: testmpfit.c libmpfit.so
	$(CC) testmpfit.c -o testmpfit -L. -lmpfit -lm

README: README.html
	sed -e '/^</ d' -e 's|</?[bi]>||g' -e 's/&lt;/</g' -e 's/&gt;/>/g' -e 's/\&amp;/\&/g' README.html > README

${HD_LIBRARY_SRC_c:.c=.${OSUF}}: mpfit.h
