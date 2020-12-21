
.PHONY: all
all:
	gcc generator.c -o generator
	./generator > tilemap.c
	cp tilemap.c ../
	# python parser.py > map