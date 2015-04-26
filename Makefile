
all: clean build

build:
	mkdir -p build; cd build; cmake ..; make

test: build
	build/test/RocketScience test/program.c

clean:
	rm -rf build