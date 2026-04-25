all: obj/main.o obj/dbManager.o obj/event.o 
	g++ obj/main.o obj/dbManager.o obj/event.o -o main -lsqlite3
	chmod +x main

obj:
	mkdir -p obj

obj/main.o: main.cpp event.h dbManager.h | obj
	g++ -c main.cpp -o obj/main.o

obj/dbManager.o: dbManager.cpp | obj
	g++ -c dbManager.cpp -o obj/dbManager.o

obj/event.o: event.cpp | obj
	g++ -c event.cpp -o obj/event.o

clean:
	rm -rf obj
