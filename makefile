buffer: main.o 
	g++ -o buffer main.o 
	./buffer

main.o: main.cpp Page.h Frame.h BufferManager.h
	g++ -c main.cpp

clean:
	rm -f *.o buffer