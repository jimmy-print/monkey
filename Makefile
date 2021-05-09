a.out: main.cpp
	g++ main.cpp -o a.out -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread
