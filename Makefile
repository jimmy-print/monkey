all: monkey
monkey: main.cpp obj.hpp utils.hpp tri.cpp
	g++ main.cpp tri.cpp -o monkey -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread -g

monkey.exe: main.cpp obj.hpp utils.hpp
	g++ main.cpp -o monkey.exe -Llibs -Iinclude -lglfw3 -DGLEW_STATIC -lglew32s -lopengl32
