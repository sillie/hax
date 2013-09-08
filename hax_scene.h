#ifndef HAX_SCENE_H
#include "hax_map.h"
#include "hax_grid.h"
#include "hax_camera.h"


typedef struct tagScene
{
	Map *map;
	Grid *grid;
	Camera *camera;
	COLOR bgcolor;
	WayAnimation bganimation;
	int enable_grid_animation;
} Scene;

int scene_render(Scene *s);
Scene *scene_create(char *spec);
int scene_free(Scene *s);
void scene_animate(Scene *s, float delta);

#define HAX_SCENE_H
#endif /* HAX_SCENE_H */
