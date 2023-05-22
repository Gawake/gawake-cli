CFLAGS = -Wall -lsqlite3

all: main.o db_checker.o cron_toff.o cron_ton.o
	gcc $(CFLAGS) main.o -o gawake-cli
	gcc $(CFLAGS) db_checker.o -o db_checker
	gcc $(CFLAGS) cron_toff.o -o cron_toff
	gcc $(CFLAGS) cron_ton.o -o cron_ton

main.o: db_checker.o
	gcc -Wall -c main.c
	
db_checker.o:
	gcc -Wall -c db_checker.c
	
cron_toff.o: 
	gcc -Wall -c cron_toff.c
	
cron_ton.o: 
	gcc -Wall -c cron_ton.c
	
clean:
	rm *.o

