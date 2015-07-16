PRG = kjclient
PRG1 = kjservice

AJTCL_BASE = ..

CFLAGS = -O0 -g -Wall -static -DAJ_MAIN -I/usr/include/keapi -I$(AJTCL_BASE)/ajtcl/inc -I$(AJTCL_BASE)/ajtcl/target/linux
LDFLAGS = -L$(AJTCL_BASE)/ajtcl -lkeapi -lajtcl_st -lpthread -lcrypto

CC = gcc
all: $(PRG) $(PRG1)
$(PRG): $(PRG).o
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

$(PRG1): $(PRG1).o
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

clean:
	rm -f $(PRG) *.o
	rm -f $(PRG1) *.o

