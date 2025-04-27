# For GNU conventions and targets see https://www.gnu.org/prep/standards/standards.html
# Using GNU standards makes it easier for some users to keep doing what they are used to.

# 'mkdir -p' is non-portable, but it is widely supported. A portable solution
# is elusive due to race conditions on testing the directory and creating it.
# Anemic toolchain users can sidestep the problem using MKDIR="mkdir".

AR = ar
ARFLAGS = cr
RM = rm -f
RANLIB = ranlib
MKDIR = mkdir -p
CXXFLAGS = -D_FILE_OFFSET_BITS=64 -fPIC
CXXFLAGS_EFFC = -Werror -Wall -Wextra -Wshadow -Wpedantic -Wformat-nonliteral \
	-Wformat-security -Wswitch-default -Wuninitialized -Wundef \
	-Wpointer-arith -Woverloaded-virtual -Wctor-dtor-privacy \
	-Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo \
	-Wno-unused-parameter -Weffc++

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

prefix = /usr/local
bindir = $(prefix)/bin
libdir = $(prefix)/lib
includedir = $(prefix)/include

all: xmltest xmltest2 staticlib

rebuild: clean all

effc:
	gcc $(CXXFLAGS_EFFC) xmltest.cpp tinyxml2.cpp -o xmltest

effc2:
	gcc $(CXXFLAGS_EFFC) xmltest2.cpp -o xmltest2

# ----------------- Build Original xmltest -----------------

xmltest: xmltest.cpp libtinyxml2.a
	$(CXX) $(CXXFLAGS) xmltest.cpp tinyxml2.cpp -o xmltest

# ----------------- Build New xmltest2 -----------------

xmltest2: xmltest2.cpp
	$(CXX) $(CXXFLAGS) xmltest2.cpp -o xmltest2

# ----------------- Static Library -----------------

staticlib: libtinyxml2.a

libtinyxml2.a: tinyxml2.o
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

tinyxml2.o: tinyxml2.cpp tinyxml2.h

# ----------------- Cleaning -----------------

clean:
	-$(RM) *.o xmltest xmltest2 libtinyxml2.a

# Standard GNU target
distclean:
	-$(RM) *.o xmltest xmltest2 libtinyxml2.a


# ----------------- Testing -----------------

test: xmltest
	./xmltest

test2: xmltest2
	./xmltest2

# Standard GNU targets
check: test
check2: test2

# ----------------- Installation -----------------

directories:
	$(MKDIR) $(DESTDIR)$(prefix)
	$(MKDIR) $(DESTDIR)$(bindir)
	$(MKDIR) $(DESTDIR)$(libdir)
	$(MKDIR) $(DESTDIR)$(includedir)

install: xmltest staticlib directories
	$(INSTALL_PROGRAM) xmltest $(DESTDIR)$(bindir)/xmltest
	$(INSTALL_DATA) tinyxml2.h $(DESTDIR)$(includedir)/tinyxml2.h
	$(INSTALL_DATA) libtinyxml2.a $(DESTDIR)$(libdir)/libtinyxml2.a

uninstall:
	$(RM) $(DESTDIR)$(bindir)/xmltest
	$(RM) $(DESTDIR)$(includedir)/tinyxml2.h
	$(RM) $(DESTDIR)$(libdir)/libtinyxml2.a