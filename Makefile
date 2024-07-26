srcs = src/main.cpp src/obj.cpp src/utils.cpp src/text.cpp
includes = include/obj.h include/utils.h include/text.h include/monkey.h
deps = $(srcs) $(includes)
flags = `pkg-config --cflags --libs freetype2` -Iinclude -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread -g
monkey: $(deps)
	g++ $(srcs) -o monkey $(flags)

monkeymacos: $(deps)
	g++ -o monkey $(srcs) `pkg-config --cflags --libs glfw3 glew freetype2` -framework OpenGL -g -std=c++17 -Iinclude
