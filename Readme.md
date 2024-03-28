3D Renderer using fixed point math. For casio ClassPad ii fx-cp400


https://github.com/ClasspadDev/CP-3D-CarGoWroom/assets/13310559/99807a22-c964-4338-ba07-8b30e5b67984

> The code was originally done by [Henri](https://github.com/im-henri/) and given away to the community
![legacy.png](docs/legacy.png)
New maintainer are welcome ! 

# Install

[![Get it with HollyHock2](docs/giw-hhk.svg)](https://github.com/ClasspadDev/CP-3D-CarGoWroom/releases)

Go to the [releases](https://github.com/ClasspadDev/CP-3D-CarGoWroom/releases) and get the latest build

# Features

- .obj models (converted to binary format)
- Textures
- Lighting
- 6 different render modes

# Controls


```
8 4 2 6 9 3 = Camera rotation
D-PAD       = Movement
+ -         = FOV +/-
0           = Render Mode Cycle
Clear       = Exit
```

# Modding

To create new binary format models + textures edit and run python script
```
python/ObjTexConverter.py
```

# Compile it yourself

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/ClasspadDev/CP-3D-CarGoWroom)

## For your calculator 

Run makefile:
```
make
```

Copy everything to the root of calculator:
```
App_3D_CarGoWroom.bin
big_endian_my_car.pkObj    (from folder ./3D_Converted_Models)
big_endian_my_car.texture  (from folder ./3D_Converted_Models)
big_endian_test2.pkObj    (from folder ./3D_Converted_Models)
big_map.map                (from folder ./python)
```

## On your PC

You'll need SDL2 graphics library
```
make PC
```

Controls are :
```
W S A D R F = Camera rotation
Arrow Keys  = Movement
1 2         = FOV +/-
E           = Render Mode Cycle
Esc         = Exit
```


# Credits
- Original code and based on CP3D Render by Henri: https://github.com/im-henri/CP_3D_render
- hollyhock2: https://github.com/SnailMath/hollyhock-2
- Libfixmath: https://github.com/PetteriAimonen/libfixmath/tree/master
