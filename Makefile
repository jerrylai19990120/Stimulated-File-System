FLAGS= -Wall -Werror -fsanitize=address -g
OBJ = simfs.o initfs.o printfs.o simfs_ops.o
DEPENDENCIES = simfs.h simfstypes.h

all : simfs

simfs : ${OBJ}
	gcc ${FLAGS} -o $@ $^

%.o : %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean :
	rm -f *.o simfs
