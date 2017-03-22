default: server client

server:  echoserveri.o echo.o csapp.o
	gcc $^ -o $@ -pthread
client: echoclient.o echo.o csapp.o
	gcc $^ -o $@ -pthread

clean:
	rm *.o server client