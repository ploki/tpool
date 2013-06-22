PROGNAME = thread_pool_test

CC ?= gcc
SRC = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(notdir $(SRC)))

CFLAGS = -std=c99 -Wall -Wunused -Werror -Wuninitialized -D_REENTRANT -fPIC -I.

LIBS = -lpthread

# in case we use __cyg_profile_func_{enter,exit}
#CFLAGS += -finstrument-functions
#LIBS += -ldl

LDFLAGS = $(LIBS)

$(PROGNAME): $(OBJS)
	$(CC) -o $(PROGNAME) $(CFLAGS) $^ $(LDFLAGS)


%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) $(PROGNAME) *~