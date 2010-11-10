MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		@HASH_CFLAGS@			\
		@SOUP_CFLAGS@			\
		-DDATADIR=\""$(datadir)"\" 	\
		-DLOCALEDIR=\"$(localedir)\" 	\
		-Wall

AM_LDFLAGS = \
	@HASH_LIBS@	\
	@SOUP_LIBS@

noinst_LTLIBRARIES = liboauth.la

liboauth_la_SOURCES =	\
	oauth.c		\
	oauth.h		\
	encode.c	\
	encode.h	\
	hash.c
