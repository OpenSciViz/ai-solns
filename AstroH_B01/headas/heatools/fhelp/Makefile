HD_COMPONENT_NAME	= heatools

HD_COMPONENT_VERS	=

HD_INSTALL_PERL_SCRIPTS	= fhelp

HD_INSTALL_EXTRA        = install-fhelp-sym-links

include ${HD_STD_MAKEFILE}

install-fhelp-sym-links:
	@${HD_INSTALL} ${HD_BIN}/fhelp ${HD_BIN}/fthelp ${HD_LN_S}
	@${HD_INSTALL} ${HD_BIN}/fthelp ${HD_TOP_BIN}/fthelp ${HD_LN_S}
