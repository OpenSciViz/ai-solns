CVSRCS_ID           := "$$Name: AstroH_B01 $$ $$Id: Makefile,v 1.9 2012/06/29 15:46:02 dhon Exp $$"

$(info $$Name: AstroH_B01 $$ $$Id: Makefile,v 1.9 2012/06/29 15:46:02 dhon Exp $$)

HD_COMPONENT_NAME	= astroh

HD_COMPONENT_VERS	= 

HD_LIBRARY_ROOT		= ahtlm

HD_LIBRARY_SRC_cxx	= ahBits.cxx ahSlice.cxx ahTLM.cxx ahTLMIO.cxx ahTLMinfoCache.cxx ahErrHashFITS.cxx

HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I./ -I../ \
                 -I../../../../mission/lib/ahmapp -I../../../../mission/lib/common \
                 -I../../../../gen/lib/ahapp -I../../.../../gen/lib/ahfits -I../../../../gen/lib/ahmath \
								 -I../../../../gen/lib/ahgen -I../../../../gen/lib/ahlog -I../../../gen/lib/st_stream

#HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I ./ -I../../lib/ahfits ../../lib/ahgen 

# useful when refactoring code (but cnu-ld on frebsd/macos does not support this) 
#HD_LFLAGS := $(HD_LFLAGS) -z muldefs -z nodefs
#
HD_CXXLIBS		:= $(HD_LFLAGS) \
								 -L../../../../mission/lib/ahmapp/src -L../../../../mission/lib/common/src \
								 -L../../../../gen/lib/ahapp/src -L../../../../gen/lib/ahfits/src -I../../../../gen/lib/ahmath/src \
								 -L../../../gen..//lib/ahgen/src -I../../../../gen/lib/ahlog/src -I../../../../gen/lib/st_stream/src \
                 -lahmapp -lahcommon \
                 -lahapp -lahfits -lahmath -lahgen -lahlog -lst_stream ${HD_STD_LIBS} ${SYSLIBS}

HD_INSTALL_LIBRARIES	= ${HD_LIBRARY_ROOT}

include ${HD_STD_MAKEFILE}
