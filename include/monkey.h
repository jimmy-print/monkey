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
	Monkey(struct shared *s, glm::vec3 pos) : Model(s) {
		int a = rand() % 2;
		int b = rand() % 2;

		random_dx = a == 1 ? -random_dx : random_dx;
		random_dz = b == 1 ? -random_dz : random_dz;

		int spawn_range = 10;

		float spawn_dx, spawn_dz;
		spawn_dx = rand() % spawn_range * 2;
		spawn_dx = rand() % 2 == 1 ? -spawn_dx : spawn_dx;
		spawn_dz = rand() % spawn_range * 2;
		spawn_dz = rand() % 2 == 1 ? -spawn_dz : spawn_dz;

		float spawn_dx_abs, spawn_dz_abs; // to origin
		spawn_dx_abs = pos[0] + spawn_dx;
		spawn_dz_abs = pos[2] + spawn_dz;

		move_x(spawn_dx_abs);
		move_z(spawn_dz_abs);
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
