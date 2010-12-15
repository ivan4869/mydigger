
SRCS= $(wildcard *.cpp)
OBJS= $(SRCS:.cpp=.o)

CPP=g++
LD=g++

COMMON_FLAGS= -Wall -g -Wextra -Wno-deprecated -fopenmp 
	# -Wfloat-equal -Wbad-function-cast \
	# -Wcast-qual \
	# -Wconversion \
	# -Wsign-compare

ifdef release
CFLAGS= $(COMMON_FLAGS) -O3 -DNDEBUG
else
CFLAGS= $(COMMON_FLAGS) -O0
endif

LIBS=c pthread

LD_FLAGS= -fopenmp 

ifdef profile
CFLAGS= -pg $(COMMON_FLAGS) -O3 -DNDEBUG
LD_FLAGS= -pg
endif

OUT_NAME=bar_digger.out

#.PHONY clean


all:$(OUT_NAME)

%.o:%.cpp
	$(CPP) -c $(CFLAGS)  -o $@ $<

$(OUT_NAME):$(OBJS)
	$(LD) $(LD_FLAGS)  -o $@  $^ $(addprefix -l,$(LIBS))

clean:
	-rm *.o 2>/dev/null
	-rm $(OUT_NAME) 2>/dev/null

rebuild: clean all
