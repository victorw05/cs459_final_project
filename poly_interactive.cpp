/*
 * poly_interactive.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: Victor
 */

/**
 * Menu Options:
 * 1) Rendering Modes
 * 		a) As Points
 * 		b) As Lines
 * 		c) As Fill
 * 		d) As Lines and Fill
 * 2) Brother Blender
 * 		a) Black
 * 		b) White
 * 3) Monkey Blender
 * 		a) White
 * 		b) Read
 * 4) Mesh Sample
 * 		a) Metal
 * 		b) Glass
 * 		c) Fabric
 * 5) Room Walls
 * 		a) Stucco
 * 		b) Dry Wall
 * 		c) Brick
 * 6) Shading
 * 		a) Smooth
 * 		b) Flat
 * 7) Lamp
 * 		a) ON
 * 		b) OFF
 * 8) Spotlight
 * 		a) ON
 * 		b) OFF
 * 9) Upper Light
 * 		a) White
 * 		b) Orange
 * 10) Front Light
 * 		a) White
 * 		b) Turquoise
 * 11) Origin
 * 		a) Hidden
 * 		b) Invisible
 * 12) Rotate While Idle
 * 13) Exit
 *
 * Expected Mesh Files: inputmesh_sample.off,
 *
 * 3D Geometric Transformations:
 * 'r' + left mouse drag = rotate
 * 'r' + 'z' + left mouse drag = rotate z-axis
 * 's' + left mouse drag = scale
 * 't' + left mouse drag = translate
 * 't' + 'z' + left mouse drag = translate z-axis
 * '0' = reset all geometric transformations
 * '1' (+ 'z') + left mouse drag = translate upper light (spotlight facing down)
 * '2' (+ 'z') + left mouse drag = translate front light (point light)
 *
 */

#ifdef _WIN32
#include <GL/glut.h>
#pragma warning(disable:4996)
// (or others, depending on the system in use)
#else
#include <GLUT/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <memory.h>

#include <map>
#include <iostream>
#include <cassert>

#define PI 3.14159265

#define HEIGHT 800
#define WIDTH 1200

void myMenu(int value);

static int EXIT_APP = 0;
static int POLYGON_MODE_POINT = 1;
static int POLYGON_MODE_LINE = 2;
static int POLYGON_MODE_FILL = 3;
static int POLYGON_MODE_LINE_FILL = 4;
static int OPTION_ROTATE_IDLE = 7;

static int MESH_BROTHER_BLENDER_BLACK = 9;
static int MESH_BROTHER_BLENDER_WHITE = 8;

static int MESH_MONKEY_BLENDER_BLACK = 10;
static int MESH_MONKEY_BLENDER_WHITE = 11;

static int MESH_SAMPLE_METAL = 6;
static int MESH_SAMPLE_GLASS = 12;
static int MESH_SAMPLE_FABRIC = 13;

static int ROOM_WALLS_STUCCO = 14;
static int ROOM_WALLS_BRICK = 15;
static int ROOM_WALLS_DRY_WALLS = 16;

static int FLAT_SHADING = 19;
static int SMOOTH_SHADING = 18;

static int POINT_LIGHT_ON = 20;
static int POINT_LIGHT_OFF = 21;

static int SPOT_LIGHT_ON = 22;
static int SPOT_LIGHT_OFF = 23;

static int UPPER_LIGHT_WHITE = 24;
static int UPPER_LIGHT_ORANGE = 25;

static int FRONT_LIGHT_WHITE = 26;
static int FRONT_LIGHT_TURQUOISE = 27;

static int ORIGIN_VISIBLE = 28;
static int ORIGIN_HIDDEN = 29;

int _polygon_render_mode = POLYGON_MODE_FILL;
int _mesh_brother_color = MESH_BROTHER_BLENDER_BLACK;
int _mesh_monkey_color = MESH_MONKEY_BLENDER_WHITE;
int _mesh_sample_mat = MESH_SAMPLE_METAL;
int _walls_mode = ROOM_WALLS_STUCCO;
int _windowID;
int _shading_model = SMOOTH_SHADING;
int _pointLightState = POINT_LIGHT_ON;
int _spotLightState = SPOT_LIGHT_ON;
int _upper_light_color = UPPER_LIGHT_WHITE;
int _front_light_color = FRONT_LIGHT_WHITE;
int _origin_visibility = ORIGIN_HIDDEN;

static int menu_all;
int subOption;

static int TRANSFORM_SCALE = 1;
static int TRANSFORM_TRANSLATE = 2;
static int TRANSFORM_ROTATE = 3;
static int TRANSFORM_LIGHT1_TRANSLATE = 4;
static int TRANSFORM_LIGHT0_TRANSLATE = 5;

int _transform_current = 0;
int _mesh_current = 5;
bool _idle_rotate_current = false;

int _current_height = HEIGHT;
int _current_width = WIDTH;

float _current_fov = 75.0;

bool _z_axis_enabled = false;

float _light1_pos[4] = { 0.0, 20.0, 0.0, 1.0 };
float _light0_pos[4] = { 5.0, 5.0, 40.0, 1.0 };

typedef struct {
	float x;
	float y;
	float z;
} FLTVECT;

typedef struct {
	FLTVECT p1;
	FLTVECT p2;
	FLTVECT p3;
} TRIANGLE;

typedef struct {
	float x;
	float y;
	float z;
	GLfloat* normal;
	int connected_count;
} FLTVECTPLUS;

typedef struct {
	FLTVECTPLUS* p1;
	FLTVECTPLUS* p2;
	FLTVECTPLUS* p3;
	GLfloat* normal;
} TRIANGLEPLUS;

typedef struct {
	int a;
	int b;
	int c;
} INT3VECT;

typedef struct {
	int a;
	int b;
	int c;
	GLfloat* normal;
} INT3VECTPLUS;

typedef struct {
	int nv;
	int nf;
	FLTVECTPLUS *vertex;
	INT3VECTPLUS *face;
} SurFaceMesh;

typedef struct {
	int count;
	TRIANGLEPLUS * list;
} RawMesh;

SurFaceMesh * _surfmesh;

RawMesh * _brother_blender_mesh;
RawMesh * _blender_monkey_mesh;
RawMesh * _room_walls_mesh;
RawMesh * _scene_mesh;
RawMesh * _tables_mesh;
RawMesh * _lamp_bases_mesh;
RawMesh * _lamp_point_mesh;
RawMesh * _lamp_spotlight_mesh;

bool _fullscreen = false;
bool _mouseDown = false;

float _xtransform = 0.0f;
float _ytransform = 0.0f;

float _xdiff_rotate = 0.0f;
float _ydiff_rotate = 0.0f;
float _zdiff_rotate = 0.0f;
float _xdiff_translate = 0.0f;
float _ydiff_translate = 0.0f;
float _zdiff_translate = 0.0f;
float _xdiff_scale = 0.0f;
float _ydiff_scale = 0.0f;
float _radius_scale = 0.0f;
float _radius_orig = 0.0f;
float _radius_diff_scale = 1.0f;

float _xgrid_mouse = 0.0f;
float _ygrid_mouse = 0.0f;

void resetTransformations() {
	_xdiff_rotate = 0.0f;
	_ydiff_rotate = 0.0f;
	_zdiff_rotate = 0.0f;
	_xdiff_translate = 0.0f;
	_ydiff_translate = 0.0f;
	_zdiff_translate = 0.0f;
	_xdiff_scale = 0.0f;
	_ydiff_scale = 0.0f;
	_radius_scale = 0.0f;
	_radius_orig = 0.0f;
	_radius_diff_scale = 1.0f;
}

void myCreateMenu() {
	int rendering_modes = glutCreateMenu(myMenu);
	glutAddMenuEntry("Polygon Point Mode", POLYGON_MODE_POINT);
	glutAddMenuEntry("Polygon Line Mode", POLYGON_MODE_LINE);
	glutAddMenuEntry("Polygon Fill Mode", POLYGON_MODE_FILL);
	glutAddMenuEntry("Polygon Line and Fill Mode", POLYGON_MODE_LINE_FILL);

	int brother_blender = glutCreateMenu(myMenu);
	glutAddMenuEntry("Black", MESH_BROTHER_BLENDER_BLACK);
	glutAddMenuEntry("White", MESH_BROTHER_BLENDER_WHITE);

	int monkey_blender = glutCreateMenu(myMenu);
	glutAddMenuEntry("White", MESH_MONKEY_BLENDER_WHITE);
	glutAddMenuEntry("Red", MESH_MONKEY_BLENDER_BLACK);

	int mesh_sample = glutCreateMenu(myMenu);
	glutAddMenuEntry("Metal", MESH_SAMPLE_METAL);
	glutAddMenuEntry("Glass", MESH_SAMPLE_GLASS);
	glutAddMenuEntry("Fabric", MESH_SAMPLE_FABRIC);

	int room_walls = glutCreateMenu(myMenu);
	glutAddMenuEntry("Stucco", ROOM_WALLS_STUCCO);
	glutAddMenuEntry("Dry Wall", ROOM_WALLS_DRY_WALLS);
	glutAddMenuEntry("Brick", ROOM_WALLS_BRICK);

	int shades = glutCreateMenu(myMenu);
	glutAddMenuEntry("Smooth", SMOOTH_SHADING);
	glutAddMenuEntry("Flat", FLAT_SHADING);

	int pointLight = glutCreateMenu(myMenu);
	glutAddMenuEntry("ON", POINT_LIGHT_ON);
	glutAddMenuEntry("OFF", POINT_LIGHT_OFF);

	int spotLight = glutCreateMenu(myMenu);
	glutAddMenuEntry("ON", SPOT_LIGHT_ON);
	glutAddMenuEntry("OFF", SPOT_LIGHT_OFF);

	int upperLight = glutCreateMenu(myMenu);
	glutAddMenuEntry("White", UPPER_LIGHT_WHITE);
	glutAddMenuEntry("Orange", UPPER_LIGHT_ORANGE);

	int frontLight = glutCreateMenu(myMenu);
	glutAddMenuEntry("White", FRONT_LIGHT_WHITE);
	glutAddMenuEntry("Turquoise", FRONT_LIGHT_TURQUOISE);

	int origin = glutCreateMenu(myMenu);
	glutAddMenuEntry("Hidden", ORIGIN_HIDDEN);
	glutAddMenuEntry("Visible", ORIGIN_VISIBLE);

	menu_all = glutCreateMenu(myMenu);
	glutAddSubMenu("Rendering Modes", rendering_modes);
	glutAddSubMenu("Brother Blender", brother_blender);
	glutAddSubMenu("Monkey Blender", monkey_blender);
	glutAddSubMenu("Mesh Sample", mesh_sample);
	glutAddSubMenu("Room Walls", room_walls);
	glutAddSubMenu("Shading", shades);
	glutAddSubMenu("Lamp", pointLight);
	glutAddSubMenu("Spotlight", spotLight);
	glutAddSubMenu("Upper Light", upperLight);
	glutAddSubMenu("Front Light", frontLight);
	glutAddSubMenu("Origin", origin);

	glutAddMenuEntry("Rotate While Idle", OPTION_ROTATE_IDLE);
	glutAddMenuEntry("Exit", EXIT_APP);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

bool glInit() {
	glClearColor(0.66f, 0.66f, 0.66f, 0.66f);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	return true;
}

void myResize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);

	_current_width = w;
	_current_height = h;

	gluPerspective(_current_fov, 1.0f * (float) w / (float) h, 1.0f, 300.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle() {
	if (!_mouseDown && _idle_rotate_current) {
		_xdiff_rotate += 0.6f;
		_ydiff_rotate += 0.5f;
		_zdiff_rotate += 0.4f;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(1);
		break;
	case 114:
		_transform_current = TRANSFORM_ROTATE;
		_z_axis_enabled = false;
		break;
	case 116:
		_transform_current = TRANSFORM_TRANSLATE;
		_z_axis_enabled = false;
		break;
	case 115:
		_transform_current = TRANSFORM_SCALE;
		_z_axis_enabled = false;
		break;
	case 49:
		_transform_current = TRANSFORM_LIGHT1_TRANSLATE;
		_z_axis_enabled = false;
		break;
	case 50:
		_transform_current = TRANSFORM_LIGHT0_TRANSLATE;
		_z_axis_enabled = false;
		break;
	case 122:
		_z_axis_enabled = true;
		break;
	case 48:
		resetTransformations();
		break;
	}
}

void specialKeyboard(int key, int x, int y) {
	if (key == GLUT_KEY_F1) {
		_fullscreen = !_fullscreen;
		if (_fullscreen)
			glutFullScreen();
		else {
			glutReshapeWindow(_current_width, _current_height);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		_mouseDown = true;
		_xtransform = x;
		_ytransform = _current_height - y;
		_xgrid_mouse = -(_current_width / 2.0 - x);
		_ygrid_mouse = -(_current_height / 2.0 - (_current_height - y));
		_radius_scale = _radius_diff_scale;
		_radius_orig = sqrt(
				pow(_xgrid_mouse - _xdiff_translate, 2)
						+ pow(_ygrid_mouse - _ydiff_translate, 2));
	} else {
		_mouseDown = false;
	}
}

void mouseMotion(int x, int y) {
	if (_transform_current == TRANSFORM_ROTATE) {
		if (_z_axis_enabled) {
			_zdiff_rotate += -(x - _xtransform);
		} else {
			_xdiff_rotate += x - _xtransform;
			_ydiff_rotate += (_current_height - y) - _ytransform;
		}
	} else if (_transform_current == TRANSFORM_SCALE) {
		_xgrid_mouse = -(_current_width / 2.0 - x);
		_ygrid_mouse = -(_current_height / 2.0 - (_current_height - y));
		GLfloat radius = sqrt(
				pow(_xgrid_mouse - _xdiff_translate, 2)
						+ pow(_ygrid_mouse - _ydiff_translate, 2));
		_radius_diff_scale = radius / _radius_orig * _radius_scale;
	} else if (_transform_current == TRANSFORM_TRANSLATE) {
		if (_z_axis_enabled) {
			_zdiff_translate += -(((_current_height - y) - _ytransform));
		} else {
			_xdiff_translate += (x - _xtransform);
			_ydiff_translate += ((_current_height - y) - _ytransform);
		}
	} else if (_transform_current == TRANSFORM_LIGHT1_TRANSLATE) {
		if (_z_axis_enabled) {
			_light1_pos[2] += (-(((_current_height - y) - _ytransform)))/10.0;
		} else {
			_light1_pos[0] += (x - _xtransform)/10.0;
			_light1_pos[1] += ((_current_height - y) - _ytransform)/10.0;
		}
	} else if (_transform_current == TRANSFORM_LIGHT0_TRANSLATE) {
		if (_z_axis_enabled) {
			_light0_pos[2] += (-(((_current_height - y) - _ytransform)))/10.0;
		} else {
			_light0_pos[0] += (x - _xtransform)/10.0;
			_light0_pos[1] += ((_current_height - y) - _ytransform)/10.0;
		}
	}
	_xtransform = x;
	_ytransform = _current_height - y;
	_xgrid_mouse = -(_current_width / 2.0 - x);
	_ygrid_mouse = -(_current_height / 2.0 - (_current_height - y));
	glutPostRedisplay();
}

void calculateNormal(FLTVECTPLUS p1, FLTVECTPLUS p2, FLTVECTPLUS p3,
		GLfloat** normal) {
	GLfloat v1[3];
	GLfloat v2[3];
	v1[0] = p2.x - p1.x;
	v1[1] = p2.y - p1.y;
	v1[2] = p2.z - p1.z;
	v2[0] = p3.x - p1.x;
	v2[1] = p3.y - p1.y;
	v2[2] = p3.z - p1.z;
	(*normal)[0] = v1[1] * v2[2] - v1[2] * v2[1];
	(*normal)[1] = v1[2] * v2[0] - v2[2] * v1[0];
	(*normal)[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void updateNormal(GLfloat** normal, GLfloat* add, int count) {
	(*normal)[0] = ((*normal)[0] + add[0]) / (GLfloat) count;
	(*normal)[1] = ((*normal)[1] + add[1]) / (GLfloat) count;
	(*normal)[2] = ((*normal)[2] + add[2]) / (GLfloat) count;
}

int readOFFMesh(const char* file) {

	int num, n, m;
	int a, b, c, d;
	float x, y, z;
	char line[256];
	FILE *fin;
	if ((fin = fopen(file, "r")) == NULL) {
		printf("read error... %s", &file);
		exit(65);
	};
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'O' && line[1] == 'F' && line[2] == 'F') /* OFF format */
			break;
	}
	fscanf(fin, "%d %d %d\n", &m, &n, &num);
	_surfmesh = (SurFaceMesh*) malloc(sizeof(SurFaceMesh));
	_surfmesh->nv = m;
	_surfmesh->nf = n;
	_surfmesh->vertex = (FLTVECTPLUS *) malloc(
			sizeof(FLTVECTPLUS) * _surfmesh->nv);
	_surfmesh->face = (INT3VECTPLUS *) malloc(
			sizeof(INT3VECTPLUS) * _surfmesh->nf);
	for (n = 0; n < _surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		_surfmesh->vertex[n].x = x;
		_surfmesh->vertex[n].y = y;
		_surfmesh->vertex[n].z = z;
		_surfmesh->vertex[n].connected_count = 0;
		_surfmesh->vertex[n].normal = (GLfloat*) malloc(3 * sizeof(GLfloat));
		_surfmesh->vertex[n].normal[0] = 0.0;
		_surfmesh->vertex[n].normal[1] = 0.0;
		_surfmesh->vertex[n].normal[2] = 0.0;
	}
	for (n = 0; n < _surfmesh->nf; n++) {
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		_surfmesh->face[n].a = b;
		_surfmesh->face[n].b = c;
		_surfmesh->face[n].c = d;
		_surfmesh->face[n].normal = (GLfloat*) malloc(3 * sizeof(GLfloat));
		calculateNormal(_surfmesh->vertex[b], _surfmesh->vertex[c],
				_surfmesh->vertex[d], &_surfmesh->face[n].normal);
		_surfmesh->vertex[a].connected_count++;
		_surfmesh->vertex[b].connected_count++;
		_surfmesh->vertex[c].connected_count++;
		updateNormal(&_surfmesh->vertex[a].normal, _surfmesh->face[n].normal,
				_surfmesh->vertex[a].connected_count);
		updateNormal(&_surfmesh->vertex[b].normal, _surfmesh->face[n].normal,
				_surfmesh->vertex[b].connected_count);
		updateNormal(&_surfmesh->vertex[c].normal, _surfmesh->face[n].normal,
				_surfmesh->vertex[c].connected_count);

		if (a != 3) {
			printf("Errors: reading mesh .... \n");
		}
	}
	fclose(fin);
	return 0;
}

bool floatEquals(float a, float b) {
	return fabs(a - b) < 0.00001;
}

bool getPoint(float x, float y, float z, RawMesh * mesh, int count,
		FLTVECTPLUS** out) {
	for (int i = 0; i < count; i++) {
		if (floatEquals(x, mesh->list[i].p1->x)
				&& floatEquals(y, mesh->list[i].p1->y)
				&& floatEquals(z, mesh->list[i].p1->z)) {
			*out = mesh->list[i].p1;
			return true;
		} else if (floatEquals(x, mesh->list[i].p2->x)
				&& floatEquals(y, mesh->list[i].p2->y)
				&& floatEquals(z, mesh->list[i].p2->z)) {
			*out = mesh->list[i].p2;
			return true;
		} else if (floatEquals(x, mesh->list[i].p3->x)
				&& floatEquals(y, mesh->list[i].p3->y)
				&& floatEquals(z, mesh->list[i].p3->z)) {
			*out = mesh->list[i].p3;
			return true;
		}
	}
	return false;
}

int readRawMesh(const char* file, RawMesh** triangular_mesh) {
	int count;
	float x, y, z, x1, y1, z1, x2, y2, z2;
	char line[256];
	FILE *fin;
	if ((fin = fopen(file, "r")) == NULL) {
		printf("read error...%s", &file);
		exit(65);
	};
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'R' && line[1] == 'A' && line[2] == 'W') /* RAW format */
			break;
	}

	fscanf(fin, "%d\n", &count);
	*triangular_mesh = (RawMesh*) malloc(sizeof(RawMesh));
	(*triangular_mesh)->count = count;

	(*triangular_mesh)->list = (TRIANGLEPLUS*) malloc(
			sizeof(TRIANGLEPLUS) * count);

	for (int n = 0; n < count; n++) {
		fscanf(fin, "%f %f %f %f %f %f %f %f %f\n", &x, &y, &z, &x1, &y1, &z1,
				&x2, &y2, &z2);
		FLTVECTPLUS* point;

		if (getPoint(x, y, z, (*triangular_mesh), n, &point)) {
			(*triangular_mesh)->list[n].p1 = point;
			(*triangular_mesh)->list[n].p1->connected_count++;
		} else {
			(*triangular_mesh)->list[n].p1 = (FLTVECTPLUS*) malloc(
					sizeof(FLTVECTPLUS));
			(*triangular_mesh)->list[n].p1->connected_count = 1;
			(*triangular_mesh)->list[n].p1->x = x;
			(*triangular_mesh)->list[n].p1->y = y;
			(*triangular_mesh)->list[n].p1->z = z;
			(*triangular_mesh)->list[n].p1->normal = (GLfloat*) malloc(
					3 * sizeof(GLfloat));
			(*triangular_mesh)->list[n].p1->normal[0] = 0.0;
			(*triangular_mesh)->list[n].p1->normal[1] = 0.0;
			(*triangular_mesh)->list[n].p1->normal[2] = 0.0;
		}

		if (getPoint(x1, y1, z1, (*triangular_mesh), n, &point)) {
			(*triangular_mesh)->list[n].p2 = point;
			(*triangular_mesh)->list[n].p2->connected_count++;
		} else {
			(*triangular_mesh)->list[n].p2 = (FLTVECTPLUS*) malloc(
					sizeof(FLTVECTPLUS));
			(*triangular_mesh)->list[n].p2->connected_count = 1;
			(*triangular_mesh)->list[n].p2->x = x1;
			(*triangular_mesh)->list[n].p2->y = y1;
			(*triangular_mesh)->list[n].p2->z = z1;
			(*triangular_mesh)->list[n].p2->normal = (GLfloat*) malloc(
					3 * sizeof(GLfloat));
			(*triangular_mesh)->list[n].p2->normal[0] = 0.0;
			(*triangular_mesh)->list[n].p2->normal[1] = 0.0;
			(*triangular_mesh)->list[n].p2->normal[2] = 0.0;
		}

		if (getPoint(x2, y2, z2, (*triangular_mesh), n, &point)) {
			(*triangular_mesh)->list[n].p3 = point;
			(*triangular_mesh)->list[n].p3->connected_count++;
		} else {
			(*triangular_mesh)->list[n].p3 = (FLTVECTPLUS*) malloc(
					sizeof(FLTVECTPLUS));
			(*triangular_mesh)->list[n].p3->connected_count = 1;
			(*triangular_mesh)->list[n].p3->x = x2;
			(*triangular_mesh)->list[n].p3->y = y2;
			(*triangular_mesh)->list[n].p3->z = z2;
			(*triangular_mesh)->list[n].p3->normal = (GLfloat*) malloc(
					3 * sizeof(GLfloat));
			(*triangular_mesh)->list[n].p3->normal[0] = 0.0;
			(*triangular_mesh)->list[n].p3->normal[1] = 0.0;
			(*triangular_mesh)->list[n].p3->normal[2] = 0.0;
		}

		(*triangular_mesh)->list[n].normal = (GLfloat*) malloc(
				3 * sizeof(GLfloat));
		calculateNormal(*(*triangular_mesh)->list[n].p1,
				*(*triangular_mesh)->list[n].p2,
				*(*triangular_mesh)->list[n].p3,
				&(*triangular_mesh)->list[n].normal);

		updateNormal(&(*triangular_mesh)->list[n].p1->normal,
				(*triangular_mesh)->list[n].normal,
				(*triangular_mesh)->list[n].p1->connected_count);
		updateNormal(&(*triangular_mesh)->list[n].p2->normal,
				(*triangular_mesh)->list[n].normal,
				(*triangular_mesh)->list[n].p2->connected_count);
		updateNormal(&(*triangular_mesh)->list[n].p3->normal,
				(*triangular_mesh)->list[n].normal,
				(*triangular_mesh)->list[n].p3->connected_count);

	}

	fclose(fin);
	return 0;
}

void myMenu(int value) {
	if (value == 0) {
		glutDestroyWindow(_windowID);
		exit(0);
	} else if (value < 5) {
		_polygon_render_mode = value;
	} else if (value == OPTION_ROTATE_IDLE) {
		_idle_rotate_current = !_idle_rotate_current;
	} else {
		if (value == MESH_SAMPLE_METAL || value == MESH_SAMPLE_GLASS
				|| value == MESH_SAMPLE_FABRIC) {
			_mesh_sample_mat = value;
		} else if (value == MESH_BROTHER_BLENDER_BLACK
				|| value == MESH_BROTHER_BLENDER_WHITE) {
			_mesh_brother_color = value;
		} else if (value == MESH_MONKEY_BLENDER_BLACK
				|| value == MESH_MONKEY_BLENDER_WHITE) {
			_mesh_monkey_color = value;
		} else if (value == ROOM_WALLS_BRICK || value == ROOM_WALLS_DRY_WALLS
				|| value == ROOM_WALLS_STUCCO) {
			_walls_mode = value;
		} else if (value == SMOOTH_SHADING || value == FLAT_SHADING) {
			_shading_model = value;
		} else if (value == POINT_LIGHT_ON || value == POINT_LIGHT_OFF) {
			_pointLightState = value;
		} else if (value == SPOT_LIGHT_ON || value == SPOT_LIGHT_OFF) {
			_spotLightState = value;
		} else if (value == UPPER_LIGHT_WHITE || value == UPPER_LIGHT_ORANGE) {
			_upper_light_color = value;
		} else if (value == FRONT_LIGHT_WHITE || value == FRONT_LIGHT_TURQUOISE) {
			_front_light_color = value;
		} else if (value == ORIGIN_HIDDEN || value == ORIGIN_VISIBLE) {
			_origin_visibility = value;
		}
	}
}

void drawOFFMeshWithoutColor(SurFaceMesh *surfmesh) {
	for (int i = 0; i < surfmesh->nf; i++) {
		glBegin(GL_TRIANGLES);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x,
				surfmesh->vertex[surfmesh->face[i].a].y,
				surfmesh->vertex[surfmesh->face[i].a].z);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x,
				surfmesh->vertex[surfmesh->face[i].b].y,
				surfmesh->vertex[surfmesh->face[i].b].z);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x,
				surfmesh->vertex[surfmesh->face[i].c].y,
				surfmesh->vertex[surfmesh->face[i].c].z);
		glEnd();
	}
}

void drawRawMeshWithoutColor(RawMesh *mesh) {
	for (int i = 0; i < mesh->count; i++) {
		glBegin(GL_TRIANGLES);
		glVertex3f(mesh->list[i].p1->x, mesh->list[i].p1->y,
				mesh->list[i].p1->z);
		glVertex3f(mesh->list[i].p2->x, mesh->list[i].p2->y,
				mesh->list[i].p2->z);
		glVertex3f(mesh->list[i].p3->x, mesh->list[i].p3->y,
				mesh->list[i].p3->z);
		glEnd();
	}
}

void setUpFaceNormal(FLTVECTPLUS p1, FLTVECTPLUS p2, FLTVECTPLUS p3) {
	GLfloat* normal = (GLfloat *) malloc(3 * sizeof(GLfloat));
	calculateNormal(p1, p2, p3, &normal);
	glNormal3fv(normal);
	free(normal);
}

void setUpFaceNormal(TRIANGLEPLUS triangle) {
	setUpFaceNormal(*triangle.p1, *triangle.p2, *triangle.p3);
}

void drawOFFMesh(SurFaceMesh *surfmesh) {
	int numberOfColors = 8;
	for (int i = 0; i < surfmesh->nf; i++) {
		if (i % numberOfColors == 0) {
			glColor3f(0.0, 0.0, 1.0);
		} else if (i % numberOfColors == 1) {
			glColor3f(0.0, 1.0, 0.0);
		} else if (i % numberOfColors == 2) {
			glColor3f(1.0, 0.0, 0.0);
		} else if (i % numberOfColors == 3) {
			glColor3f(1.0, 0.0, 1.0);
		} else if (i % numberOfColors == 4) {
			glColor3f(0.0, 1.0, 1.0);
		} else if (i % numberOfColors == 5) {
			glColor3f(1.0, 1.0, 0.0);
		} else if (i % numberOfColors == 6) {
			glColor3f(1.0, 1.0, 1.0);
		} else if (i % numberOfColors == 7) {
			glColor3f(0.0, 0.0, 0.0);
		}
		if (_shading_model == FLAT_SHADING) {
			glNormal3fv(surfmesh->face[i].normal);
			glBegin(GL_TRIANGLES);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x,
					surfmesh->vertex[surfmesh->face[i].a].y,
					surfmesh->vertex[surfmesh->face[i].a].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x,
					surfmesh->vertex[surfmesh->face[i].b].y,
					surfmesh->vertex[surfmesh->face[i].b].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x,
					surfmesh->vertex[surfmesh->face[i].c].y,
					surfmesh->vertex[surfmesh->face[i].c].z);
			glEnd();
		} else if (_shading_model == SMOOTH_SHADING) {
			glBegin(GL_TRIANGLES);
			glNormal3fv(surfmesh->vertex[surfmesh->face[i].a].normal);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x,
					surfmesh->vertex[surfmesh->face[i].a].y,
					surfmesh->vertex[surfmesh->face[i].a].z);
			glNormal3fv(surfmesh->vertex[surfmesh->face[i].b].normal);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x,
					surfmesh->vertex[surfmesh->face[i].b].y,
					surfmesh->vertex[surfmesh->face[i].b].z);
			glNormal3fv(surfmesh->vertex[surfmesh->face[i].c].normal);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x,
					surfmesh->vertex[surfmesh->face[i].c].y,
					surfmesh->vertex[surfmesh->face[i].c].z);
			glEnd();
		}

	}
}

void drawRawMesh(RawMesh * mesh, float* rgb) {
	float currentColor = 0.0;
	for (int i = 0; i < mesh->count; i++) {
		if (currentColor >= 1.0) {
			currentColor = 0.0;
		}
		//glColor3f(currentColor, currentColor - currentColor / 6.0, 0);
		currentColor += 0.05;
		glColor3f(rgb[0], rgb[1], rgb[2]);
		if (_shading_model == FLAT_SHADING) {
			glNormal3fv(mesh->list[i].normal);
			glBegin(GL_TRIANGLES);
			glVertex3f(mesh->list[i].p1->x, mesh->list[i].p1->y,
					mesh->list[i].p1->z);
			glVertex3f(mesh->list[i].p2->x, mesh->list[i].p2->y,
					mesh->list[i].p2->z);
			glVertex3f(mesh->list[i].p3->x, mesh->list[i].p3->y,
					mesh->list[i].p3->z);
			glEnd();
		} else if (_shading_model == SMOOTH_SHADING) {
			glBegin(GL_TRIANGLES);
			glNormal3fv(mesh->list[i].p1->normal);

			glVertex3f(mesh->list[i].p1->x, mesh->list[i].p1->y,
					mesh->list[i].p1->z);
			glNormal3fv(mesh->list[i].p2->normal);
			glVertex3f(mesh->list[i].p2->x, mesh->list[i].p2->y,
					mesh->list[i].p2->z);
			glNormal3fv(mesh->list[i].p3->normal);
			glVertex3f(mesh->list[i].p3->x, mesh->list[i].p3->y,
					mesh->list[i].p3->z);
			glEnd();
		}
	}
}

void drawSampleMesh(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.3, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.4, 0.0, 1.0 };
	float shine = 0.0;
	GLfloat shininess[4] = { 0.0, 0.0, 0.0, 1.0 };
	if (_mesh_sample_mat == MESH_SAMPLE_METAL) {
		shininess[0] = 0.5;
		shininess[1] = 0.5;
		shininess[2] = 0.5;
		shininess[3] = 1.0;
		shine = 25.0;
	} else if (_mesh_sample_mat == MESH_SAMPLE_GLASS) {
		shininess[0] = diffuse[0] = 0.75;
		shininess[1] = diffuse[1] = 0.75;
		shininess[2] = diffuse[2] = 0.75;
		shine = 100.0;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
	glTranslatef(0.3, 2.0, 0.4);
	glScalef(0.05, 0.05, 0.05);
	if (withColor) {
		drawOFFMesh(_surfmesh);
	} else {
		drawOFFMeshWithoutColor(_surfmesh);
	}
	glPopMatrix();
}

void drawBrotherBlender(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.6, 0.33, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.66, 0.33, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.66, 0.33, 0.0, 1.0 };
	if (_mesh_brother_color == MESH_BROTHER_BLENDER_WHITE) {
		ambient[0] = diffuse[0] = shininess[0] = 1.0;
		ambient[1] = diffuse[1] = shininess[1] = 1.0;
		ambient[2] = diffuse[2] = shininess[2] = 1.0;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 15.0);
	if (withColor) {
		float rgb[3] = { 0.1, 0.1, 0.1 };
		drawRawMesh(_brother_blender_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_brother_blender_mesh);
	}
	glPopMatrix();
}

void drawBlenderMonkey(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat diffuse[4] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat shininess[4] = { 0.3, 0.3, 0.3, 1.0 };
	if (_mesh_monkey_color == MESH_MONKEY_BLENDER_BLACK) {
		ambient[0] = diffuse[0] = shininess[0] = 0.66;
		ambient[1] = diffuse[1] = shininess[1] = 0.1;
		ambient[2] = diffuse[2] = shininess[2] = 0.1;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	if (withColor) {
		float rgb[3] = { 0.0, 0.0, 0.0 };
		drawRawMesh(_blender_monkey_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_blender_monkey_mesh);
	}
	glPopMatrix();
}

void drawRoomWalls(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat ambient[4] = { 0.2, 0.2, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.3, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.1, 0.0, 0.4, 1.0 };
	float shine = 10.0;
	if (_walls_mode == ROOM_WALLS_DRY_WALLS) {
		shininess[0] = shininess[1] = shininess[2]  = 0.0;
		ambient[0] = diffuse[0] = 0.4;
		ambient[1] = diffuse[1] = 0.4;
		ambient[2] = diffuse[2] = 0.4;
		shine = 0.0;
	} else if (_walls_mode == ROOM_WALLS_BRICK) {
		ambient[0] = diffuse[0] = shininess[0] = 0.4;
		ambient[1] = diffuse[1] = shininess[1] = 0.2;
		ambient[2] = diffuse[2] = shininess[2] = 0.0;
		shine = 20.0;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);

	if (withColor) {
		float rgb[3] = { 0.15, 0.15, 0.85 };
		drawRawMesh(_room_walls_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_room_walls_mesh);
	}
	glPopMatrix();
}

void drawTables(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.2, 0.1, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.2, 0.1, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.2, 0.1, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);

	if (withColor) {
		float rgb[3] = { 0.15, 0.15, 0.85 };
		drawRawMesh(_tables_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_tables_mesh);
	}
	glPopMatrix();
}

void drawLampBases(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);

	if (withColor) {
		float rgb[3] = { 0.15, 0.15, 0.85 };
		drawRawMesh(_lamp_bases_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_lamp_bases_mesh);
	}
	glPopMatrix();
}

void drawLampPoint(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat shininess[4] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);

	if (withColor) {
		float rgb[3] = { 0.15, 0.15, 0.85 };
		drawRawMesh(_lamp_point_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_lamp_point_mesh);
	}
	glPopMatrix();
}

void drawLampSpotlight(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat ambient[4] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat shininess[4] = { 0.0, 0.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);

	if (withColor) {
		float rgb[3] = { 0.15, 0.15, 0.85 };
		drawRawMesh(_lamp_spotlight_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_lamp_spotlight_mesh);
	}
	glPopMatrix();
}

void drawScene(bool withColor) {
	glPushMatrix();
	if (withColor) {
		drawRawMesh(_scene_mesh, 0);
	} else {
		drawRawMeshWithoutColor(_scene_mesh);
	}
	glPopMatrix();
}

void setUpSpotlight() {
	glEnable(GL_LIGHT2);
	GLfloat pos[4] = { -1.2, -5.8, 0.2, 1.0 };
	glLightfv(GL_LIGHT2, GL_POSITION, pos);
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specular);
	GLfloat direction[3] = { -2.0, 1.0, -0.1 };
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, direction);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 40.0);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 0.0);
}

void setUpPointlight() {
	glEnable(GL_LIGHT3);
	GLfloat pos[4] = { 2.1, -6.2, 0.2, 1.0 };
	glLightfv(GL_LIGHT3, GL_POSITION, pos);
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT3, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, specular);
}

void drawAll(bool withColor, bool separate) {
	if (separate) {
		glPushMatrix();
		glScalef(3.0, 3.0, 3.0);
		glRotatef(-80, 1.0, 0.0, 0.0);
		drawRoomWalls(withColor);
		drawBrotherBlender(withColor);
		drawBlenderMonkey(withColor);
		drawTables(withColor);
		drawLampBases(withColor);
		drawLampPoint(withColor);
		drawLampSpotlight(withColor);
		drawSampleMesh(true);
		if (_spotLightState == SPOT_LIGHT_ON) {
			setUpSpotlight();
		} else {
			glDisable(GL_LIGHT2);
		}
		if (_pointLightState == POINT_LIGHT_ON) {
			setUpPointlight();
		} else {
			glDisable(GL_LIGHT3);
		}
		glPopMatrix();
	} else {
		drawScene(withColor);
	}
}

void drawAllWithMode() {
	if (_polygon_render_mode == POLYGON_MODE_FILL) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawAll(true, true);
	} else if (_polygon_render_mode == POLYGON_MODE_POINT) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(2.0);
		drawAll(true, true);
	} else if (_polygon_render_mode == POLYGON_MODE_LINE) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawAll(true, true);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawAll(true, true);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor3f(1.0, 1.0, 1.0);
		drawAll(false, true);
	}
}

void setUpDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void cleanUpDisplay() {
	glFlush();
	glutSwapBuffers();
}

void drawLightSource1() {
	glPushMatrix();
	GLfloat emission[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.0, 0.0, 0.0, 1.0 };
	if (_upper_light_color == UPPER_LIGHT_ORANGE) {
		emission[0] = 1.0;
		emission[1] = 0.5;
		emission[2] = 0.0;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	glTranslatef(_light1_pos[0], _light1_pos[1], _light1_pos[2]);
	glutSolidSphere(1, 10, 10);
	glPopMatrix();
}

void drawLightSource0() {
	glPushMatrix();
	GLfloat emission[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.0, 0.0, 0.0, 1.0 };
	if (_front_light_color == FRONT_LIGHT_TURQUOISE) {
			emission[0] = 0.0;
			emission[1] = 1.0;
			emission[2] = 1.0;
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	glTranslatef(_light0_pos[0], _light0_pos[1], _light0_pos[2]);
	glutSolidSphere(1, 10, 10);
	glPopMatrix();
}

void drawOrigin() {
	if (_origin_visibility == ORIGIN_HIDDEN) {
		return;
	}
	glPushMatrix();
	GLfloat emission[4] = { 1.0, 0.8, 0.0, 1.0 };
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat shininess[4] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0);
	glutSolidSphere(0.5, 10, 10);
	glPopMatrix();
}

void drawObjects() {
	glPushMatrix();
	glTranslatef(_xdiff_translate / 10.0, _ydiff_translate / 10.0,
			_zdiff_translate);
	glScalef(_radius_diff_scale, _radius_diff_scale, _radius_diff_scale);
	glRotatef(-_ydiff_rotate, 1.0f, 0.0f, 0.0f);
	glRotatef(_xdiff_rotate, 0.0f, 1.0f, 0.0f);
	glRotatef(_zdiff_rotate, 0.0f, 0.0f, 1.0f);
	drawAllWithMode();
	glPopMatrix();
}

void setUpLight1() {
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, _light1_pos);
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	if (_upper_light_color == UPPER_LIGHT_ORANGE) {
		diffuse[0] = specular[0] = 1.0;
		diffuse[1] = specular[1] = 0.5;
		diffuse[2] = specular[2] = 0.0;
	}
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	GLfloat direction[3] = { 0.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 80.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);
}

void setUpLight0() {
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, _light0_pos);
	GLfloat ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	if (_front_light_color == FRONT_LIGHT_TURQUOISE) {
			diffuse[0] = specular[0] = 0.0;
			diffuse[1] = specular[1] = 1.0;
			diffuse[2] = specular[2] = 1.0;
		}
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

void setUpShading() {
	if (_shading_model == FLAT_SHADING) {
		glShadeModel(GL_FLAT);
	} else if (_shading_model == SMOOTH_SHADING) {
		glShadeModel(GL_SMOOTH);
	}
	glEnable(GL_NORMALIZE);
}

void display() {
	setUpDisplay();
	setUpShading();
	drawObjects();
	drawOrigin();
	drawLightSource0();
	drawLightSource1();
	setUpLight0();
	setUpLight1();
	cleanUpDisplay();
}

void myGlutInit(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	_windowID = glutCreateWindow("3D Triangle Mesh Manipulation");
	myCreateMenu();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(myResize);
	glutIdleFunc(idle);
}

void readBrotherBlender() {
	readRawMesh("brother_blender.raw", &_brother_blender_mesh);
}

void readBlenderMonkey() {
	readRawMesh("blender_monkey.raw", &_blender_monkey_mesh);
}

void readRoomWalls() {
	readRawMesh("room_walls.raw", &_room_walls_mesh);
}

void readScene() {
	readRawMesh("all.raw", &_scene_mesh);
}

void readTables() {
	readRawMesh("tables.raw", &_tables_mesh);
}

void readLampBases() {
	readRawMesh("lamp_bases.raw", &_lamp_bases_mesh);
}

void readLampPoint() {
	readRawMesh("lamp_point.raw", &_lamp_point_mesh);
}

void readLampSpotlight() {
	readRawMesh("lamp_spotlight.raw", &_lamp_spotlight_mesh);
}

void readAll() {
	readBrotherBlender();
	readBlenderMonkey();
	readRoomWalls();
	readTables();
	readLampBases();
	readLampPoint();
	readLampSpotlight();
	readOFFMesh("inputmesh_sample.off");
}

void setUpLighting() {
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHTING);
}

int main(int argc, char *argv[]) {
	myGlutInit(argc, argv);
	if (!glInit()) {
		return 1;
	}
	readAll();
	setUpLighting();
	glutMainLoop();
	return 0;
}

