a.out: main.cpp reference.hpp utils.hpp
	g++ main.cpp -o a.out -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread
