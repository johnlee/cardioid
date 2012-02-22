#------------------------------------------------------------------------

CXX=mpic++ -fopenmp
CC =mpicc --std=gnu99
LD=$(CXX)

DFLAGS = -DOSX -DWITH_PIO -DWITH_MPI \
	-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

INCLUDE = -I/opt/local/include/

CFLAGS_BASE =   $(INCLUDE) $(DFLAGS)
CXXFLAGS_BASE = $(INCLUDE) $(DFLAGS)

HAVE_GSL = 1
ifeq ($(HAVE_GSL),1) 
   CFLAGS_BASE  += -DHAVE_GSL
   CXXFLAGS_BASE  += -DHAVE_GSL
   LDFLAGS_BASE += -L/opt/local/lib -lgsl -lgslcblas
endif


CFLAGS_OPT =   $(CFLAGS_BASE) -g -O3
CFLAGS_DEBUG = $(CFLAGS_BASE) -g -ggdb -O0 -fno-inline
CFLAGS_PROF =  $(CFLAGS_BASE) -g -pg -O3 -DPROFILE

CXXFLAGS_OPT =   $(CXXFLAGS_BASE) -g -O3
CXXFLAGS_DEBUG = $(CXXFLAGS_BASE) -g -ggdb -O0 -fno-inline
CXXFLAGS_PROF =  $(CXXFLAGS_BASE) -g -pg -O3 -DPROFILE

LDFLAGS_OPT   = $(LDFLAGS_BASE) $(CFLAGS_OPT) $(CXXFLAGS_OPT)
LDFLAGS_DEBUG = $(LDFLAGS_BASE) $(CFLAGS_DEBUG) $(CXXFLAGS_DEBUG)
LDFLAGS_PROF  = $(LDFLAGS_BASE) $(CFLAGS_PROF) $(CXXFLAGS_PROF)
