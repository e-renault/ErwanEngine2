# ErwanEngine2.0

This is an early buggy access. If it works (houra!), be carefull with CPU and GPU T°, since they could heat a lot, and the program is not designed to deal with it.

Also, do not import obj file that have more than 600 triangles or 400 points. 

Currently, lights are experimental and cannot be changed.

I'm not responsible of what could cause the execution of the program in any way.


## Install
The program have been developped under kde neon 5.27

You would probably need to install :
 * gcc
 * freeglut3
 * opencl
 
 	It may depends on you hardware and configuration, for me it was packages :
    intel-opencl-icd, ocl-icd-opencl-dev, opencl-headers
    

Depending on you hardware, you could have to change DEVICE_ID and PLATFORM_ID in order to select you GPU (check first line of "opencl_renderer.c")

## Compile
You can use the command :
```
$ gcc main.c -lm -lOpenCL -lpthread -lGL -lglut 
```

optional: 
`-DDEBUG_RUN_INFO=1` -> additionnal debug info

## Run
```
./a.out [--optionnals arguments]
```

optional:
```
--file  [OBJ_FILE_TO_LOAD]              (default:?)  <- this one is really experimental++
--fov   [FIELD_OF_VIEW_IN_DEGREE]       (default:70)
--xres  [HORISONTAL_RESOLUTION_IN_PX]   (default:?)
--yres  [VERTICAL_RESOLUTION_IN_PX]     (default:?)
```

debug (experimental):
```
--path  [SCENE_PATH]                    (default:10)
--max_triangle [NB_TRIANGLES_MAX]       (default:?)
--max_lightsource [NB_LIGHT_MAX]        (default:?)
--FPS
--GINFO
--KINFO
--HINFO
```
## Available scenes
This is what files you can use using "--file=" argument.

 * cheval.obj
 * cube.obj
 * default.obj
 * fox.obj
 * horse2.obj
 * maxwell.obj
 * ~~nature.obj~~ (Please don't)
 * pyramid.obj
 * tenet.obj

## Controls
### Camera
    `zqsd`    -> move
    `space` -> go up
    `e`     -> go down
    `oklm`    -> rotate camera


### Sun (experimental)
    `t/g`  -> increase/decrease x vector property
    `y/h`  -> increase/decrease y vector property
    `u/j`  -> increase/decrease z vector property

### quit (properly)
    `x`
This will also save last frame into _output/output.ppm and erase last one. Consider exporting as png  using an external tool (ppm format is really row).