CFLAGS=-g
default: server client


server:  echoserverc.o echo.o csapp.o
	gcc $^ -o $@ -pthread -g
client: echoclient.o echo.o csapp.o
	gcc $^ -o $@ -pthread -g

clean:
	rm *.o server client