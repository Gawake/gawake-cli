#!/bin/sh

# Documentation available by running man "gdbus-codegen"
# or at https://manpages.ubuntu.com/manpages/focal/en/man1/gdbus-codegen.1.html

# Note: the variable ${MESON_SOURCE_ROOT} access ".../Gawake/gawake-cli/"
cd "${MESON_SOURCE_ROOT}/src/gawake-dbus-server/"
gdbus-codegen 	--generate-c-code dbus-server\
		--c-namespace GawakeServer\
		--interface-prefix io.github.kelvinnovais.\
		io.github.kelvinnovais.xml