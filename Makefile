all: mbr-parse

mbr-parse: main.o
	$(LINK.cc) $^ -o $@

clean:
	rm -f mbr-parse *.o