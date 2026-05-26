TARGET:client

main.o:main.cpp
	g++ -c -o main.o -I Connector -I ../shared main.cpp

connector.o:Connector/connector.cpp
	g++ -c -o connector.o -I Connector -I ../shared Connector/connector.cpp

libconnector.a:connector.o
	ar rs libconnector.a connector.o

client:libconnector.a main.o
	g++ -o client main.o libconnector.a ../shared/libhelper.a

clean:
	rm main.o
	rm connector.o
	rm libconnector.a
	rm client