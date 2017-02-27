/* dangerball, Copyright (c) 2001-2008 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#define DEFAULTS	"*showFPS:      False       \n" \

# define refresh_graphene 0
#undef countof
#define countof(x) (sizeof((x))/sizeof((*x)))

#include "xlockmore.h"
#include "graphene_scene.h"
#include <ctype.h>

#ifdef USE_GL /* whole file */

Scene *scene=NULL;
static char *cl_scene_spec;
static char *cl_spec_dumper;
static int cl_do_list;


typedef struct tagGrapheneInfo
{
	GLXContext *glx_context;
	char *scene_spec;
} GrapheneInfo;

static GrapheneInfo *graphene_info=NULL;

static XrmOptionDescRec opts[] =
{
	{ "-list", ".list", XrmoptionNoArg, "True" },
	{ "-scene", ".scene", XrmoptionSepArg, NULL },
	{ "-dump", ".dump", XrmoptionSepArg, NULL },
};

static argtype vars[] =
{
	{&cl_do_list, "list",  "List embedded scenes",  "False", t_Bool},
	{&cl_scene_spec, "scene",  "Scene specification",  "random", t_String},
	{&cl_spec_dumper, "dump",  "Dump scene specification",  "none", t_String},
};

ENTRYPOINT ModeSpecOpt graphene_opts = {countof(opts), opts, countof(vars), vars, NULL};

static double start_time=0.0;
static double last_drawn_time=0.0;

static double current_time(void)
{
	struct timeval tv;
# ifdef GETTIMEOFDAY_TWO_ARGS
	struct timezone tzp;
	gettimeofday(&tv, &tzp);
# else
	gettimeofday(&tv);
# endif

	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

ENTRYPOINT void reshape_graphene (ModeInfo *mi, int width, int height)
{
	glViewport(0, 0, width, height);
	camera_set_viewport(scene->camera,width,height);
	camera_render(scene->camera);
}

ENTRYPOINT Bool graphene_handle_event (ModeInfo *mi, XEvent *event)
{
	/*GrapheneInfo *gi=&graphene_info[MI_SCREEN(mi)];*/
	return False;
}

char *read_scene_spec(char *s, char **n);
void list_scenes(void);

ENTRYPOINT void init_graphene (ModeInfo *mi)
{

	GrapheneInfo *gi;
	char *scname=NULL;
	if (cl_do_list)
	{
		list_scenes();
		exit(0);
	};
	
	if (graphene_info==NULL)
	{
		graphene_info=(GrapheneInfo*)calloc(MI_NUM_SCREENS(mi),sizeof(GrapheneInfo));
		if (graphene_info==NULL) error_exit("out of memory");
	};

	init_fast_math();
	gi=&graphene_info[MI_SCREEN(mi)];
	gi->scene_spec=cl_scene_spec;
	gi->glx_context=init_GL(mi);

	if (!strcmp(cl_spec_dumper,"minified")) spec_dumper=SD_MINIFIED;
	if (!strcmp(cl_spec_dumper,"explained")) spec_dumper=SD_EXPLAINED;
	scene=scene_create(read_scene_spec(cl_scene_spec,&scname));
	if (spec_dumper==SD_EXPLAINED && scname!=NULL) printf("scene name %s\n",scname);
	start_time=current_time();
	reshape_graphene (mi, MI_WIDTH(mi), MI_HEIGHT(mi));
}


ENTRYPOINT void draw_graphene (ModeInfo *mi)
{
	GrapheneInfo *gi=&graphene_info[MI_SCREEN(mi)];
	Display *display = MI_DISPLAY(mi);
	Window window = MI_WINDOW(mi);
	double cur_time;

	if (!gi->glx_context) return;

	glXMakeCurrent(MI_DISPLAY(mi), MI_WINDOW(mi), *(gi->glx_context));

	cur_time=current_time()-start_time;
	scene_animate(scene,cur_time-last_drawn_time);
	scene_render(scene);
	last_drawn_time=cur_time;

	if (mi->fps_p) do_fps (mi);
	glFinish();
	glXSwapBuffers(display, window);
}

ENTRYPOINT void release_graphene (ModeInfo *mi)
{
	scene_free(scene);
	free(cl_scene_spec);
	if (graphene_info) free(graphene_info);
	graphene_info=NULL;
}

XSCREENSAVER_MODULE ("Graphene", graphene)

typedef struct tagSCENE_SPEC {
	char *name;
	char *spec;
} SCENE_SPEC;


SCENE_SPEC scenes[]={
	{"demo", "1 0x0 17 0.2 bg 1 #888 10 0 color 1 1 #fff #fff 10 0 wav 1 0 0 0 0.1 1 1 wc 1 0 0 cam 1 3 3 3 0 0 0 0 1 0 45 10 0"},
	{"1","1 0x2 48 50 1 #f000 5 0 1 1 #fff0 #ff80ffff 5 0 3 9900 0 -6550 35 160 5 0 -9435 0 5440 32 -1250 10 0 6480 0 -5480 16 -217 7 0 1 0 3200 0 0 0 0 0 0 1 90 10 0"},
	{"2","1 0x2 48 50 2 #f000 5 0 #ff242424 5 0 1 2 #ff0000a0 #ff0080ff 5 0 #ff000080 #ff80ffff 5 0 2 -3830 0 3240 12 -50 4 0 1640 0 -6680 21 -1725 5 0 1 0 3200 0 0 0 0 0 0 1 90 10 0"},
	{"4","1 0x2 48 50 2 #f000 5 0 #ff242424 5 0 1 2 #ff0000a0 #f0ff 5 0 #ff000080 #ffff 5 0 2 -7662 0 -1733 14 308 4 0 -9472 0 -2188 26 -367 7 0 1 0 3200 0 0 0 0 0 0 1 90 10 0"},
	{"above_ocean","1 0x2 52 50 1 #ff000080 5 0 1 1 #ff6c6c6c #ffc0c0c0 5 0 2 -10000:10000 0 -10000:10000 10:35 -2000:2000 2:11 0 0 0 0 20:50 300:1000 1:5 1 1 0 3200 0 0 0 0 0 0 1 80 10 0"},
	{"Acid_machine","1 0x2 30 60 1 #f000 10 0 1 1 #f0f0 #ff004000 10 0 2 0 0 0 70 200 5 0 0 0 0 20:50 300:1000 3:5 0 4 0 200 -500 0 0 0 0 0 1 90 10 0 0 400 0 0 0 0 0 0 1 90 10 0 0 200 500 0 0 0 0 0 1 90 10 0 0 400 0 0 0 0 0 0 1 90 10 0"},
	{"Ambient","1 0x2 55 50 3 #f000 10 0 #f000 10 0 #f000 10 0 1 3 #f000 #ff80ff80 10 0 #f000 #f0ff 10 0 #f000 #ffff 10 0 4 3000 0 3000 0 30 4 0 -3000 0 3000 0 30 4 0 -3000 0 -3000 10 30 4 0 3000 0 -3000 10 30 4 0 8 0 3000 0 0 0 0 0 0 1 90 10 0 0 3000 0 0 0 0 1 0 1 90 10 0 0 3000 0 0 0 0 1 0 0 90 10 0 0 3000 0 0 0 0 1 0 -1 90 10 0 0 3000 0 0 0 0 0 0 -1 90 10 0 0 3000 0 0 0 0 -1 0 -1 90 10 0 0 3000 0 0 0 0 -1 0 0 90 10 0 0 3000 0 0 0 0 -1 0 1 90 10 0"},
	{"Arabian_Pattern","1 0x0 55 50 3 #f000 10 0 #f000 10 0 #f000 10 0 11 3 #f00f #ffff80ff 10 0 #ff008000 #ff00 10 0 #ff400080 #ffff8000 10 0 4 3000 0 3000 10 10 4 0 -3000 0 3000 10 10 4 0 -3000 0 -3000 10 10 4 0 3000 0 -3000 10 10 4 0 8 0 3000 0 0 0 0 0 0 1 90 120 0 0 2000 0 0 0 0 1 0 1 120 5 0 0 1000 0 0 0 0 1 0 0 90 5 0 0 2000 0 0 0 0 1 0 -1 30 5 0 0 3000 0 0 0 0 0 0 -1 90 5 0 0 2000 0 0 0 0 -1 0 -1 120 5 0 0 1000 0 0 0 0 -1 0 0 90 5 0 0 2000 0 0 0 0 -1 0 1 30 5 0"},
	{"bio-like","1 0x2 60 10 4 #f000 10 0 #f000 10 0 #f000 10 0 #f000 10 0 11 4 #ffce0000 #ff8000ff 10 0 #ff8000ff #f0f0 10 0 #f0f0 #ffce0000 10 0 #ffce0000 #f000 10 0 3 0 0 500000 1200 600 4 0 500000 0 0 1200 600 4 0 0 0 0 1000 300:1000 1:5 0 4 0 1000 0 0 0 0 0 0 1 50:60 10 0 0 1000 0 0 0 0 1 0 0 50:60 10 0 0 1000 0 0 0 0 0 0 -1 50:60 10 0 0 1000 0 0 0 0 -1 0 0 50:60 10 0"},
	{"bi","1 0x2 52 50 1 #f000 26 0 1 1 #f000 #ffff 26 0 2 2800 0 -2800 20 600 3 0 -2800 0 2800 20 600 4 0 4 0 3400 0 0 0 0 0 0 1 80 20 0 0 3400 0 0 0 0 1 0 0 80 20 0 0 3400 0 0 0 0 0 0 -1 80 20 0 0 3400 0 0 0 0 -1 0 0 80 20 0"},
	{"carusel","1 0x2 55 50 1 #f000 26 0 1 1 #ff008000 #ffff0080 26 0 2 -4000 0 3500 20 -1000:1000 1:7 0 -4000 0 -3500 20 -1000:1000 1:7 0 1 0 3400 0 0 0 0 0 0 1 80 20 0"},
	{"Cosmic_Star","1 0x2 30 60 1 #f000 10 0 1 1 #f00f #ff000080 10 0 1 0 0 0 100 -100 10 0 10 0 2000 0 0 0 0 0 0 1 90 3 0 0 500 0 0 0 0 0 0 1 90 5 0 1000 500 0 0 0 0 0 1 0 45 10 0 2000 1000 0 0 0 0 0 0 1 90 10 0 2000 2000 2000 0 0 0 0 0 1 90 10 0 0 2000 0 0 0 0 0 0 1 90 3 0 0 500 0 0 0 0 0 0 1 90 5 0 -1000 500 0 0 0 0 0 0 1 45 10 0 -2000 1000 0 0 0 0 0 0 1 90 10 0 -2000 2000 2000 0 0 0 0 0 1 90 10 0"},
	{"Crystal_Blaze","1 0x2 55 50 3 #f000 10 0 #f000 10 0 #f000 10 0 1 3 #f000 #f0ff 10 0 #f000 #ff0f 10 0 #f000 #ff9e3eff 10 0 4 3000 0 3000 0 10 1 0 -3000 0 3000 0 10 1 0 -3000 0 -3000 50 10 1 0 3000 0 -3000 50 10 1 0 8 0 3000 0 0 0 0 0 0 1 90 5 0 0 2000 0 0 0 0 1 0 1 90 5 0 0 3000 0 0 0 0 1 0 0 90 5 0 0 2000 0 0 0 0 1 0 -1 90 5 0 0 3000 0 0 0 0 0 0 -1 90 5 0 0 2000 0 0 0 0 -1 0 -1 90 5 0 0 3000 0 0 0 0 -1 0 0 90 5 0 0 2000 0 0 0 0 -1 0 1 90 5 0"},
	{"curves","1 0x0 55 50 1 #ff000080 10 0 11 1 #f000 #ffff 10 0 2 -3000:3000 0 -3000:3000 10 1000:2000 1:4 0 0 0 0 20:50 300:1000 1:5 1 8 0 3000 0 0 0 0 0 0 1 90 30 0 0 3000 0 0 0 0 0 0 1 90 1 0 0 2000 0 0 0 0 1 0 0 90 30 0 0 2000 0 0 0 0 1 0 0 90 1 0 0 2500 0 0 0 0 0 0 1 90 30 0 0 2500 0 0 0 0 0 0 1 90 1 0 1000 2000 0 1000 0 0 -1 0 0 90 30 0 1000 2000 0 1000 0 0 -1 0 0 90 1 0"},
	{"default","1 0x2 30 60 1 #f000 10 0 1 1 #f000 #ffff 10 0 1 0 0 0 50 300 3 0 1 0 1000 0 0 0 0 0 0 1 90 5 0"},
	{"discharge","1 0x2 20 60 1 #f000 10 0 1 1 #f0ff #ffff 10 0 4 0 0 0 100 11 0:2 0 0 0 0 20:50 13 0 0 30000 0 0 20 500 1 0 0 0 100000 50 2000 2 0 1 2900 0 120 0 0 0 0 1 0 88 10 0"},
	{"Disco","1 0x2 55 50 4 #f000 10 0 #f000 10 0 #f000 10 0 #f000 10 0 1 4 #f000 #f0f0 10 0 #f000 #f0ff 10 0 #f000 #fff0 10 0 #f000 #ff00 10 0 4 3000 0 3000 1 1000:2000 1:4 0 -3000 0 3000 1 1000:2000 1:4 0 -3000 0 -3000 1 1000:2000 1:4 0 3000 0 -3000 1 1000:2000 1:4 0 8 0 3200 0 0 0 0 0 0 1 90 1 0 0 3000 0 0 0 0 0 0 1 90 1 0 0 2000 0 0 0 0 1 0 0 90 1 0 0 2000 0 0 0 0 1 0 0 90 1 0 0 2500 0 0 0 0 0 0 1 90 1 0 0 2500 0 0 0 0 0 0 1 90 1 0 1000 2000 0 1000 0 0 -1 0 0 90 1 0 1000 2000 0 1000 0 0 -1 0 0 90 1 0"},
	{"Dive_Blood","1 0x2 60 50 2 #f000 5 0 #f000 1 0 1 2 #ff804040 #ff00 5 0 #ff800000 #ff00 1 0 3 0:1000 0:1000 0:1000 20:50 300:1000 1:5 0 0:200 0:500 0:1000 0:100 300:1000 1:5 0 0 0 1000 50 300:1000 1:5 0 4 0 1000 0 0 0 0 0 0 1 90 10 0 0 4000 0 0 0 1 0 0 1 90 5 0 500 300 0 0 1 0 0 0 1 45 10 0 500 500 0 0 0 0 0 1 0 45 10 0"},
	{"DragonScale","1 0x2 48 50 2 #f000 5 0 #ff242424 5 0 1 2 #ff0000a0 #ff0080ff 5 0 #ff000080 #ffff 5 0 3 8122 0 -8060 31 10 8 0 9935 0 4436 20 -635 3 0 1274 0 -7370 31 -91 3 0 1 0 3000 0 0 0 0 0 0 1 80 10 0"},
	{"dragons_eye","1 0x2 25 40 5 #ff202020 20 0 #ff404040 20 0 #ff202020 20 0 #ff202020 20 0 #ff202020 10 0 11 5 #ffff #fff0 20 0 #ffff8000 #ffff 20 0 #ffff8000 #ff00 20 0 #ffff #ff00 20 0 #ff00 #f0ff 10 0 2 -1000:1000 0 -1000:1000 150 -200:200 4:8 0 0 0 0 20:50 300:1000 1:5 10 1 0 1500 0 0 0 0 0 0 1 130 10 0"},
	{"drops","1 0x2 52 50 7 #f000 26 0 #ff008040 26 0 #ff800000 26 0 #ffff8000 26 0 #ff0080ff 26 0 #ff004080 26 0 #ffff8040 26 0 11 7 #f000 #fff0 26 0 #ff008040 #fff0 26 0 #ff800000 #fff0 26 0 #ffff8000 #fff0 26 0 #ff0080ff #fff0 26 0 #ff004080 #fff0 26 0 #ffff8040 #fff0 26 0 4 1000:3000 0 -3000:3000 20 600 3 0 -2000:-3000 0 -3000:3000 20 600 3 0 -3000:3000 0 1300:2800 20 600 3 0 -3000:3000 0 -2700:-1700 20 600 3 0 1 0 3000 0 0 0 0 0 0 1 83 10 0"},
	{"dunes","1 0x0 48 50 1 #ff404040 5 0 1 1 #ff6d631b #ffb1a654 5 0 3 10698 0 -2699 25 425 17 0 14403 0 -1799 27 600 19 0 7865 0 -7627 25 650 23 0 1 -2500 300:800 -4000 0 0 0 0 1 0 90 10 0"},
	{"fignja_kakaja-to","1 0x2 54 20 3 #f000 5 0 #f000 5 0 #ff000080 5 0 1 3 #ff000040 #ff80ffff 5 0 #ff000040 #ff0080ff 5 0 #ff000040 #f00f 5 0 4 0 0 500000 400 -600 3 0 0 0 -2000 50 -600 10000 0 0 0 0 3000 300:1000 1:5 0 0 0 0 20:50 300:1000 1:5 1 4 500 1500 0 0 0 0 0 0 1 80 3 0 500 1000 0 0 0 0 0 0 1 80 3 0 500 1500 0 0 0 0 1 0 1 80 3 0 500 1000 0 0 0 0 0 0 1 80 3 0"},
	{"Fireworks","1 0x2 20 50 4 #f000 5 0 #f000 5 0 #f000 5 0 #f000 5 0 1 4 #fff0 #ffff8000 5 0 #ff0f #f0f0 5 0 #ff0080ff #ff0f 5 0 #ffff8000 #f00f 5 0 2 -1000:1000 0 -1000:1000 150 -200:200 4:8 0 0 0 0 20:50 300:1000 1:5 10 2 0 1800 0 0 0 0 0 0 1 130 10 0 0 2700 0 0 0 0 0 0 1 130 10 0"},
	{"flying","1 0x2 48 50 1 #ff404040 5 0 1 1 #ff6c6c6c #ffc0c0c0 5 0 2 -2000:2000 0 -2000:2000 10:35 200:2000 2:11 0 0 0 0 20:50 300:1000 1:5 1 5 -2000:2000 500:2200 -2000:2000 -1000:1000 100 -1000:1000 0 1 0 90 20:35 0 0 4000 0 0 0 0 0 0 1 90 10 10 -2222 400 0 0 400 0 0 1 0 140 10:20 0 0 400 2222 -1000:1000 400 0 0 1 0 140 13 0 2222 400 0 0 400 0 0 1 0 140 18 0"},
	{"Grass","1 0x2 48 50 2 #f000 5 0 #f000 5 0 1 2 #ff000080 #fff0 5 0 #ff000080 #f0f0 5 0 3 5698 0 -2699 25 -425 6 0 9403 0 -1799 27 -600 6 0 2865 0 -7627 25 -650 4 0 1 0 3000 0 0 0 0 0 0 1 80 10 0"},
	{"Hyper","1 0x2 55 50 3 #f000 10 0 #f000 10 0 #f000 10 0 1 3 #f00f #ffff80ff 10 0 #ff008000 #ff00 10 0 #ff400080 #ffff8000 10 0 4 3000 0 3000 10000 1000:2000 1:4 0 -3000 0 3000 10000 1000:2000 1:4 0 -3000 0 -3000 10000 1000:2000 1:4 0 3000 0 -3000 10000 1000:2000 1:4 0 8 0 3000 0 0 0 0 0 0 1 90 5 0 0 2000 0 0 0 0 1 0 1 120 5 0 0 1000 0 0 0 0 1 0 0 90 5 0 0 2000 0 0 0 0 1 0 -1 30 5 0 0 3000 0 0 0 0 0 0 -1 90 5 0 0 2000 0 0 0 0 -1 0 -1 120 5 0 0 1000 0 0 0 0 -1 0 0 90 5 0 0 2000 0 0 0 0 -1 0 1 30 5 0"},
	{"hypno","1 0x2 45 50 8 #ff404040 28 0 #ff202020 28 0 #ff202020 28 0 #ff202020 28 0 #ff202020 28 0 #ff202020 28 0 #ff202020 28 0 #ff202020 28 0 1 8 #ff8080ff #ffff0080 28 0 #ffff8000 #ff00ff40 28 0 #ff00 #fff0 28 0 #f00f #f0ff 28 0 #ff804000 #ffff0080 28 0 #ff0080c0 #ffff8000 28 0 #f00f #fff0 28 0 #ff00ff40 #ff00 28 0 1 0 0 0 10 -500 1 0 2 0 2800 0 0 0 0 0 0 1 80 50 0 0 1700 0 0 0 0 0 0 1 80 50 0"},
	{"Ice_Wind","1 0x2 48 45 1 #ff000040 5 0 1 1 #ff0080ff #ffff 5 0 3 9900 0 -6550 100 160 5 0 -9435 0 5440 50 -1250 10 0 0 0 0 50:100 300:1000 1:5 0 1 0 3200 0 0 0 0 0 0 1 90 10 0"},
	{"kaleidoscope","1 0x2 52 50 4 #ff800000 30 0 #ff800000 2 0 #ff800000 20 0 #ff800000 2 0 11 4 #ff800000 #fff0 30 0 #ff800000 #fff0 2 0 #ff5e0000 #fff0 20 0 #ff5e0000 #fff0 2 0 6 2000 0 0 20 700 3 0 -1000 0 1732 20 700 3 0 -1000 0 -1732 20 700 3 0 1000 0 1732 20 700 4 0 -2000 0 0 20 700 4 0 1000 0 -1732 20 700 4 0 4 0 3000 0 0 0 0 0 0 1 83 90 0 0 3000 0 0 0 0 1 0 0 83 90 0 0 3000 0 0 0 0 0 0 -1 83 90 0 0 3000 0 0 0 0 -1 0 0 83 90 0"},
	{"Mask","1 0x0 55 45 7 #ff141414 10 0 #ff141414 10 0 #ff141414 10 0 #ff141414 10 0 #ff141414 10 0 #ff141414 10 0 #ff141414 10 0 1 7 #f000 #ff00 10 0 #f000 #ffff8000 10 0 #f000 #fff0 10 0 #f000 #f0f0 10 0 #f000 #f0ff 10 0 #f000 #f00f 10 0 #f000 #ff8000ff 10 0 3 0 0 3000 20 40 8 0 2000 0 -2500 40 40 5 0 -2000 0 -2500 40 40 5 0 1 0 3000 0 0 0 0 0 0 1 90 120 0"},
	{"matrix_q","1 0x2 55 50 1 #ff202020 20 0 1 1 #ff004800 #f0f0 20 0 1 0 0 0 20 0:1 5 0 1 0 3000 0 0 0 0 0 0 1 88 10 0"},
	{"medusa","1 0x2 40 60 1 #ff27004f 26 0 1 1 #ff008000 #ff8080ff 26 0 2 0 0 0 300 800 8 0 0 0 0 250 1600 8 0 1 0 3800 0 0 0 0 0 0 1 120 20 0"},
	{"micro2","1 0x2 55 50 1 #ffff 10 0 11 1 #ffff #f000 10 0 4 -3000:3000 0 -3000:3000 1 1000:2000 4:12 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 4 0 3000 0 0 0 0 0 0 1 90 30 0 0 3000 0 0 0 0 0 0 1 90 1 0 0 2000 0 0 0 0 1 0 0 90 30 0 0 2000 0 0 0 0 1 0 0 90 1 0"},
	{"micro","1 0x2 55 50 1 #f000 40 0 11 1 #f000 #ffff 40 0 4 -3000:3000 0 -3000:3000 1 1000:2000 4:12 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 4 0 3000 0 0 0 0 0 0 1 90 30 0 0 3000 0 0 0 0 0 0 1 90 1 0 0 2000 0 0 0 0 1 0 0 90 30 0 0 2000 0 0 0 0 1 0 0 90 1 0"},
	{"Mist","1 0x2 48 50 2 #f000 5 0 #ff202020 5 0 1 2 #ff000080 #f0ff 5 0 #ff000080 #ffff 5 0 3 2390 0 9198 28 1353 3 0 -7295 0 5656 18 1500 5 0 -3125 0 1742 16 -947 3 0 1 0 3000 0 0 0 0 0 0 1 80 10 0"},
	{"mountains","1 0x1 70 80 1 #ff363636 26 0 1 1 #ff006200 #ffff 26 0 6 80000 0 80000 1000 8000:12000 18000 0 80000 0 -80000 800 8000:12000 20000 0 -5000:5000 0 -5000:5000 30:80 1000:2000 10000 0 0 0 0 20:50 300:1000 1:5 2 -5000:5000 0 -5000:5000 20:50 50:800 10020 0 0 0 0 20:50 300:1000 1:5 2 3 9000 2500 0 0 0 0 0 1 0 90 20 0 0 2500 -9000 0 0 0 0 1 0 90 20 0 -9000 2500 0 0 0 0 0 1 0 90 20 0"},
	{"neon","1 0x2 54 20 3 #f000 5 0 #f000 5 0 #f000 5 0 1 3 #f000 #f0f0 5 0 #f000 #fff0 5 0 #f000 #ffff 5 0 3 0 0 500000 400 -600 3 0 0 0 -2000 50 -600 10000 0 0 0 0 3000 300:1000 1:5 0 4 500 1500 0 0 0 0 0 0 1 80 3 0 500 1000 0 0 0 0 0 0 1 80 3 0 500 1500 0 0 0 0 0 0 1 80 3 0 500 1000 0 0 0 0 0 0 1 80 3 0"},
	{"noise","1 0x2 30 50 1 #ff404040 20 0 1 1 #ff404040 #ffff 20 0 2 -10000:10000 0 -10000:10000 10:100 -200:200 4:8 0 0 0 0 20:50 300:1000 1:5 3 1 -1000:1000 2000 -1000:-2000 0 0 0 0 0 1 90 10 0"},
	{"North_shine","1 0x2 35 60 3 #f000 10 0 #f000 10 0 #f000 10 0 1 3 #ffffff80 #ff000080 10 0 #f0f0 #ff00 10 0 #ffff8000 #ff800080 10 0 3 0 0 0 50 300 3 0 0 0 0 100 400 2:3 0 1000 0 0 50 800 2:3 0 7 0 1000 -1000 0 0 0 0 0 1 90 5 0 0 400 -1000 0 0 0 0 0 1 90 5 0 0 1000 0 0 0 0 0 0 1 90 10 0 0 400 1000 0 0 0 1 0 0 90 10 0 0 400 1000 0 0 0 1 1 0 90 5 0 0 1000 1000 0 0 0 1 1 0 90 5 0 0 1000 0 0 0 0 1 1 0 90 5 0"},
	{"Oceanic_gazer","1 0x2 40 60 4 #f000 4 0 #f000 2 0 #f000 4 0 #f000 4 0 1 4 #ff0080ff #ff000080 4 0 #ff80ffff #ff000080 2 0 #ff0080ff #ff000080 4 0 #ffc683fc #ff000080 4 0 3 0 0 0 50 300 3 0 0 0 0 100 400 2:4 0 1000 0 0 50 800 2:4 0 4 0 1500 -1000 0 0 0 0 0 1 90 10 0 1000 300 0 0 0 0 0 1 0 90 10 0 0 1500 1000 0 0 0 0 1 0 90 10 0 -1000 300 0 0 0 0 1 1 0 90 10 0"},
	{"patterns","1 0x2 52 50 6 #ff202020 20 0 #ff202020 2 0 #ff202020 20 0 #ff202020 2 0 #ff202020 20 0 #ff202020 2 0 11 6 #ff006200 #f0f0 20 0 #ff006200 #f0f0 2 0 #ff802900 #ffff8000 20 0 #ff802900 #ffff8000 2 0 #ff202020 #ffff 20 0 #ff202020 #ffff 2 0 1 0 0 0 20 1:150 5 0 1 0 3000 0 0 0 0 0 0 1 84 10 0"},
	{"proba01","1 0x2 55 9 1 #f000 5 0 1 1 #f000 #ffff 5 0 2 500 0 500 150 400 5 0 0 0 0 250 200 4 0 1 0 1300 -200 0 0 0 0 0 1 90 10 0"},
	{"proba02","1 0x2 55 80 1 #ff363636 26 0 1 1 #ff006200 #ffff 26 0 6 80000 0 80000 1000 8000:12000 18000 0 80000 0 -80000 800 8000:12000 20000 0 -5000:5000 0 -5000:5000 30:80 1000:2000 10000 0 0 0 0 20:50 300:1000 1:5 2 -5000:5000 0 -5000:5000 20:50 50:800 10020 0 0 0 0 20:50 300:1000 1:5 2 3 8000 2500 0 0 0 0 0 1 0 80 20 0 0 2500 -8000 0 0 0 0 1 0 90 20 0 -8000 2500 0 0 0 0 0 1 0 90 20 0"},
	{"rays","1 0x2 53 50 1 #ff000080 10 0 1 1 #f000 #ffff 10 0 6 800 0 200 0 8 3 0 800 0 200 20 8 4 0 -700 0 -200 20 8 5 0 -700 0 -200 0 8 6 0 -2000 0 100 20 9 7 0 -2000 0 100 0 9 8 0 1 0 3000 0 0 0 0 0 0 1 86 50 0"},
	{"ribs","1 0x2 48 50 2 #f000 5 0 #ff202020 5 0 1 2 #ff0000a0 #ffff8080 5 0 #ff000080 #ff00 5 0 3 -7662 0 -1733 14 308 4 0 -9472 0 -2188 26 -367 7 0 9400 0 2200 30 -400 6 0 1 0 2000 0 0 0 0 0 0 1 90 10 0"},
	{"roaming","1 0x2 25 60 3 #ff210042 26 0 #ff202020 26 0 #ff002346 26 0 1 3 #ff008000 #ff8080ff 26 0 #ff004080 #f0ff 26 0 #ff000080 #ffff 26 0 1 0 0 0 40 10:60 8 0 5 0 400 0 0 0 0 0 0 1 120 42 0 1000 450 0 1000 0 0 0 0 1 120 42 0 1000 300 1000 1000 0 1000 1 0 0 120 42 0 -1100 300 1000 -1100 0 1000 0 0 -1 120 42 0 -1000 500 -1000 -1000 0 -1000 -1 0 0 120 42 0"},
	{"Rose2","1 0x2 55 9 2 #f000 5 0 #f000 5 0 1 2 #ff800000 #ffff0080 5 0 #ff800000 #ffff8080 5 0 2 500 0 500 150 400 5 0 0 0 0 250 200 4 0 4 0 1500 -200 0 0 0 0 0 1 90 10 0 -200 1600 0 0 0 0 1 0 0 90 10 0 0 1800 200 0 0 0 0 0 -1 90 10 0 200 1300 0 0 0 0 -1 0 0 90 10 0"},
	{"Rose","1 0x2 50 60 2 #f000 5 0 #f000 5 0 1 2 #ff800000 #ffff0080 5 0 #ff800000 #ffff8080 5 0 2 0 0 0 150 500 5 0 500 0 500 250 375 4 0 4 0 3000 0 0 0 0 0 0 1 90 10 0 500 1000 0 0 0 0 0 1 0 90 10 0 1000 3000 0 0 0 0 0 1 0 90 10 0 500 1000 0 0 0 0 0 0 1 90 10 0"},
	{"seasons","1 0x2 30 50 4 #ff404040 10 0 #ff404040 10 0 #ff404040 10 0 #ff404040 10 0 11 4 #f000 #ff80ff80 10 0 #f000 #fffacd45 10 0 #f000 #ffff 10 0 #f000 #ff804000 10 0 2 -10000:10000 0 -10000:10000 10:100 -200:200 4:8 0 0 0 0 20:50 300:1000 1:5 3 1 -1000:1000 2000 -1000:-2000 0 0 0 0 0 1 90 10 0"},
	{"Silk","1 0x2 52 50 1 #ff2c2c2c 5 0 1 1 #ff000080 #ff8ac5ff 5 0 3 -1310 0 4519 34 -1254 2 0 -8142 0 7643 12 -550 4 0 -3953 0 8941 30 811 8 0 1 0 3000 0 0 0 0 0 0 1 85 10 0"},
	{"Smaragd","1 0x2 40 60 3 #ff004000 10 0 #f000 10 0 #ff004000 10 0 1 3 #ff80ff00 #fff0 10 0 #fff0 #ffffff80 10 0 #fff0 #ff80ff00 10 0 1 0 0 0 200 -500 10 0 2 0 3000 0 0 0 0 0 0 1 82 10 0 0 2000 0 0 0 0 0 0 1 82 10 0"},
	{"sonar","1 0x2 54 50 1 #f000 26 0 11 1 #f000 #f0f0 26 0 2 0 0 500000 20 600 3 0 0 0 -2000 20 600 10000 0 1 0 3400 0 0 0 0 0 0 1 80 20 0"},
	{"spots2","1 0x2 52 50 7 #f000 26 0 #ff008040 26 0 #ff800000 26 0 #ffff8000 26 0 #ff0080ff 26 0 #ff004080 26 0 #ffff8040 26 0 11 7 #f000 #fff0 26 0 #ff008040 #fff0 26 0 #ff800000 #fff0 26 0 #ffff8000 #fff0 26 0 #ff0080ff #fff0 26 0 #ff004080 #fff0 26 0 #ffff8040 #fff0 26 0 4 2000 0 0 20 600 3 0 -2000 0 0 20 600 3 0 0 0 2000 20 600 3 0 0 0 -2000 20 600 3 0 1 0 3000 0 0 0 0 0 0 1 83 10 0"},
	{"spots3","1 0x2 52 50 7 #f000 26 0 #ff008040 26 0 #ff800000 26 0 #ffff8000 26 0 #ff0080ff 26 0 #ff004080 26 0 #ffff8040 26 0 11 7 #f000 #fff0 26 0 #ff008040 #fff0 26 0 #ff800000 #fff0 26 0 #ffff8000 #fff0 26 0 #ff0080ff #fff0 26 0 #ff004080 #fff0 26 0 #ffff8040 #fff0 26 0 3 1000 0 0 20 600 3 0 -500 0 866 20 600 3 0 -500 0 -866 20 600 3 0 4 0 3000 0 0 0 0 0 0 1 83 120 0 0 3000 0 0 0 0 1 0 0 83 120 0 0 3000 0 0 0 0 0 0 -1 83 120 0 0 3000 0 0 0 0 -1 0 0 83 120 0"},
	{"spots6","1 0x2 52 50 7 #f000 26 0 #ff008040 26 0 #ff800000 26 0 #ffff8000 26 0 #ff0080ff 26 0 #ff004080 26 0 #ffff8040 26 0 11 7 #f000 #fff0 26 0 #ff008040 #fff0 26 0 #ff800000 #fff0 26 0 #ffff8000 #fff0 26 0 #ff0080ff #fff0 26 0 #ff004080 #fff0 26 0 #ffff8040 #fff0 26 0 6 2000 0 0 20 600 3 0 -1000 0 1732 20 600 3 0 -1000 0 -1732 20 600 3 0 1000 0 1732 20 600 3 0 -2000 0 0 20 600 3 0 1000 0 -1732 20 600 3 0 1 0 3000 0 0 0 0 0 0 1 83 10 0"},
	{"spots","1 0x2 52 50 7 #f000 26 0 #ff008040 26 0 #ff800000 26 0 #ffff8000 26 0 #ff0080ff 26 0 #ff004080 26 0 #ffff8040 26 0 1 7 #f000 #fff0 26 0 #f000 #fff0 26 0 #f000 #fff0 26 0 #f000 #fff0 26 0 #f000 #fff0 26 0 #f000 #fff0 26 0 #f000 #fff0 26 0 4 2000 0 0 20 600 3 0 -2000 0 0 20 600 3 0 0 0 2000 20 600 3 0 0 0 -2000 20 600 3 0 1 0 3000 0 0 0 0 0 0 1 83 10 0"},
	{"squares2","1 0x2 55 50 1 #ff400000 26 0 1 1 #ff8c2e00 #ffff 26 0 2 0 0 500000 50 -600 -5:5 0 500000 0 0 50 -600 -5:5 0 1 0 3400 0 0 0 0 0 0 1 80 120 0"},
	{"squares","1 0x2 55 50 1 #ff004000 26 0 11 1 #f000 #ffff 26 0 2 0 0 500000 50 -600 -5:5 0 500000 0 0 50 -600 -5:5 0 1 0 3400 0 0 0 0 0 0 1 80 120 0"},
	{"strange_thing","1 0x2 25 40 5 #ff202020 10 0 #ff404040 10 0 #ff202020 10 0 #ff202020 10 0 #ff202020 10 0 1 5 #ffff #fff0 10 0 #ffff8000 #ffff 10 0 #ffff8000 #ff00 10 0 #ffff #ff00 10 0 #ff00 #f0ff 10 0 2 -1000:1000 0 -1000:1000 150 -200:200 4:8 0 0 0 0 20:50 300:1000 1:5 10 1 0 1500 0 0 0 0 0 0 1 130 10 0"},
	{"test2","1 0x0 55 45 3 #f000 10 0 #f000 10 0 #f000 10 0 1 3 #f000 #ffff80ff 10 0 #f000 #ff00ff40 10 0 #f000 #f0ff 10 0 3 0 0 3000 20 40 8 0 2000 0 -2500 40 40 5 0 -2000 0 -2500 40 40 5 0 8 0 3000 0 0 0 0 0 0 1 90 120 0 0 2000 0 0 0 0 1 0 1 120 5 0 0 1000 0 0 0 0 1 0 0 90 5 0 0 2000 0 0 0 0 1 0 -1 30 5 0 0 3000 0 0 0 0 0 0 -1 90 5 0 0 2000 0 0 0 0 -1 0 -1 120 5 0 0 1000 0 0 0 0 -1 0 0 90 5 0 0 2000 0 0 0 0 -1 0 1 30 5 0"},
	{"test","1 0x2 52 50 4 #ff800000 10 0 #ff800000 2 0 #ff800000 10 0 #ff800000 2 0 11 4 #ff800000 #fff0 10 0 #ff800000 #fff0 2 0 #ff800000 #ffff 10 0 #ff800000 #ffff 2 0 6 2000 0 0 20 700 3 0 -1000 0 1732 20 700 3 0 -1000 0 -1732 20 700 3 0 1000 0 1732 20 700 4 0 -2000 0 0 20 700 4 0 1000 0 -1732 20 700 4 0 4 0 3000 0 0 0 0 0 0 1 83 90 0 0 3000 0 0 0 0 1 0 0 83 90 0 0 3000 0 0 0 0 0 0 -1 83 90 0 0 3000 0 0 0 0 -1 0 0 83 90 0"},
	{"two_circles","1 0x2 55 50 1 #ff000080 10 0 1 1 #f000 #ffff 10 0 2 -3000:3000 0 -3000:3000 1 1000:2000 1:4 0 0 0 0 20:50 300:1000 1:5 1 8 0 3000 0 0 0 0 0 0 1 90 30 0 0 3000 0 0 0 0 0 0 1 90 1 0 0 2000 0 0 0 0 1 0 0 90 30 0 0 2000 0 0 0 0 1 0 0 90 1 0 0 2500 0 0 0 0 0 0 1 90 30 0 0 2500 0 0 0 0 0 0 1 90 1 0 1000 2000 0 1000 0 0 -1 0 0 90 30 0 1000 2000 0 1000 0 0 -1 0 0 90 1 0"},
	{"Under_Water","1 0x2 55 55 2 #f000 5 0 #ff242424 5 0 1 2 #ff0000a0 #ff0080ff 5 0 #ff000080 #ff80ffff 5 0 6 -3830 0 3240 12 -50 4 0 0 0 0 20:50 300:1000 1:5 1 1640 0 -6680 21 -1725 5 0 0 0 0 20:50 300:1000 1:5 1 0 0 0 20:50 300:1000 1:5 0 0 0 0 20:50 300:1000 1:5 1 1 0 3200 0 0 0 0 0 0 1 90 10 0"},
	{"virus_1","1 0x0 60 10 1 #f000 26 0 1 1 #f00f #ffff8000 26 0 2 0 0 500000 1200 600 4 0 500000 0 0 1200 600 4 0 4 0 3400 0 0 0 0 0 0 1 50:60 10 0 0 2400 0 0 0 0 1 0 0 50:60 10 0 0 3400 0 0 0 0 0 0 -1 50:60 10 0 0 2400 0 0 0 0 -1 0 0 50:60 10 0"},
	{"virus","1 0x2 50 10 1 #ff000053 26 0 1 1 #ff808080 #ffff8000 26 0 2 0 0 500000 1000 -600 4 0 500000 0 0 1000 -600 4 0 4 0 3400 0 0 0 0 0 0 1 50:60 120 0 0 3400 0 0 0 0 1 0 0 50:60 120 0 0 3400 0 0 0 0 0 0 -1 50:60 120 0 0 3400 0 0 0 0 -1 0 0 50:60 120 0"},
	{"water","1 0x2 48 50 1 #ff404040 5 0 1 1 #ff6c6c6c #ffc0c0c0 5 0 2 -10000:10000 0 -10000:10000 10:35 200:900 1:20 0 0 0 0 20:50 300:1000 1:5 1 4 -3000 500 0 0 200 0 0 1 0 90 60 0 0 500 -3000 0 200 0 0 1 0 90 60 0 3000 500 0 0 200 0 0 1 0 90 60 0 0 500 3000 0 200 0 0 1 0 90 60 0"},
};

char *read_scene_spec(char *s, char **n)
{
	int i=0;

	if (!strcmp(s,"random"))
	{
		i=(sizeof(scenes)/sizeof(scenes[0]))*(long int)random()/RAND_MAX;
		if (n) *n=scenes[i].name;
		return scenes[i].spec;
	};

	for (i=0;i<sizeof(scenes)/sizeof(scenes[0]);i++)
	{
		if (strcmp(scenes[i].name,s)) continue;
		if (n) *n=scenes[i].name;
		return scenes[i].spec;
	};
	return s;
}

void list_scenes(void)
{
	int i=0;
	for (i=0;i<sizeof(scenes)/sizeof(scenes[0]);i++)
	{
		printf("%s\n",scenes[i].name);
	};
}

#endif /* USE_GL */
