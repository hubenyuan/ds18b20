

PREFIX = `pwd`/../


LIBNAME1 = my_sqlite3
LIBNAME2 = time
LIBNAME3 = temp
LIBNAME4 = logger

all: static_lib

static_lib:
	@gcc -c *.c
	ar -rcs lib${LIBNAME1}.a *.o
	ar -rcs lib${LIBNAME2}.a *.o
	ar -rcs lib${LIBNAME3}.a *.o
	ar -rcs lib${LIBNAME4}.a *.o

clean:
	rm -f *.o
	rm -f *.o

distclean:clean
	rm -f *.a

install:
	mkdir -p ${PREFIX}/lib
	mkdir -p ${PREFIX}/include
	cp lib*.a ${PREFIX}/lib
	cp *.h ${PREFIX}/include

uninstall:
	rm -f ${PREFIX}/lib/lib${LIBNAME1}.*
	rm -f ${PREFIX}/lib/lib${LIBNAME2}.*
	rm -f ${PREFIX}/lib/lib${LIBNAME3}.*
	rm -f ${PREFIX}/lib/lib${LIBNAME4}.*
