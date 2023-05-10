
PORT     = 9987
HOSTNAME = 192.168.68.129
INTERVAL = 3
APPNAME  = temp_cli
DATABASE = test.db
LIBNAME  = lib
LIBPATH1 = `pwd`/include/
LIBPATH2 = `pwd`/lib/
LIBPATH3 = logger
LIBPATH4 = client
LIBPATH5 = temp
LIBPATH6 = my_sqlite3
LIBPATH7 = get_time


CFLAGS+=-I${LIBPATH1}
LDFLAGS+=-L${LIBPATH2}
LDFLAGS+=-l${LIBPATH3}
LDFLAGS+=-l${LIBPATH4}
LDFLAGS+=-l${LIBPATH5}
LDFLAGS+=-l${LIBPATH6}
LDFLAGS+=-l${LIBPATH7}


CC = gcc
AR = ar


all:
	${CC} ${CFLAGS} socket_client.c -o ${APPNAME} ${LDFLAGS} -l sqlite3

clean:
	rm -f ${APPNAME}
	rm -f ${DATABASE}
	rm -rf ${LIBNAME}

run:
	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:LIBPATH
	./${APPNAME} -p ${PORT} -h ${HOSTNAME} -t ${INTERVAL}
