CXXFLAGS = 

SRCS = main.cpp dbManager.cpp event.cpp task.cpp
OBJS = $(SRCS:.cpp=.o)

all: main daemon

main: obj/main.o obj/dbManager.o obj/event.o obj/task.o
	g++ $(CXXFLAGS) $(addprefix obj/ $(OBJS)) -o main -lsqlite3
	chmod +x main

daemon:

obj:
	mkdir -p obj

obj/%.o: src/%.cpp | obj

# obj/main.o: main.cpp event.h dbManager.h | obj
# 	g++ $(CXXFLAGS) -c main.cpp -o obj/main.o
#
# obj/dbManager.o: dbManager.cpp | obj
# 	g++ $(CXXFLAGS) -c dbManager.cpp -o obj/dbManager.o
#
# obj/event.o: event.cpp | obj
# 	g++ $(CXXFLAGS) -c event.cpp -o obj/event.o
#
# obj/task.o: task.cpp | obj
# 	g++ $(CXXFLAGS) -c task.cpp -o obj/task.o

clean:
	rm -rf obj
