CVSRCS_ID           := "$$Name: AstroH_B01 $$ $$Id: Makefile,v 1.5 2012/07/15 20:57:16 dhon Exp $$"

$(info $$Name: AstroH_B01 $$ $$Id: Makefile,v 1.5 2012/07/15 20:57:16 dhon Exp $$)

HD_COMPONENT_NAME	= astroh

HD_COMPONENT_VERS	= 

HD_LIBRARY_ROOT		= ahsgd

HD_LIBRARY_SRC_cxx	= doSGD.cxx doSGDtest.cxx SGDtemplate.cxx ioSGD.cxx lutSGD.cxx

HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I./ -I../ -I../../ahtlm -I../../ahsgd -I../../../tasks/hxisgdsff \
                           -I../../../mission/lib/ahmapp -I../../../mission/lib/common \
                           -I../../../gen/lib/ahapp -I../../../gen/lib/ahfits -I../../../gen/lib/ahmath \
                           -I../../../gen/lib/ahgen -I../../../gen/lib/ahlog -I../../../gen/lib/st_stream

HD_INSTALL_LIBRARIES	= ${HD_LIBRARY_ROOT}

include ${HD_STD_MAKEFILE}
