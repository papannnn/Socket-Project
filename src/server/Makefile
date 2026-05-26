TARGET:server

main.o:main.cpp
	g++ -c -o main.o -std=c++20 -I ../shared -I Listener main.cpp

listener.o:Listener/listener.cpp
	g++ -c -o listener.o -std=c++20 -I Listener -I ../shared Listener/listener.cpp

liblistener.a:listener.o
	ar rs liblistener.a listener.o

server:main.o liblistener.a
	g++ main.o liblistener.a -o server