# Makefile - KbdLock.dsp
# created 01/08/2005 by Roger Filipe
#
#

CC    = $(SILENT)gcc
CXX   = $(SILENT)g++
RC    = $(SILENT)windres -O COFF
LD    = $(SILENT)dllwrap
RM    = $(SILENT)-rm -f
RMDIR = $(SILENT)-rm -rf
MKDIR = $(SILENT)-mkdir
ECHO  = @echo

CFLAGS+=-W -O2 -finline-functions -march=pentiumpro -D_M_IX86=600 -DWIN32 -DNDEBUG -D_WINDOWS
LDFLAGSTARGET=kbdlock.dll
LDFLAGS+=-Wl,--subsystem,windows --implib libkbdlock.a
LIBS+=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32

ifndef TARGET
TARGET=KbdLock.dll
endif

.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.res: %.rc
	$(RC) $(CPPFLAGS) -o $@ -i $<

CFLAGS+=-DWIN32 -DNDEBUG -D_WINDOWS -O2

SRCS= \
	KbdLock.cpp \
	KbdLock.def

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) KbdLock.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > KbdLock.dep

-include KbdLock.dep

