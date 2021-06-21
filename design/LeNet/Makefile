all: snn
snn: src.o tb.o
	g++ src.o tb.o -o snn
src.o: src.cpp
	g++ -c src.cpp 
tb.o: tb.cpp
	g++ -c tb.cpp 
clean:
	rm -f *.o snn
