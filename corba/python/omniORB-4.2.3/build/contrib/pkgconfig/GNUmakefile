TOP=../..
CURRENT=contrib/pkgconfig
BASE_OMNI_TREE=../../..
VPATH=../../../contrib/pkgconfig
INSTALL=/usr/bin/install -c

INSTALLDIR = $(INSTALLLIBDIR)/pkgconfig

PKGFILES = omnithread3.pc omniORB4.pc omniDynamic4.pc \
           omniCOS4.pc omniCOSDynamic4.pc omniConnectionMgmt4.pc \
           omniZIOP4.pc omniZIOPDynamic4.pc

include $(TOP)/mk/beforeauto.mk

install:: $(PKGFILES)
	@(dir="$(INSTALLDIR)"; \
          for file in $^; do \
            $(ExportFileToDir) \
          done )

include $(TOP)/mk/afterauto.mk
