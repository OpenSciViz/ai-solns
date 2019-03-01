HD_COMPONENT_NAME	= astroh

HD_COMPONENT_VERS	= 

HD_CXXTASK		= coordevt

HD_CXXTASK_SRC_cxx	= coordevt.cxx param.cxx earthvel.cxx info.cxx

HD_CXXFLAGS		= ${HD_STD_CXXFLAGS}

HD_CXXLIBS		= ${HD_LFLAGS} -lahgen -lahlog -lst_stream \
			  -lrew -lcoord -lcoordfits -lrandom \
			  -l${ATFUNCTIONS} -lparam_wrappers\
			  ${HD_STD_LIBS} ${SYSLIBS}

HD_INSTALL_TASKS	= ${HD_CTASK}

HD_INSTALL_PFILES	= coordevt.par

HD_INSTALL_HEADERS	=

HD_INSTALL_HELP		= coordevt.html

include ${HD_STD_MAKEFILE}
