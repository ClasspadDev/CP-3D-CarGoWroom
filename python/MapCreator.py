from ALL_PATHS import *
from dataclasses import dataclass
from enum import Enum

# 0 = (0, 0) coordinate and Player location
# x = wall/slow down block
# s = boost block
# . = empty space

# " " (whitespace) is ignored

SQUARE_SIZE = 10.0

myMap ="""
x x x x x x x x x x . . . . . . . . . . . . . . . .
x . . . . . . . . x x x x x x x x x x x x x x x x .
x . . . . . . . . . . . . . . x . . . . . . . . x .
x . . . . . . . . . . . . . . x . . . . . . . . x .
x . . . . . . s . . . . . . . x . . . . . . . . x .
x . . . . . . . . . . . . . . x . . . . . . . . x .
. x . . s . . . . . x . . . . x . . . . x . . s x .
. x . . . . . . . x . . . . . x . . . . x . . . x .
. x . . . . . x . . . . . . . x . . . . x . . . x .
. x . . . . x . . . . . . . . x . . . . x . . . x .
. x . s . . x . . . . . . . . x . . . . x . . . x .
. x . . . . x x . . . . . . . . . . . . x . . . x .
. x . . . . x . x . . . . . . . . . . . x . . . x .
. x . . . . x . . x . . . . . . . . . . x . . . x .
. x . s . . x . . x x x x x x x x x x x x . . . x .
. x . . . . x . . . . . . . . . . . . . x . . . x .
. x . . . . x . . . . . . . . . . . . . x . . . x .
. x . . . . x . . . . . . . . . . . . . x . . . x .
. x . . . . x . . . . . . . . . . . . . x . . . x .
. x . . . . x . . . . . . . . . . . . . x . . s x .
. x . s . . x . . . . . . x . . . . . . x . . . x .
. x . . . . . . . . . . . . x . . . . . x . 0 . x .
. x . . . . . . . . . . . . . x . . . . x . . . x .
. x . . . . . . . . . . . . . . x . . . x . . . x x
. x . . . . . . . . . . . . . . . x . . x . . . . x
. x . . s . . . . . . . . . . . . . x . x . . . . x
. x . . . . . . . . . . . . . . . . . x x . . . . x
. x . . . . . . s . . . . . . . . . . . . . . . . x
. x . . . . . . . . . . . . . . . . . . . . . . . x
. . x x x x x x x x x x x x x x x x x x x x x x x x
"""

##########################
##
## Format:
##      1. Count of items - 32b unsigned
##         Multiple times:
##     (2)     Fix16 x;
##     (3)     Fix16 y;
##     (4)     32b type;
##
##########################

class MapElementTypes(Enum):
    Player = 0
    Wall   = 1
    Boost  = 2

@dataclass
class MapElement:
    x: int
    y: int
    type: MapElementTypes

def main():
    mapElements = createMap()
    writeMap(mapElements, f"{script_path}/big_map.map", f"{script_path}/little_map.map")

def writeMap(mapElements, out_big_endian, out_little_endian):
    # Classpad wants big-endian
    fBig = open(out_big_endian, "wb")
    # Computer ("normal" computer uses little endian, because big endian is stupid choice from hw perspective...)
    fLit = open(out_little_endian, "wb")

    # Write info about length of our data
    write_out_32b(fBig, fLit, len(mapElements))

    # Write vertices in fix16 format
    for elem in mapElements:
        elem: MapElement = elem
        xPos = float(elem.x) * SQUARE_SIZE
        yPos = float(elem.y) * SQUARE_SIZE
        write_out_32b(fBig, fLit, float_to_fix16(xPos))
        write_out_32b(fBig, fLit, float_to_fix16(yPos))
        write_out_32b(fBig, fLit, elem.type.value)

    fBig.close()
    fLit.close()

def float_to_fix16(value: float):
    value = int(value * (1<<16))
    return value + (1<<32 if value < 0 else 0)

def write_out_32b(fBig, fLit, value):
    fBig.write(value.to_bytes(4, 'big'))
    fLit.write(value.to_bytes(4, 'little'))

def createMap():
    # Final elements to be added to actual map will be here
    allMapElements = []
    #
    stringMap = []
    rowCnt = 0
    colCnt = 0
    zero_point = None
    for row in myMap.split("\n"):
        # Initial text processing and skipping empty rows
        row:str = row.strip()
        row:str = row.replace(" ", "")
        if len(row) == 0: continue
        row = row[::-1]

        # Row and col counting
        if colCnt == 0:          colCnt = len(row)
        elif colCnt != len(row): raise ValueError("Each row must have equal amount of items!")
        rowCnt += 1

        # Actual map processing
        mapRow = []
        col_idx = 0
        for itm in row:
            if   itm == ".": mapRow.append(".")
            elif itm == "x": mapRow.append("x")
            elif itm == "0":
                if zero_point == None:
                    zero_point = (col_idx, rowCnt-1)
                    mapRow.append("0")
                else:
                    raise ValueError(f"Must have only one zero location! -> ({itm})")
            elif itm == "s": mapRow.append("s")
            else: raise ValueError(f"Unknow map item! -> ({itm})")
            col_idx += 1
        stringMap.append(mapRow)

    # Add coordinates to map
    coordinates = []
    for y in range(rowCnt):
        tmpRow = []
        for x in range(colCnt):
            tmpRow.append((x-zero_point[0],y-zero_point[1]))
        coordinates.append(tmpRow)

    # Now loop both together
    for y in range(rowCnt):
        for x in range(colCnt):
            itm = stringMap[y][x]
            coords = coordinates[y][x]
            if itm == ".": continue
            elif itm == "x": allMapElements.append(MapElement(coords[0], coords[1], MapElementTypes.Wall))
            elif itm == "0": allMapElements.append(MapElement(coords[0], coords[1], MapElementTypes.Player))
            elif itm == "s": allMapElements.append(MapElement(coords[0], coords[1], MapElementTypes.Boost))
            else: raise NotImplementedError(f"TODO: ({itm})")

    return allMapElements

if __name__ == "__main__":
    main()