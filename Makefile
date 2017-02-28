ESDK = ${EPIPHANY_HOME}
ELIBS =
INCLUDES = -I/usr/include/SDL2 -I.

HOST_CFLAGS = -std=c99 -g -fopenmp -Wall -DREENTRANT 

HOST_LIBS = -L/usr/arm-linux-gnueabihf/lib

HOST_LIB_NAMES = -lm -lSDL2 -lSDL2_image


all: ann_main.c ann_matrix_ops.c ann_matrix_ops.h ann_file_ops.c ann_file_ops.h ann_config.h 
	@echo "CC $<"
	gcc $(HOST_CFLAGS) $< ann_matrix_ops.c ann_file_ops.c -o $@ $(ELIBS) $(INCLUDES) $(HOST_LIB_NAMES) 

%: %.c
	@echo "CCC $<"
	gcc $< -o $@ -Wall -g -fopenmp -lm -I.

clean:
	rm -r bin
