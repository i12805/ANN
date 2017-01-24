ESDK = ${EPIPHANY_HOME}
ELIBS = -L${ESDK}/tools/host/lib
INCLUDES = -I${ESDK}/tools/host/include -I.
ELDF=${ESDK}/bsps/current/internal.ldf


E_CFLAGS = -g -O3 -ffast-math -Wall
HOST_CFLAGS = -g -fopenmp -Wall

HOST_LIBS = -L${ESDK}/tools/host/lib \
            -L/usr/arm-linux-gnueabihf/lib

HOST_LIB_NAMES = -le-hal -le-loader -lpthread -lm

E_LIB_NAMES = -le-lib -lm 


all: ann_main.c ann_matrix_ops.c ann_matrix_ops.h ann_file_ops.c ann_file_ops.h ann_config.h e_task.elf
	@echo "CC $<"
	gcc $(HOST_CFLAGS) $< ann_matrix_ops.c ann_file_ops.c -o $@ $(ELIBS) $(INCLUDES) $(HOST_LIB_NAMES) #-D USE_PARALLELLA

%: %.c
	@echo "CCC $<"
	gcc $< -o $@ -Wall -g -fopenmp -lm -I.

%.elf: %.c
	@echo "CC $<"
	@e-gcc $(E_CFLAGS) -T ${ELDF} -o $@ $< $(LIBS) $(ELIBS) $(E_LIB_NAMES) $(INCLUDES)

clean:
	rm -r bin
