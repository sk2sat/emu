GIT_COMMIT_ID  := $(shell git log -1 --format='%H')
GIT_COMMIT_DATE:= $(shell git log -1 --format='%ad')

CC	:= gcc
CXX	:= g++

INCFLAGS += -I./libsksat
LDFLAGS  += 

CFLAGS   += $(INCFLAGS) -MD -Wall -g
CXXFLAGS += $(INCFLAGS) -MD -std=c++14 -Wall
CXXFLAGS += -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\"" -DGIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""
ifeq ($(BUILD_TYPE),Debug)
	CXXFLAGS += -DDEBUG -g -O0
else
	CXXFLAGS += -DNO_DEBUG -O3
endif

%.o:%.c $(HEAD)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o:%.cc $(HEADXX)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

DUMMY:
.PHONE: DUMMY

_clean: DUMMY
	rm -f *.o *.a *.d

-include *.d
