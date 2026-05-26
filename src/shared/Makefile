TARGET:libhelper.a

helper.o:helper.cpp
	g++ -c -o helper.o helper.cpp -I .

libhelper.a:helper.o
	ar rs libhelper.a helper.o