exec: compile
	./ee --RES 800x600 --FOV 80 --SCN scn/test/test.obj

compile:
	gcc main.c -lm -lOpenCL -lpthread -lGL -lglut -o ee
	chmod +x ee

windows:
	x86_64-w64-mingw32-gcc main.c -lm -lOpenCL -lpthread -lGL -lglut

install:
	apt install gcc freeglut3-dev opencl-headers ocl-icd-opencl-dev intel-opencl-icd ndidia-opencl-icd

clean:
	rm ee
