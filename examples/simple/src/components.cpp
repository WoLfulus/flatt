// Generated, don't touch me.

#include "flecs.h"
#include "components.h"

components::components(flecs::world& world) {
  world.component<Position>()
    ;
  world.component<Velocity>()
    ;

}