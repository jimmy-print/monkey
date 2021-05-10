test: test.cpp
	g++ test.cpp -o test -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread
a.out: main.cpp
	g++ main.cpp -o a.out -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread
