SUBDIRS = twitter

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@EVAS_CFLAGS@			\
		@ELEMENTARY_CFLAGS@ 		\
		@MOKOSUITE_CFLAGS@		\
		@GLIB_CFLAGS@			\
		@REST_CFLAGS@			\
		-DDATADIR=\""$(datadir)"\" 	\
		-DLOCALEDIR=\"$(localedir)\" 	\
		-Wall

AM_LDFLAGS = \
	@EVAS_LIBS@		\
	@ELEMENTARY_LIBS@	\
	@GLIB_LIBS@		\
	@MOKOSUITE_LIBS@

bin_PROGRAMS = micromoko

micromoko_SOURCES =	\
	globals.h	\
	main.c		\
	timeline.c	\
	timeline.h	\
	auth.c		\
	auth.h

micromoko_LDADD = -Ltwitter -ltwitter
