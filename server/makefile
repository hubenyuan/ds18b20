
PORT     = 9987
APPNAME  = temp_serv
DATABASE = server.db
LIBNAME  = lib
LIBPATH1 = `pwd`/include/
LIBPATH2 = `pwd`/lib/
LIBPATH3 = logger
LIBPATH4 = server_sqlite3

CFLAGS+=-I${LIBPATH1}
LDFLAGS+=-L${LIBPATH2}
LDFLAGS+=-l${LIBPATH3}
LDFLAGS+=-l${LIBPATH4}


CC = gcc

all:
	${CC} ${CFLAGS} select.c -o ${APPNAME} ${LDFLAGS}  -l sqlite3

clean:
	rm -f ${APPNAME}
	rm -f ${DATABASE}
	rm -rf ${LIBNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH
	./${APPNAME} -p ${PORT}
