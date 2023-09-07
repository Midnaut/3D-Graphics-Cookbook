## GLAD
 - https://glad.dav1d.de/ generate for version 4.6 core. Download drc + include
 - Inside vs, add to PROJECT an external file - glad.c

## GLFW
 - bootstrap the package down to deps
 - make build folder + previous build steps
 - inisde vs, build the glfw3 project
 - link vs to build/debug
 - clear out all additional include libs except for glfw3.lib

 
## Include directories required
  - `C:\DEV\3D-Graphics-Cookbook-Projects\deps\src\glm\glm`
  - `C:\DEV\3D-Graphics-Cookbook-Projects\deps\src\glfw\include`
  - `C:\DEV\3D-Graphics-Cookbook-Projects\deps\glad\include`
  - `C:\DEV\3D-Graphics-Cookbook-Projects\deps\src\stb`

## Library directories required (assuming you built glfw the same way)
  - `C:\DEV\3D-Graphics-Cookbook-Projects\deps\src\glfw\build\src\Debug`
