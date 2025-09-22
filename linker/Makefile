
OBJS = linker.o parser.o pass1.o pass2.o symbol.o

linker: $(OBJS)
	g++ -g $(OBJS) -o linker

linker.o: linker.cpp linker.h
	g++ -g -c linker.cpp

parser.o: parser.cpp linker.h
	g++ -g -c parser.cpp

pass1.o: pass1.cpp linker.h
	g++ -g -c pass1.cpp

pass2.o: pass2.cpp linker.h
	g++ -g -c pass2.cpp

symbol.o: symbol.cpp linker.h
	g++ -g -c symbol.cpp

clean:
	rm -f linker tokenizer *.o *~ *.out

tokenize: tokenizer.cpp
	g++ tokenizer.cpp -g -o tokenizer