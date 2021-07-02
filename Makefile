srcs = src/main.cpp src/obj.cpp src/utils.cpp
includes = include/obj.h include/utils.h
deps = $(srcs) $(includes)
flags = -Iinclude -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread -g
monkey: $(deps)
	g++ $(srcs) -o monkey $(flags)
