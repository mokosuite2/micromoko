SUBDIRS = oauth

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@MOKOSUITE_CFLAGS@	\
		@GLIB_CFLAGS@		\
		@SOUP_CFLAGS@		\
		-Wall

AM_LDFLAGS = \
	@GLIB_LIBS@		\
	@MOKOSUITE_LIBS@	\
	@SOUP_LIBS@		\
	-lreadline
# !!!TEST TEST TEST!!!

noinst_LTLIBRARIES = libtwitter.la

libtwitter_la_SOURCES =	\
	private.h	\
	twitter.h	\
	twitter.c

libtwitter_la_LIBADD = -Loauth -loauth
