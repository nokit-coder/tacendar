CXXFLAGS = 

SRCS = main.cpp dbManager.cpp event.cpp action.cpp daemon.cpp
OBJS = $(addprefix obj/, $(SRCS:.cpp=.o))

all: main daemon

main: $(OBJS) | obj
	g++ $(CXXFLAGS) $(OBJS) -o main -lsqlite3
	chmod +x main

daemon:

obj:
	mkdir -p obj

obj/%.o: src/%.cpp include/%.h | obj
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj
