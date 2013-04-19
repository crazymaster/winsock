SRC=server.c
OBJS=$(SRC:.c=.o)
RC=resource_ser.rc
OBJS+=$(RC:.rc=.o)
PROG=server.exe
CC=gcc
MENU=windres
SED=sed 's/\\/\\\\/'
CFLAGS=-Wall -O3 -finput-charset=cp932 -fexec-charset=cp932
LDFLAGS=-mwindows -lws2_32 -lwsock32
RM=rm


%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.rc
	$(SED) $(RC) | $(MENU) -o $*.o

.PHONY : all
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

.PHONY : clean
clean:
	$(RM) $(OBJS)
