
PORT     = 9987
APPNAME  = temp_serv
LIBPATH1 = `pwd`/include/
LIBPATH2 = `pwd`/lib/

CFLAGS+=-I${LIBPATH1}
LDFLAGS+=-L${LIBPATH2}

CC = gcc

all:
	make -C src
	${CC} ${CFLAGS} select.c -o ${APPNAME} ${LDFLAGS} -llogger -lserver_sqlite3 -l sqlite3

clean:
	rm -f ${APPNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH
	./${APPNAME} -p ${PORT}
