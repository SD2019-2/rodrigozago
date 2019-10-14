#!/usr/bin/env python
"""Python language binding for the ORBit2 CORBA implementation.

PyORBit aims to take advantage of new features found in ORBit2 to make
language bindings more efficient.  This includes:
  - Use of ORBit2 type libraries to generate stubs
  - use of the ORBit_small_invoke_stub() call for operation
    invocation, which allows for short circuited invocation on local
    objects.
"""

from commands import getoutput
from distutils.core import setup
from distutils.extension import Extension
import os

from dsextras import have_pkgconfig, GLOBAL_MACROS
from dsextras import InstallLib, PkgConfigExtension

MAJOR_VERSION = 2
MINOR_VERSION = 0
MICRO_VERSION = 0

VERSION = "%d.%d.%d" % (MAJOR_VERSION,
                        MINOR_VERSION,
                        MICRO_VERSION)

ORBIT2_REQUIRED  = '2.4.4'

GLOBAL_MACROS.append(('ORBIT2_STUBS_API', 1))

class PyORBitInstallLib(InstallLib):
    def run(self):
        self.add_template_option('ORBIT2_REQUIRED_VERSION', ORBIT2_REQUIRED)
        self.prepare()

        self.install_template('pyorbit-2.pc.in',
                              os.path.join(self.libdir, 'pkgconfig'))
        InstallLib.run(self)
        
orbit = PkgConfigExtension(name='ORBit',
                     pkc_name='ORBit-2.0',
                     pkc_version=ORBIT2_REQUIRED,
                     sources=['src/ORBitmodule.c',
                              'src/pycorba-typecode.c',
                              'src/pycorba-object.c',
                              'src/pycorba-method.c',
                              'src/pycorba-marshal.c',
                              'src/pycorba-orb.c',
                              'src/pycorba-any.c',
                              'src/pycorba-exceptions.c',
                              'src/pycorba-struct.c',
                              'src/pycorba-enum.c',
                              'src/pycorba-fixed.c',
                              'src/stub-gen.c',
                              'src/pyorbit-servant.c',
                              'src/pyorbit-poa.c',
                              'src/pyorbit-utils.c'])

if not orbit.can_build():
    raise SystemExit

doclines = __doc__.split("\n")

setup(name="pyorbit",
      version=VERSION,
      license='LGPL',
      platforms=['yes'],
      maintainer="James Henstridge",
      maintainer_email="james@daa.com.au",
      description = doclines[0],
      long_description = "\n".join(doclines[2:]),
      py_modules=['CORBA', 'PortableServer'],
      ext_modules=[orbit],
      data_files=[('include/pyorbit-2.0', ['src/pyorbit.h'])],
      cmdclass={'install_lib': PyORBitInstallLib})
