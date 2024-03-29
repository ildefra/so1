SRCDIR = src
OBJDIR = obj
BINDIR = bin
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Wshadow

.PHONY: all clean

all: $(BINDIR)/client $(BINDIR)/server
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server $(OBJDIR)/*.o

$(BINDIR)/client: $(OBJDIR)/bel_client.o $(OBJDIR)/bel_common.o
	gcc $(CFLAGS) -o $(BINDIR)/client $(OBJDIR)/bel_client.o \
			$(OBJDIR)/bel_common.o
$(OBJDIR)/bel_client.o: $(SRCDIR)/bel_client.c \
		$(SRCDIR)/bel_common.h
	gcc $(CFLAGS) -c -o $(OBJDIR)/bel_client.o $(SRCDIR)/bel_client.c

$(BINDIR)/server: $(OBJDIR)/bel_server.o \
		$(OBJDIR)/bel_common.o $(OBJDIR)/msg_storage.o
	gcc $(CFLAGS) -o $(BINDIR)/server $(OBJDIR)/bel_server.o \
			$(OBJDIR)/bel_common.o $(OBJDIR)/msg_storage.o
$(OBJDIR)/bel_server.o: $(SRCDIR)/bel_server.c \
		$(SRCDIR)/bel_common.h $(SRCDIR)/msg_storage.h
	gcc $(CFLAGS) -c -o $(OBJDIR)/bel_server.o $(SRCDIR)/bel_server.c

$(OBJDIR)/bel_common.o: $(SRCDIR)/bel_common.h $(SRCDIR)/bel_common.c
	gcc $(CFLAGS) -c -o $(OBJDIR)/bel_common.o $(SRCDIR)/bel_common.c

$(OBJDIR)/msg_storage.o: $(SRCDIR)/msg_storage.h $(SRCDIR)/msg_storage.c
	gcc $(CFLAGS) -c -o $(OBJDIR)/msg_storage.o $(SRCDIR)/msg_storage.c
