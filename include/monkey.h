#ifndef MONKEY_H
#define MONKEY_H

#include <obj.h>
#include <random>

// No cpp file (couldn't figure out how to get rid of redefinition error
// for constructor)

class Monkey : public Model {
public:
	int health = 100;
	int cant_see_player = false;
	int randomly_moving = false;
	float random_dx = (float) (rand() % 50) / 1000;
	float random_dz = (float) (rand() % 50) / 1000;
	Monkey(std::string objpath) : Model(objpath) {
		int a = rand() % 2;
		int b = rand() % 2;

		random_dx = -random_dx ? a == 1 : random_dx;
		random_dz = -random_dz ? b == 1 : random_dz;
	}

	void handle_random_movement() {
		if (cant_see_player) {
			int n = rand() % 200;
			if (n == 1) {
				randomly_moving = !randomly_moving;
				if (randomly_moving) { // if it switched from still to moving
					// gen new values for d*
					random_dx = (float) (rand() % 50) / 1000;
					random_dz = (float) (rand() % 50) / 1000;
					int a = rand() % 2;
					int b = rand() % 2;
					random_dx = a == 1 ? -random_dx : random_dx;
					random_dz = b == 1 ? -random_dz : random_dz;
				}
			}

			if (randomly_moving) {
				move_x(random_dx);
				move_z(random_dz);
			}
		}
	}
};

#endif
