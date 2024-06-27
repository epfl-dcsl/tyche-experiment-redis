all: app_stdin app_tcp 

RB_PATH=../../vmxvmm/C/libraries/ringbuf/
COMMON_PATH=../../vmxvmm/linux/drivers/tyche/libraries/common/

# ————————————————————————————————— Includes ————————————————————————————————— #

INCLUDES = -I$(RB_PATH)/include -I$(COMMON_PATH)/include -Iinclude/

# ————————————————————————————————— Sources —————————————————————————————————— #

SRCS = $(wildcard src/*.c)


app_stdin: $(SRCS)
	gcc -g -DRUN_STDIN=1 -DTYCHE_USER_SPACE=1 -o $@ $(INCLUDES) $(SRCS)

app_tcp: $(SRCS)
	gcc -g -DRUN_TCP=1 -DTYCHE_USER_SPACE=1 -o $@ $(INCLUDES) $(SRCS)

.PHONY: clean

clean:
	rm app_stdin
