all: lib user1 user2 init

.PHONY: all lib user1 user2 init clean

lib: ksocket.c ksocket.h
	@echo "\n****Building library****"
	gcc -c -Wall ksocket.c -o ksocket.o -I. 
	ar rcs libksocket.a ksocket.o
	@echo "****Library built****\n"

user1: lib user1.c
	@echo "\n****Building user1****"
	gcc -c -Wall user1.c -o user1.o -I. 
	gcc user1.o -o user1 -L. -lksocket
	@echo "****User1 built****\n"

user2: lib user2.c
	@echo "\n****Building user2****"
	gcc -c -Wall user2.c -o user2.o -I.
	gcc user2.o -o user2 -L. -lksocket
	@echo "****User2 built****\n"

init: lib initksocket.c
	@echo "\n****Creating initksocket****"
	gcc -g -pthread initksocket.c -o initksocket -L. -lksocket -I.
	@echo "****Created****\n"

clean:
	@echo "\n****Cleaning up****"
	rm -f *.o  *.a user1 user2 initksocket clear1 ksocket socket_test user1_logs.txt user2_logs.txt init_log.txt output_*.txt
	@echo "****Cleaned****\n"
