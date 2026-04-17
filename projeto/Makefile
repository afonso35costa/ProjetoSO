CC = gcc
CFLAGS = -Wall -g -Iinclude `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`

all: folders controller runner

controller: bin/controller

runner: bin/runner

folders:
	@mkdir -p src include obj bin tmp

bin/controller: obj/controller.o obj/scheduler.o obj/comandos.o obj/queue.o obj/package.o obj/executer.o obj/hashRR.o
	$(CC) $^ -o $@ $(LDFLAGS)

bin/runner: obj/runner.o obj/comandos.o obj/package.o obj/executer.o
	$(CC) $^ -o $@ $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* tmp/* bin/*