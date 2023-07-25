# ErwanEngine2.0

Hello there =)
Here is my second version of the ErwanEngine, implemented in C using opencl to implement a 3D engine. It has been developped from scratch without  tutorial.

![preview](https://github.com/e-renault/ErwanEngine2/blob/main/_output/EEAO_test6.png?raw=true)

Currently in developpement. Some strange behaviours could occure.

**This is an early buggy access**

## Install

You would probably need to install :

* gcc
* freeglut3
* opencl

It may depends on you hardware and configuration, for me it was packages :
```
apt install gcc freeglut3-dev opencl-headers ocl-icd-opencl-dev intel-opencl-icd ndidia-opencl-icd
```
The lasts two packages is related to you GPU model.

Depending on you hardware, you could have to change DEVICE_ID and PLATFORM_ID in order to select you GPU (check firsts line of "main.c"). Mine is intel integrated Intel(R) UHD Graphics 620.

## Makefile

A Makefile is included in the project. It basically just compile and run the program. You may still need to install packages by yourself (see install section)

## Compile

You can use the command :

```
$ gcc main.c -lm -lOpenCL -lpthread -lGL -lglut 
```

## Run

```
./a.out [--optionnals arguments]
```

optional:

```
--obj  [OBJ_FILE_TO_LOAD] (default:"src/default/default.obj")  <- you should use one of the Availables scenes (see section "Available scenes").
--FOV   [FIELD_OF_VIEW_IN_DEGREE]       (default:70)
--RES   [RESOLUTION_IN_PX]              (default:800x600)
```

debug (experimental, notes for myself):

```
--XYZ
--GINFO
--KINFO
--HINFO
--RINFO
```

## Available scenes

This is what files you can use using "--obj=" argument.

```
default      --obj=src/default/default.obj
minecraft    --obj=src/minecraft/minecraft.obj
ball         --obj=src/ball/ball.obj
cheval       --obj=src/cheval/cheval.obj
crate        --obj=src/crate/crate.obj
maxwell      --obj=src/maxwell/maxwell.obj
tenet        --obj=src/tenet/tenet.obj
test         --obj=src/test/test.obj
```

## Controls

### Camera

```
`zqsd`    -> move
`space`   -> go up
`e`       -> go down
`oklm`    -> rotate camera
```

### Misk (experimental)

```
`r`    -> process render of global illumination
`t/g`  -> increase/decrease sun x vector property 
`y/h`  -> increase/decrease sun y vector property
`u/j`  -> increase/decrease sun z vector property
```

### quit (properly)

```
`x`
```

This will also save last frame into \_output/output.ppm and erase last one. Consider exporting as png  using an external tool (ppm format is really row).
