
SRCS = sched.cpp process.cpp event.cpp scheduler.cpp utils.cpp
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -Iinclude
CXXFLAGS = -g $(INCLUDES)

sched: $(OBJS)
	g++ $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -f sched *~ *.out