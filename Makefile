ACXXFLAGS = 
CXXFLAGS = $(ACXXFLAGS) -std=c++23 -lsqlite3 -Iinclude

SRCS = main.cpp dbManager.cpp event.cpp action.cpp daemon.cpp threadPool.cpp
OBJS = $(addprefix obj/, $(SRCS:.cpp=.o))

all: main

main: $(OBJS) obj
	g++ $(CXXFLAGS) $(OBJS) -o main -lsqlite3
	chmod +x main

obj:
	mkdir -p obj

obj/%.o: src/%.cpp include/%.h | obj
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj
