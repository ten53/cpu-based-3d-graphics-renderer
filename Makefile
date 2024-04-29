build:
	gcc -Wall -std=c99 ./src/*.c -o renderer -lSDL2

run:
	./renderer

clean:
	rm renderer
