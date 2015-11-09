CFLAGS += -Wall

all: list_micro tp_list_mchsh

list_micro: list_micro.c
	$(CC) $(CFLAGS) -o $@ $^

tp_list_mchsh: tp_list_mchsh.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	@-rm -rf list_micro tp_list_mchsh > /dev/null 2>&1

.PHONY: clean all
