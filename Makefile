srcs = main.cpp obj.hpp utils.hpp
deps = $(srcs)
monkey: $(deps)
	g++ main.cpp -o monkey -lglfw3 -lGLEW -lGL -ldl -lX11 -lpthread -g

monkey.exe: $(deps)
	g++ main.cpp -o monkey.exe -Llibs -Iinclude -lglfw3 -DGLEW_STATIC -lglew32s -lopengl32
