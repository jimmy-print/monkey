a.out: main.cpp reference.hpp utils.hpp
	g++ main.cpp -o a.out -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread

a.exe: main.cpp reference.hpp utils.hpp
	g++ main.cpp -o a.exe -Llibs -Iinclude -lglfw3 -DGLEW_STATIC -lglew32s -lopengl32
