#ifndef MONKEY_H
#define MONKEY_H

#include <obj.h>

// No cpp file (couldn't figure out how to get rid of redefinition error
// for constructor)

class Monkey : public Model {
public:
    int health = 100;
    Monkey(std::string objpath) : Model(objpath) {

    }
};

#endif