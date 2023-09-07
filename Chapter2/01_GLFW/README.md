This did not spark joy. Inside the book there was no mention of libraries or such. To get this built required

#GLAD
 - https://glad.dav1d.de/ generate for version 4.6 core. Download drc + include
 - Inside vs, add to PROJECT an external file - glad.c

#GLFW
 - bootstrap the package down to deps
 - make build folder + previous build steps
 - inisde vs, build the glfw3 project
 - link vs to build/debug
 - clear out all additional include libs except for glfw3.lib
