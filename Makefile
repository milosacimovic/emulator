CC = g++
CFLAGS = -g -w -std=gnu++11
EMULATOR_FILES = src/utils.cpp src/tables.cpp src/interpret.cpp src/memory.cpp
OBJ_FILES = src/utils.o src/tables.o src/interpret.o src/memory.o

all:
	$(CC) $(CFLAGS) -o bin/emulator src/emulator.cpp $(EMULATOR_FILES) -pthread

src/%.o: $(EMULATOR_FILES) clean
	$(CC) -o $@ -c $< $(CFLAGS)
emulator: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o emulator
