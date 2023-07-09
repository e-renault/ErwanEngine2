exec: compile
	./a.out --XRES 800 --YRES 600 --FOV 80 --obj default.obj --texture default_texture.ppm

compile:
	gcc main.c -lm -lOpenCL -lpthread -lGL -lglut
	echo "Compilation Done"

clean:
	rm a.out
