MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@ELEMENTARY_CFLAGS@ 		\
		@MOKOSUITE_CFLAGS@		\
		@GLIB_CFLAGS@			\
		-DDATADIR=\""$(datadir)"\" 	\
		-DLOCALEDIR=\"$(localedir)\" 	\
		-Wall

AM_LDFLAGS = \
	@ELEMENTARY_LIBS@	\
	@GLIB_LIBS@		\
	@MOKOSUITE_LIBS@


bin_PROGRAMS = microtwitter

microtwitter_SOURCES =	\
	globals.h	\
	main.c		\
	timeline.c	\
	timeline.h
