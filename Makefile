MODE?=DEBUG

CC=clang++-19
CXX=$(CC)
CLD=lld-19
CXXVERSION=-std=c++23
BASECXXFLAGS=${CXXVERSION} -pthread -pedantic -I util -DELUCIDATE_CXX=${CXX} -DELUCIDATE_CLD=${CLD} -DCXXVERSION=${CXXVERSION} ${BOOST_INCLUDE} 
BASEDEBUGFLAGS=-ggdb -O0 -gsplit-dwarf -Werror -Wall -Wextra -Wformat=2 -Wconversion -Wimplicit-fallthrough -fPIE
BASEOPTFLAGS=-g0 -O3 -DNDEBUG -march=native
BASELINKFLAGS=${BOOST_LINK} -fno-rtti -fuse-ld=${CLD}

ifeq ($(MODE), DEBUG)
CXXFLAGS=$(BASEDEBUGFLAGS) $(BASECXXFLAGS) -fsanitize=undefined -Weffc++ -D_GLIBCXX_ASSERTIONS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fstrict-flex-arrays=3 \
  -fstack-clash-protection -fstack-protector-strong -fcf-protection=full
LINKOPTS=$(BASELINKFLAGS) -fsanitize=undefined -rdynamic -pie -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
else
CXXFLAGS=$(BASEOPTFLAGS) $(BASECXXFLAGS) -flto
LINKOPTS=$(BASELINKFLAGS) -O3 -s -flto -Wl,-z,nodlopen -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
endif

BENCHMARK_RT=bin/$(MODE)/benchmark-rt

BINS=${BENCHMARK_RT}
all: ${BINS}

OBJECTFILE = bin/$(MODE)/$(patsubst %.cpp,%.o,$(notdir $1))
TARGET = $(eval ALL_OBJS += $(call OBJECTFILE,$1)) $(call OBJECTFILE, $1): $1 $2
RULES = mkdir -p $(@D); ${CXX} ${CXXFLAGS} $1 -c -o $@ $< -MMD
define ADD_OBJECT
$$(call TARGET,$1)
	$$(call RULES,$2)
$3+= $$(call OBJECTFILE,$1)
$(eval ALL_CUS+= $1)
endef

$(eval $(call ADD_OBJECT,Process.cpp,,SYS_OBJS))
$(eval $(call ADD_OBJECT,Thread.cpp,,SYS_OBJS))
$(eval $(call ADD_OBJECT,Signal.cpp,,SYS_OBJS))
$(eval $(call ADD_OBJECT,BackTrace.cpp,,UTIL_OBJS))
$(eval $(call ADD_OBJECT,TracedTerminate.cpp,,UTIL_OBJS))

${BENCHMARK_RT}: ${DATAFLOW_OBJS} ${RT_OBJS} ${RT_HOST_OBJS} ${UTIL_OBJS} ${SYS_OBJS} $(call OBJECTFILE,benchmark-rt.cpp)
	${CC} ${LINKOPTS} $^ -o $@ \
	-L ~/local/HdrHistogram_c/lib \
	-lboost_program_options -lglog -lstdc++ -lstdc++fs -lhdr_histogram_static -lm -ldl -latomic -lpthread
$(call TARGET,benchmark-rt.cpp)
	$(call RULES,-isystem ~/local/HdrHistogram_c/include)

DEP=$(ALL_OBJS:.o=.d)
-include ${DEP}
DWARF=$(ALL_OBJS:.o=.dwo)

clean:
	rm -f ${BINS} ${ALL_OBJS} ${DEP} ${DWARF}

