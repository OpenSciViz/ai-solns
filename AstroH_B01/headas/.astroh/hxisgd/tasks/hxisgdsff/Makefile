CVSRCS_ID           := "$$Name: AstroH_B01 $$ $$Id: Makefile,v 1.12 2012/06/28 16:21:59 dhon Exp $$"

$(info $$Name: AstroH_B01 $$ $$Id: Makefile,v 1.12 2012/06/28 16:21:59 dhon Exp $$)

# hon:
# not that this version of the makefile somehow lacks the dependency logic 
# to ensure libahvslice.so is compiled first and available to link with?
# so for now must manually hmake within lib/src first then hmake here?

HD_COMPONENT_NAME		= astroh

HD_COMPONENT_VERS		= 

# hon: manual make from ./lib works just fine,
# but this lacks a built-in dependency?

SYSLIBS             := -lpthread ${SYSLIBS}
HD_SUBDIRS			    = ./lib 

HD_TEST_SUBDIRS			= 

#HD_LIB              = ./lib
#HD_TOP_LIB           = ./lib
#HD_LIBRARY_ROOT     =
#HD_SUBDIRS          = ./lib ${HD_SUBDIRS}

HD_CXXTASK		= hxisgdsff

HD_CXXTASK_SRC_cxx	= ${HD_CXXTASK}.cxx \
doInstrum.cxx \
doWork.cxx \
finalize.cxx \
getPar.cxx \
initialize.cxx \
shutDown.cxx \
startUp.cxx

HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I./ -I../../lib/ahhxi -I../../lib/ahsgd -I../../lib/ahtlm \
                 -I../../../mission/lib/ahmapp -I../../../mission/lib/common \
                 -I../../../gen/lib/ahapp -I../../../gen/lib/ahfits -I../../../gen/lib/ahmath \
								 -I../../../gen/lib/ahgen -I../../../gen/lib/ahlog -I../../../gen/lib/st_stream

#HD_CXXFLAGS		=  -g -O00 ${HD_STD_CXXFLAGS} -I ./ -I../../lib/ahfits ../../lib/ahgen 

#HD_LFLAGS := $(HD_LFLAGS) -z muldefs -z nodefs
#
HD_CXXLIBS		:= $(HD_LFLAGS) -L../../lib/ahhxi/src -L../../lib/ahsgd/src -L../../lib/ahtlm/src \
								 -L../../../mission/lib/ahmapp/src -L../../../mission/lib/common/src \
								 -L../../../gen/lib/ahapp/src -L../../../gen/lib/ahfits/src -I../../../gen/lib/ahmath/src \
								 -L../../../gen/lib/ahgen/src -I../../../gen/lib/ahlog/src -I../../../gen/lib/st_stream/src \
                 -lahhxi -lahsgd -lahtlm \
                 -lahfits -lahmath -lahgen -lahlog -lst_stream ${HD_STD_LIBS} ${SYSLIBS}
# 							 -lahmapp -lahcommon \
#                -lahapp -lahfits -lahmath -lahgen -lahlog -lst_stream ${HD_STD_LIBS} ${SYSLIBS}

HD_INSTALL_TASKS	= ${HD_CXXTASK}

HD_INSTALL_PFILES	= ${HD_CXXTASK}.par

HD_INSTALL_HELP		= ${HD_CXXTASK}.html

include ${HD_STD_MAKEFILE}

