include ../MakeVars

C_SRCS= $(shell ls *.c 2> /dev/null)
C_OBJS= $(patsubst %.c,%.o,$(C_SRCS))
CURR_DIR=$(shell basename `pwd`)

all: .INDEX $(C_OBJS)

.INDEX : $(C_OBJS)
	@rm -f .INDEX
	@(for i in $(C_OBJS); \
	do \
	echo "$(CURR_DIR)/$$i " >> .INDEX; \
	done)

%.o: %.c
	$(CC) $(CFLAGS) -c $< 

clean :
	rm -f *.o
	rm -f *~ .INDEX
