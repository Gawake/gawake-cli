all: main.o db_checker.o
	gcc -Wall -lsqlite3 main.o -o gawake-cli
	gcc -Wall -lsqlite3 db_checker.o -o db_checker

main.o: db_checker.o
	gcc -Wall -c main.c
	
db_checker.o:
	gcc -Wall -c db_checker.c

clean:
	rm *.o

