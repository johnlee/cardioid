#-------------------------------------------------------------------------------

CXX=mpixlcxx_r
CC=mpixlc_r
LD=$(CXX)

DFLAGS = -DWITH_PIO -DWITH_MPI -DBGP \
	-DADD_ -DUSE_CSTDIO_LFS -DMPICH_IGNORE_CXX_SEEK

INCLUDE =  -I/bgsys/drivers/ppcfloor/arch/include \
	-I/usr/local/tools/gsl/include/


CFLAGS_BASE =   -qarch=450d $(INCLUDE) $(DFLAGS)
CXXFLAGS_BASE = -qarch=450d $(INCLUDE) $(DFLAGS)
#ewd DEBUG: add floating-point exception traps
#CFLAGS_BASE =   -qarch=450 $(INCLUDE) $(DFLAGS) -qflttrap=enable:nanq:overflow:zerodivide
#CXXFLAGS_BASE = -qarch=450 $(INCLUDE) $(DFLAGS) -qflttrap=enable:nanq:overflow:zerodivide
OMP_FLAGS = -qsmp=omp 

HAVE_GSL = 1
ifeq ($(HAVE_GSL),1) 
   CFLAGS_BASE  += -DHAVE_GSL
   CXXFLAGS_BASE  += -DHAVE_GSL
   LDFLAGS_BASE += -L/usr/local/tools/gsl/lib -lgsl -lgslcblas
endif

CFLAGS_OPT =   $(CFLAGS_BASE) $(OMP_FLAGS) -g -O3 -qtune=450 
CFLAGS_DEBUG = $(CFLAGS_BASE) $(OMP_FLAGS) -g -O0
CFLAGS_PROF =  $(CFLAGS_BASE) $(OMP_FLAGS) -g -pg -O3 -DPROFILE
CFLAGS_MPIP =  $(CFLAGS_BASE) -g -O3 -qtune=450 

CXXFLAGS_OPT =   $(CXXFLAGS_BASE) $(OMP_FLAGS) -g -O3 -qtune=450 
CXXFLAGS_DEBUG = $(CXXFLAGS_BASE) $(OMP_FLAGS) -g -O0
CXXFLAGS_PROF =  $(CXXFLAGS_BASE) $(OMP_FLAGS) -g -pg -O3 -DPROFILE
CXXFLAGS_MPIP =  $(CXXFLAGS_BASE) -g -O3 -qtune=450 

LDFLAGS_OPT   = $(LDFLAGS_BASE) $(CFLAGS_OPT) $(CXXFLAGS_OPT)
LDFLAGS_DEBUG = $(LDFLAGS_BASE) $(CFLAGS_DEBUG) $(CXXFLAGS_DEBUG)
LDFLAGS_PROF  = $(LDFLAGS_BASE) $(CFLAGS_PROF) $(CXXFLAGS_PROF)

MPIP_DIR = /usr/local/tools/mpiP/lib
LDFLAGS_MPIP   =  -L$(MPIP_DIR) -lmpiP -lm $(LDFLAGS_BASE) $(CFLAGS_MPIP) $(CXXFLAGS_MPIP)
