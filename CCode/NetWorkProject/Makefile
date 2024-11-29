.PHONY:release
release:
	gcc ./base.c ./server.c -o server
	gcc ./base.c ./client.c -o client

.PHONY:debug
debug:
	gcc -g ./base.c ./server.c -o server
	gcc -g ./base.c ./client.c -o client

.PHONY:unit
unit:
	gcc ./base.c ./unit_test.c -o unit_test
	