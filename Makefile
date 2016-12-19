EBSP=epiphany-bsp/
ESDK=${EPIPHANY_HOME}
ELDF=${EBSP}/ebsp_fast.ldf

# no-tree-loop-distribute-patters makes sure the compiler
# does NOT replace loops with calls to memcpy, residing in external memory
CFLAGS=-std=c99 -Wall -O3 -fno-tree-loop-distribute-patterns
#CFLAGS=-std=c99 -O3 -ffast-math -Wall

INCLUDES = -I${EBSP}/include \
           -I${ESDK}/tools/host/include

LIBS = -L${EBSP}/lib

HOST_LIBS = -L${ESDK}/tools/host/lib \
            -L/usr/arm-linux-gnueabihf/lib

E_LIBS = -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

all: cannon

bin:
	@mkdir -p bin

bin/%: %.c
	@echo "CC $<"
	@gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)

bin/%.elf: %.c
	@echo "CC $<"
	@e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/%.s: %.c
	@echo "CC $<"
	@$e-gcc $(CFLAGS) -T $(ELDF) $(INCLUDES) -fverbose-asm -S $< -o $@ $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/%.srec: bin/%.elf
	@e-objcopy --srec-forceS3 --output-target srec $< $@


##############################################

cannon: bin bin/host_cannon bin/e_cannon.elf common.h


clean:
	rm -r bin
