

PREFIX = `pwd`/../


LIBNAME1 = my_sqlite3
LIBNAME2 = get_time
LIBNAME3 = temp
LIBNAME4 = logger
LIBNAME5 = client

all: static_lib

static_lib:
	@gcc -c *.c
	ar -rcs lib${LIBNAME1}.a *.o
	ar -rcs lib${LIBNAME2}.a *.o
	ar -rcs lib${LIBNAME3}.a *.o
	ar -rcs lib${LIBNAME4}.a *.o
	ar -rcs lib${LIBNAME5}.a *.o

clean:
	rm -f *.a
	rm -f *.o

distclean:clean
	rm -f *.a

install:
	mkdir -p ${PREFIX}/lib
	mkdir -p ${PREFIX}/include
	cp lib*.a ${PREFIX}/lib
	cp *.h ${PREFIX}/include

uninstall:
	rm -f ${PREFIX}/lib/lib*
