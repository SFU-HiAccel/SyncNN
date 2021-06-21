all: snn
snn: src.o tb.o
	g++ -O2 src.o tb.o -o snn
src.o: src.cpp
	g++ -O2 -c src.cpp 
tb.o: tb.cpp
	g++ -O2 -c tb.cpp 
clean:
	rm -f *.o snn
