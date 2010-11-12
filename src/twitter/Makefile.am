MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@MOKOSUITE_CFLAGS@	\
		@GLIB_CFLAGS@		\
		@REST_CFLAGS@		\
		-Wall

noinst_LTLIBRARIES = libtwitter.la

libtwitter_la_SOURCES =	\
	private.h	\
	twitter.h	\
	twitter.c	\
	timeline.c

libtwitter_la_LIBADD = \
	@GLIB_LIBS@		\
	@MOKOSUITE_LIBS@	\
	@REST_LIBS@

libtwitter_la_LDFLAGS = -no-undefined
