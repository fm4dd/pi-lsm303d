CC=gcc
CFLAGS= -O3 -Wall -g
LIBS= -lm
AR=ar

ALLBIN=getlsm303d

all: ${ALLBIN}

clean:
	rm -f *.o ${ALLBIN}

getlsm303d: i2c_lsm303d.o getlsm303d.o
	$(CC) i2c_lsm303d.o getlsm303d.o -o getlsm303d ${LIBS}

