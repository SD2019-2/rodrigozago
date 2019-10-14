
INCLUDES = $(PYTHON_INCLUDES) $(PYORBIT_CFLAGS) -DORBIT2_STUBS_API

python_PYTHON = CORBA.py PortableServer.py

pyexec_LTLIBRARIES = ORBit.la

common_ldflags = -module -avoid-version

ORBit_la_LDFLAGS = $(common_ldflags) -export-symbols-regex initORBit
ORBit_la_LIBADD = $(PYORBIT_LIBS)
ORBit_la_SOURCES = \
  ORBitmodule.c \
  pycorba-typecode.c \
  pycorba-object.c \
  pycorba-method.c \
  pycorba-marshal.c \
  pycorba-orb.c \
  pycorba-any.c \
  pycorba-exceptions.c \
  pycorba-struct.c \
  pycorba-enum.c \
  pycorba-fixed.c \
  stub-gen.c \
  pyorbit-servant.c \
  pyorbit-poa.c \
  pyorbit-utils.c \
  pyorbit-private.h

pyorbitincludedir = $(includedir)/pyorbit-2

pyorbitinclude_HEADERS = pyorbit.h
