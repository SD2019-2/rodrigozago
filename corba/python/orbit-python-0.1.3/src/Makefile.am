INCLUDES = $(PYTHON_INCLUDES) $(PYTHON_CFLAGS) $(ORBIT_CFLAGS)
LIBS = $(ORBIT_LIBS) 

pyexec_PROGRAMS = CORBAmodule$(SO) 

CORBAmodule__SO__SOURCES = CORBAmodule.c CORBA_ORB.c CORBA_fixed.c \
	CORBA_TypeCode.c CORBA_any.c idl.c client.c marshal.c \
	PortableServer.c \
	demarshal.c server.c except.c types.c \
	CORBA_ORB.h CORBA_TypeCode.h CORBA_any.h CORBA_fixed.h \
	CORBAmodule.h types.h PortableServer.h

CORBAmodule__SO__LINK = $(PYTHON_LINK)
CORBAmodule__SO__LDADD = $(ORBIT_LIBS) $(LIBIDL_LIBS)

TESTS_ENVIRONMENT=PYTHONPATH=.
TESTS=test.sh

EXTRA_DIST=test.sh test-server test-client test-suite.idl
