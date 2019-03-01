HD_COMPONENT_NAME	= attitude

HD_COMPONENT_VERS	=

HD_SUBDIRS		= aberrator applyxform aspcorr aspect attcombine \
			  attdump combinexform coordinator det2att draw_teldef \
			  expomap getwcs getxform imagetrans imagexform minnow \
			  newmakefilter pointxform prefilter transform_wcs \
			  tristarid
# aligncal

HD_TEST_SUBDIRS		= ${HD_SUBDIRS}

include ${HD_STD_MAKEFILE}

subdir-newmakefilter:
	@if [ "x${CXX}" != "x:" ]; then \
		${HD_MAKE} hd-std-subdir HD_SUBDIR=newmakefilter; \
	fi
