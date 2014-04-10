
export CFLAGS += -Wall -Wextra

test: test.o ../libaw-parse.a
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I.. -c $< -o $@

../libaw-parse.a:
	$(MAKE) -C..

.PHONY: clean
clean:
	rm -f test test.o

