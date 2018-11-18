lcc: lcc.c

test: lcc
	./test.sh

clean:
	rm -f lcc *.o *~ tmp*
