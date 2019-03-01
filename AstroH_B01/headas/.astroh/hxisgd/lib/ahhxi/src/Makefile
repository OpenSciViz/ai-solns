CVSRCS_ID           := "$$Name: AstroH_B01 $$ $$Id: Makefile,v 1.7 2012/07/02 22:24:52 dhon Exp $$"

$(info $$Name: AstroH_B01 $$ $$Id: Makefile,v 1.7 2012/07/02 22:24:52 dhon Exp $$)

HD_COMPONENT_NAME	= astroh

HD_COMPONENT_VERS	= 

HD_LIBRARY_ROOT		= ahhxi

HD_LIBRARY_SRC_cxx	= doHXI.cxx doHXItest.cxx HXItemplate.cxx ioHXI.cxx lutHXI.cxx

HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I./ -I../ -I../../ahtlm -I../../ahsgd -I../../../tasks/hxisgdsff \
                           -I../../../mission/lib/ahmapp -I../../../mission/lib/common \
                           -I../../../gen/lib/ahapp -I../../../gen/lib/ahfits -I../../../gen/lib/ahmath \
                           -I../../../gen/lib/ahgen -I../../../gen/lib/ahlog -I../../../gen/lib/st_stream

HD_INSTALL_LIBRARIES	= ${HD_LIBRARY_ROOT}

include ${HD_STD_MAKEFILE}
