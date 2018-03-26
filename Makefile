CXX = i686-w64-mingw32-g++
CCOPT = -O3 -static-libstdc++ -static-libgcc -static

all: gcode2vtk

gcode2vtk: gcode2vtk.cpp
	${CXX} ${CCOPT} gcode2vtk.cpp -o gcode2vtk 
	
clean: 
	rm *.o gcode2vtk
