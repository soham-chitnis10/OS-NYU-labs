
SRCS = mmu.cpp process.cpp frame.cpp pager.cpp utils.cpp
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -Iinclude
CXXFLAGS = -g $(INCLUDES)

mmu: $(OBJS)
	g++ $(CXXFLAGS) -o mmu $(OBJS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f mmu *.o *~ *.out