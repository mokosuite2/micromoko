DESCRIPTION = "Twitter client for SHR"
HOMEPAGE = "http://gitorious.org/mokosuite2"
AUTHOR = "Daniele Ricci"
LICENSE = "GPLv3"
DEPENDS = "elementary mokosuite-utils mokosuite-ui librest glib-2.0 edje-native"
SECTION = "misc/utils"

PV = "1.0.99+gitr${SRCPV}"
#SRCREV = "4c3a2f890987a23880c7b1809e360908f85e30fa"

SRC_URI = "git://gitorious.org/mokosuite2/micromoko.git;protocol=git"
S = "${WORKDIR}/git"

PARALLEL_MAKE = ""

EXTRA_OECONF = " --with-edje-cc=${STAGING_BINDIR_NATIVE}/edje_cc --enable-debug"
FILES_${PN} += "${datadir}/micromoko"

inherit autotools
