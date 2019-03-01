HD_COMPONENT_NAME	= heacore

HD_COMPONENT_VERS	=

HD_SUBDIRS		= @h_components@ help refdata

HD_TEST_SUBDIRS		= heautils

HD_DISTCLEAN		= config.cache config.log config.status \
			  configure.lineno hd_install headas-setup hmakerc

HD_CLEAN		= ${HD_BLD_EXEC_PFX} ${HD_BLD_PFX}

HD_CTASK		= hd_install

HD_CTASK_SRC_c		= hd_install.c

HD_CFLAGS               = ${CFLAGS}

HD_INSTALL_TASKS	= hd_install

HD_INSTALL_SHELL_SCRIPTS	= hd_scanenv hmake lhea-fixperl

HD_INSTALL_PERL_SCRIPTS	= hdmk

HD_INSTALL_EXTRA	= install-BUILD_DIR install-symlinks

include ${HD_STD_MAKEFILE}

distclean: hd-std-distclean
	cp -pf Makefile.in Makefile

# Install BUILD_DIR; tap dance needed because the built tools live in BUILD_DIR
#-------------------------------------------------------------------------------
install-BUILD_DIR:
	@${HD_INSTALL} ${HD_BUILD_DIR} ${HD_EXEC_PFX}/BUILD_DIR ${HD_CP_P}
#-------------------------------------------------------------------------------

# Specializations for cfitsio
#-------------------------------------------------------------------------------
subdir-cfitsio:
	@cd ${CFITSIO_DIR}; ${MAKE} ${HD_TARGET} ${CFITSIO_OVERRIDE}
	@if [ "x${HD_TARGET}" = xclean -o "x${HD_TARGET}" = xdistclean ]; then \
		echo "rm -f ${CFITSIO_DIR}/libcfitsio*"; \
		rm -f ${CFITSIO_DIR}/libcfitsio*; \
		echo "rm -f ${CFITSIO_DIR}/so_locations"; \
		rm -f ${CFITSIO_DIR}/so_locations; \
		echo "rm -f ${CFITSIO_DIR}/fpack${BSUF}"; \
		rm -f ${CFITSIO_DIR}/fpack${BSUF}; \
		echo "rm -f ${CFITSIO_DIR}/funpack${BSUF}"; \
		rm -f ${CFITSIO_DIR}/funpack${BSUF}; \
	fi

CFITSIO_INSTALL_HEADERS	= cfortran.h f77_wrap.h fitsio.h \
			  fitsio2.h longnam.h region.h

all-cfitsio default-cfitsio: build-cfitsio publish-cfitsio

build-cfitsio:
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		${MAKE} subdir-cfitsio HD_TARGET=shared \
			CFITSIO_OVERRIDE="SHLIB_LD='${SHLIB_LD}' SHLIB_SUFFIX=${CFITSIO_VERS}${SHLIB_SUFFIX}"; \
		${MAKE} subdir-cfitsio HD_TARGET=fpack; \
		${MAKE} subdir-cfitsio HD_TARGET=funpack; \
	else \
		${MAKE} subdir-cfitsio HD_TARGET=; \
		${MAKE} subdir-cfitsio HD_TARGET=fpack; \
		${MAKE} subdir-cfitsio HD_TARGET=funpack; \
		${HD_INSTALL} ${CFITSIO_DIR}/libcfitsio.a \
			${CFITSIO_DIR}/lib${CFITSIO}.a ${HD_MV_F}; \
	fi

publish-cfitsio:
	@${MAKE} install-cfitsio HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_CP_P="${HD_LN_S}"

install-cfitsio:
	@for f in ${CFITSIO_INSTALL_HEADERS}; do \
	  hd_install_headers="$$hd_install_headers ${CFITSIO_DIR}/$$f"; \
	done; \
	${MAKE} install-headers HD_INSTALL_HEADERS="$$hd_install_headers"
	${MAKE} install-libraries HD_INSTALL_LIBRARIES=${CFITSIO_DIR}/${CFITSIO}
	${MAKE} install-tasks HD_INSTALL_TASKS=${CFITSIO_DIR}/fpack
	${MAKE} install-tasks HD_INSTALL_TASKS=${CFITSIO_DIR}/funpack
#-------------------------------------------------------------------------------

# Specializations for CCfits
#-------------------------------------------------------------------------------

subdir-CCfits:
	@if [ "x${EXT}" = xwin ]; then \
		${HD_MAKE} hd-std-subdir HD_SUBDIR=CCfits HD_MAKE="${HD_MAKE} -f ${HD_BUILD_DIR}/Makefile-CCfits" HD_LIB_STYLE=static; \
	else \
		${HD_MAKE} hd-std-subdir HD_SUBDIR=CCfits HD_MAKE="${HD_MAKE} -f ${HD_BUILD_DIR}/Makefile-CCfits"; \
	fi

#-------------------------------------------------------------------------------

# Specializations for readline
#-------------------------------------------------------------------------------
subdir-readline:
	@cd ${READLINE_DIR}; ${MAKE} ${HD_TARGET}

all-readline default-readline: build-readline publish-readline

build-readline:
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		if [ "x${SHLIB_SUFFIX}" = x.dll ]; then \
			${MAKE} readline-dll READLINE_NAME=readline; \
		else \
			${MAKE} subdir-readline HD_TARGET=shared; \
		fi; \
	else \
		${MAKE} subdir-readline HD_TARGET=static; \
	fi

readline-dll:
	${MAKE} subdir-readline HD_TARGET=lib${READLINE_NAME}.a
	@if [ -f ${READLINE_DIR}/lib${READLINE_NAME}.a ]; then \
		rm -rf ./$$.tmp; mkdir ./$$.tmp; cd ./$$.tmp; \
		echo "${AR} x ${READLINE_DIR}/lib${READLINE_NAME}.a; rm -f _*"; \
		${AR} x ${READLINE_DIR}/lib${READLINE_NAME}.a; rm -f _*; \
		echo "${SHLIB_LD} -o ${READLINE_DIR}/lib${READLINE_NAME}${SHLIB_SUFFIX} * ${SYSLIBS}"; \
		${SHLIB_LD} -o ${READLINE_DIR}/lib${READLINE_NAME}${SHLIB_SUFFIX} * ${SYSLIBS}; \
	fi
	rm -rf ./$$.tmp
	
publish-readline:
	@${MAKE} install-readline HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_CP_P="${HD_LN_S}"

install-readline:
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		${MAKE} subdir-readline \
			HD_TARGET="installdirs install-headers" \
			prefix=${HD_EXEC_PFX} exec_prefix=${HD_EXEC_PFX} \
			libdir=${HD_LIB}; \
		if [ "x${SHLIB_SUFFIX}" != x.dll ]; then \
			${MAKE} subdir-readline \
			    READLINE_DIR="${READLINE_DIR}/shlib" \
			    HD_TARGET="install" \
			    prefix=${HD_EXEC_PFX} exec_prefix=${HD_EXEC_PFX} \
				libdir=${HD_LIB}; \
			${HD_INSTALL} ${HD_LIB}/libreadline${SHLIB_SUFFIX} \
			  ${HD_LIB}/lib${READLINE}${SHLIB_SUFFIX} ${HD_LN_S}; \
		else \
			${HD_INSTALL} ${READLINE_DIR}/libreadline${SHLIB_SUFFIX} \
			  ${HD_LIB}/libreadline${SHLIB_SUFFIX} ${HD_CP_P}; \
			${HD_INSTALL} ${HD_LIB}/libreadline${SHLIB_SUFFIX} \
			  ${HD_LIB}/lib${READLINE}${SHLIB_SUFFIX} ${HD_LN_S}; \
		fi; \
	else \
		${MAKE} subdir-readline \
			HD_TARGET="installdirs install-headers" \
			prefix=${HD_EXEC_PFX} exec_prefix=${HD_EXEC_PFX} \
			libdir=${HD_LIB}; \
		${HD_INSTALL} ${READLINE_DIR}/libreadline.a \
			${HD_LIB}/lib${READLINE}.a ${HD_CP_P}; \
		${HD_INSTALL} ${HD_LIB}/lib${READLINE}.a \
			${HD_LIB}/libreadline.a ${HD_LN_S}; \
	fi

clean-readline:
	@${MAKE} subdir-readline HD_TARGET=clean
	@rm -f ${READLINE_DIR}/libreadline${SHLIB_SUFFIX} \
		${READLINE_DIR}/libhistory${SHLIB_SUFFIX}

distclean-readline:
	@${MAKE} subdir-readline HD_TARGET=distclean
	@rm -rf ${READLINE_DIR}/a.out.dSYM \
		${READLINE_DIR}/configure.lineno \
		${READLINE_DIR}/shlib/so_locations \
		${READLINE_DIR}/libreadline${SHLIB_SUFFIX} \
		${READLINE_DIR}/libhistory${SHLIB_SUFFIX}

#-------------------------------------------------------------------------------

# Specializations for pil
#-------------------------------------------------------------------------------
subdir-pil:
	@cd ${PIL_DIR}; ${MAKE} -f Makefile.HEAdas ${HD_TARGET} PIL_EXTRA_CFLAGS="${PIL_EXTRA_CFLAGS}"
	@if [ "x${HD_TARGET}" = xclean -o "x${HD_TARGET}" = xdistclean ]; then \
	      echo "rm -f ${PIL_DIR}/lib${PIL}${SHLIB_SUFFIX}"; \
	      rm -f ${PIL_DIR}/lib${PIL}${SHLIB_SUFFIX}; \
	fi

all-pil default-pil: build-pil publish-pil

build-pil:
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		if [ "x${SHLIB_SUFFIX}" = x.dll ]; then \
		      ${MAKE} subdir-pil HD_TARGET=shared \
			LINK_LIBS="-L${HD_BLD_LIB} -l${READLINE} ${SYSLIBS}" \
			ISDC_ENV=${HD_BLD_EXEC_PFX}; \
		else \
		      ${MAKE} subdir-pil HD_TARGET=shared \
			LINK_LIBS= \
			ISDC_ENV=${HD_BLD_EXEC_PFX}; \
		fi; \
	else \
		${MAKE} subdir-pil HD_TARGET="" ISDC_ENV=${HD_BLD_EXEC_PFX}; \
	fi

publish-pil:
	@${MAKE} install-pil HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_CP_P="${HD_LN_S}"

install-pil:
	@${MAKE} subdir-pil HD_TARGET=global_install \
		ISDC_ENV=${HD_EXEC_PFX} TO_INSTALL_LIB=""
	@-rmdir ${HD_BLD_EXEC_PFX}/pfiles
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		${HD_INSTALL} ${PIL_DIR}/lib${PIL}${SHLIB_SUFFIX} \
			${HD_LIB}/lib${PIL}${SHLIB_SUFFIX} ${HD_CP_P}; \
	else \
		${HD_INSTALL} ${PIL_DIR}/libpil.a \
			${HD_LIB}/lib${PIL}.a ${HD_CP_P}; \
	fi
#-------------------------------------------------------------------------------

# Specializations for fitsperl
#-------------------------------------------------------------------------------
subdir-fitsperl:
	cd ${FITSPERL_DIR}; ${MAKE} ${HD_TARGET}

makemake-fitsperl: ${FITSPERL_DIR}/Makefile

${FITSPERL_DIR}/Makefile: ${FITSPERL_DIR}/Makefile.PL
	cd ${FITSPERL_DIR}; ${PERL} Makefile.PL OPTIMIZE=-O

all-fitsperl default-fitsperl:
	${HD_MAKE} makemake-fitsperl build-fitsperl publish-fitsperl

build-fitsperl:
	@${MAKE} subdir-fitsperl HD_TARGET=

publish-fitsperl:
	@${MAKE} install-fitsperl HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX}

install-fitsperl:
	@${HD_INSTALL} ${FITSPERL_DIR}/blib/arch/auto/Astro/FITS/CFITSIO/CFITSIO${DYNPERL_SUFFIX} ${HD_PERL_LIB}/auto/Astro/FITS/CFITSIO/CFITSIO${DYNPERL_SUFFIX} ${HD_CP_P}
	@${HD_INSTALL} ${FITSPERL_DIR}/blib/lib/Astro/FITS/CFITSIO.pm  ${HD_PERL_LIB}/Astro/FITS/CFITSIO.pm ${HD_CP_P}

# If a Perl module distclean is run in an environment different from the
# build, it may fail, leaving files behind which can break new builds:
distclean-fitsperl: distclean-fitsperl-native
	@rm -rf ${FITSPERL_DIR}/Makefile ${FITSPERL_DIR}/*.o \
		${FITSPERL_DIR}/CFITSIO.c ${FITSPERL_DIR}/CFITSIO.bs \
		${FITSPERL_DIR}/blib ${FITSPERL_DIR}/pm_to_blib

distclean-fitsperl-native:
	@-${MAKE} subdir-fitsperl HD_TARGET=distclean
#-------------------------------------------------------------------------------

# Specializations for pilperl
#-------------------------------------------------------------------------------
subdir-pilperl:
	cd ${PILPERL_DIR}; ${MAKE} ${HD_TARGET}

makemake-pilperl: ${PILPERL_DIR}/Makefile

${PILPERL_DIR}/Makefile: ${PILPERL_DIR}/Makefile.PL
	cd ${PILPERL_DIR}; ${PERL} Makefile.PL OPTIMIZE=-O

all-pilperl default-pilperl:
	${HD_MAKE} makemake-pilperl build-pilperl publish-pilperl

build-pilperl:
	@${MAKE} subdir-pilperl HD_TARGET=

publish-pilperl:
	@${MAKE} install-pilperl HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX}

install-pilperl:
	@${HD_INSTALL} ${PILPERL_DIR}/blib/arch/auto/HEACORE/PIL/PIL${DYNPERL_SUFFIX} ${HD_PERL_LIB}/auto/HEACORE/PIL/PIL${DYNPERL_SUFFIX} ${HD_CP_P}
	@${HD_INSTALL} ${PILPERL_DIR}/blib/lib/HEACORE/PIL.pm  ${HD_PERL_LIB}/HEACORE/PIL.pm ${HD_CP_P}

# If a Perl module distclean is run in an environment different from the
# build, it may fail, leaving files behind which can break new builds:
distclean-pilperl: distclean-pilperl-native
	@rm -rf ${PILPERL_DIR}/Makefile ${PILPERL_DIR}/*.o \
		${PILPERL_DIR}/PIL.c ${PILPERL_DIR}/PIL.bs \
		${PILPERL_DIR}/blib ${PILPERL_DIR}/pm_to_blib

distclean-pilperl-native:
	@-${MAKE} subdir-pilperl HD_TARGET=distclean
#-------------------------------------------------------------------------------

# Specializations for hdutilsperl
#-------------------------------------------------------------------------------
subdir-hdutilsperl:
	cd ${HDUTILSPERL_DIR}; ${MAKE} ${HD_TARGET}

makemake-hdutilsperl: ${HDUTILSPERL_DIR}/Makefile

${HDUTILSPERL_DIR}/Makefile: ${HDUTILSPERL_DIR}/Makefile.PL
	cd ${HDUTILSPERL_DIR}; ${PERL} Makefile.PL OPTIMIZE=-O

all-hdutilsperl default-hdutilsperl:
	${HD_MAKE} makemake-hdutilsperl build-hdutilsperl publish-hdutilsperl

build-hdutilsperl:
	@${MAKE} subdir-hdutilsperl HD_TARGET=

publish-hdutilsperl:
	@${MAKE} install-hdutilsperl HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX}

install-hdutilsperl:
	@${HD_INSTALL} ${HDUTILSPERL_DIR}/blib/arch/auto/HEACORE/HEAUTILS/HEAUTILS${DYNPERL_SUFFIX} ${HD_PERL_LIB}/auto/HEACORE/HEAUTILS/HEAUTILS${DYNPERL_SUFFIX} ${HD_CP_P}
	@${HD_INSTALL} ${HDUTILSPERL_DIR}/blib/lib/HEACORE/HEAUTILS.pm  ${HD_PERL_LIB}/HEACORE/HEAUTILS.pm ${HD_CP_P}

# If a Perl module distclean is run in an environment different from the
# build, it may fail, leaving files behind which can break new builds:
distclean-hdutilsperl: distclean-hdutilsperl-native
	@rm -rf ${HDUTILSPERL_DIR}/Makefile ${HDUTILSPERL_DIR}/*.o \
		${HDUTILSPERL_DIR}/HEAUTILS.c ${HDUTILSPERL_DIR}/HEAUTILS.bs \
		${HDUTILSPERL_DIR}/blib ${HDUTILSPERL_DIR}/pm_to_blib

distclean-hdutilsperl-native:
	@-${MAKE} subdir-hdutilsperl HD_TARGET=distclean
#-------------------------------------------------------------------------------

# Specializations for hdinitperl
#-------------------------------------------------------------------------------
subdir-hdinitperl:
	cd ${HDINITPERL_DIR}; ${MAKE} ${HD_TARGET}

makemake-hdinitperl: ${HDINITPERL_DIR}/Makefile

${HDINITPERL_DIR}/Makefile: ${HDINITPERL_DIR}/Makefile.PL
	cd ${HDINITPERL_DIR}; ${PERL} Makefile.PL OPTIMIZE=-O

all-hdinitperl default-hdinitperl:
	${HD_MAKE} makemake-hdinitperl build-hdinitperl publish-hdinitperl

build-hdinitperl:
	@${MAKE} subdir-hdinitperl HD_TARGET=

publish-hdinitperl:
	@${MAKE} install-hdinitperl HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX}

install-hdinitperl:
	@${HD_INSTALL} ${HDINITPERL_DIR}/blib/arch/auto/HEACORE/HEAINIT/HEAINIT${DYNPERL_SUFFIX} ${HD_PERL_LIB}/auto/HEACORE/HEAINIT/HEAINIT${DYNPERL_SUFFIX} ${HD_CP_P}
	@${HD_INSTALL} ${HDINITPERL_DIR}/blib/lib/HEACORE/HEAINIT.pm  ${HD_PERL_LIB}/HEACORE/HEAINIT.pm ${HD_CP_P}

# If a Perl module distclean is run in an environment different from the
# build, it may fail, leaving files behind which can break new builds:
distclean-hdinitperl: distclean-hdinitperl-native
	@rm -rf ${HDINITPERL_DIR}/Makefile ${HDINITPERL_DIR}/*.o \
		${HDINITPERL_DIR}/HEAINIT.c ${HDINITPERL_DIR}/HEAINIT.bs \
		${HDINITPERL_DIR}/blib ${HDINITPERL_DIR}/pm_to_blib

distclean-hdinitperl-native:
	@-${MAKE} subdir-hdinitperl HD_TARGET=distclean
#-------------------------------------------------------------------------------

# Specializations for ast 
#-------------------------------------------------------------------------------

subdir-ast:
	@${HD_MAKE} hd-std-subdir HD_SUBDIR=ast HD_MAKE="${HD_MAKE} -f Makefile.HEAdas"
	@if [ "x${HD_TARGET}" = xdistclean ]; then \
		rm -rf ${AST_DIR}/configure.lineno ${AST_DIR}/a.out.dSYM; \
	fi
#-------------------------------------------------------------------------------

# Specializations for wcslib
#-------------------------------------------------------------------------------
subdir-wcslib:
	@if [ "x${HD_TARGET}" = xclean ]; then \
		cd ${WCSLIB_DIR}/C; ${MAKE} ${HD_TARGET}; \
	else \
		cd ${WCSLIB_DIR}; ${MAKE} ${HD_TARGET} CC="${CC}" FC="${FC}" \
			FLAVOUR=${WCSFLAV} WCSTRIG=WRAPPER FLEX=""; \
	fi

WCSLIB_INSTALL_HEADERS	= wcsconfig.h C/cel.h C/fitshdr.h C/getwcstab.h \
			  C/lin.h C/log.h C/prj.h C/spc.h C/sph.h C/spx.h \
			  C/tab.h C/wcs.h C/wcsfix.h C/wcshdr.h C/wcslib.h \
			  C/wcsmath.h C/wcstrig.h C/wcsunits.h C/wcsutil.h

all-wcslib default-wcslib: build-wcslib publish-wcslib

build-wcslib:
	@if [ "x${HD_LIB_STYLE}" = xshared ]; then \
		if [ "x${EXT}" = xsol ]; then \
			${MAKE} subdir-wcslib \
				SHRLIB="lib${WCSLIB}${SHLIB_SUFFIX}" \
				SHRLD="${SHLIB_LD}" SHRFLAGS="${CFLAGS}"; \
		else \
			${MAKE} subdir-wcslib \
				SHRLIB="lib${WCSLIB}${SHLIB_SUFFIX}"; \
		fi; \
	else \
		${MAKE} subdir-wcslib LIBWCS="lib${WCSLIB}.a"; \
	fi

clean-wcslib:
	@${MAKE} build-wcslib HD_TARGET=clean

distclean-wcslib:
	@${MAKE} build-wcslib HD_TARGET=realclean
	@rm -rf ${WCSLIB_DIR}/a.out.dSYM

publish-wcslib:
	@${MAKE} install-wcslib HD_TOP_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_EXEC_PFX=${HD_BLD_EXEC_PFX} HD_CP_P="${HD_LN_S}"

install-wcslib:
	@for f in ${WCSLIB_INSTALL_HEADERS}; do \
	  hd_install_headers="$$hd_install_headers ${WCSLIB_DIR}/$$f"; \
	done; \
	${MAKE} install-headers HD_INSTALL_HEADERS="$$hd_install_headers" \
		HD_INC_SUBDIR="wcslib"
	${MAKE} install-libraries HD_INSTALL_LIBRARIES=${WCSLIB_DIR}/C/${WCSLIB}
