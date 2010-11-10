MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@MOKOSUITE_CFLAGS@	\
		@GLIB_CFLAGS@		\
		@REST_CFLAGS@		\
		-Wall

AM_LDFLAGS = \
	@GLIB_LIBS@		\
	@MOKOSUITE_LIBS@	\
	@REST_LIBS@

noinst_LTLIBRARIES = libtwitter.la

libtwitter_la_SOURCES =	\
	private.h	\
	twitter.h	\
	twitter.c
