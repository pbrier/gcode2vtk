
all: gcode2vtk

gcode2vtk: gcode2vtk.cpp
	${CXX} -O3 gcode2vtk.cpp -o gcode2vtk
	
clean: 
	rm *.o gcode2vtk