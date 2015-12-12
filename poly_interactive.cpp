/*
 * poly_interactive.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: Victor
 */

/**
 * Menu Options:
 * 1) Render as Points
 * 2) Render as Lines
 * 3) Render as Fill
 * 4) Render as Line and Fills
 * 5) Render a Octahedron Mesh
 * 6) Render the sample Triangular Mesh (as provided with project)
 * 7) Render Brother Blender
 * 8) Rotate the Polygon While Idle (as provided with project)
 * 9) Exit
 *
 * Expected Mesh Files: inputmesh.off & inputmesh_sample.off
 *
 * 3D Geometric Transformations:
 * 'r' + left mouse drag = rotate
 * 's' + left mouse drag = scale
 * 't' + left mouse drag = translate
 *
 */

#ifdef _WIN32
#include <GL/glut.h>      // (or others, depending on the system in use)
#else
#include <GLUT/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <memory.h>

#define PI 3.14159265

#define HEIGHT 800
#define WIDTH 1200

void myMenu(int value);

static int EXIT_APP = 0;
static int POLYGON_MODE_POINT = 1;
static int POLYGON_MODE_LINE = 2;
static int POLYGON_MODE_FILL = 3;
static int POLYGON_MODE_LINE_FILL = 4;
static int MESH_OCTAHEDRON = 5;
static int MESH_SAMPLE = 6;
static int OPTION_ROTATE_IDLE = 7;
static int MESH_BROTHER_BLENDER = 8;

int _polygon_render_mode = POLYGON_MODE_FILL;
int _windowID;

static int _menu_all;

static int TRANSFORM_SCALE = 1;
static int TRANSFORM_TRANSLATE = 2;
static int TRANSFORM_ROTATE = 3;

int _transform_current = 0;
int _mesh_current = 5;
bool _idle_rotate_current = false;

int _current_height = HEIGHT;
int _current_width = WIDTH;

float _current_fov = 75.0;

bool _render_brother_blender = true;

bool _z_axis_enabled = false;

typedef struct {
	float x;
	float y;
	float z;
} FLTVECT;

typedef struct {
	int a;
	int b;
	int c;
} INT3VECT;

typedef struct {
	FLTVECT p1;
	FLTVECT p2;
	FLTVECT p3;
} TRIANGLE;

typedef struct {
	int nv;
	int nf;
	FLTVECT *vertex;
	INT3VECT *face;
} SurFaceMesh;

typedef struct {
	int count;
	TRIANGLE * list;
} RawMesh;

SurFaceMesh * _surfmesh;

RawMesh * _brother_blender_mesh;
RawMesh * _blender_monkey_mesh;
RawMesh * _room_walls_mesh;
RawMesh * _scene_mesh;

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
	_menu_all = glutCreateMenu(myMenu);
	glutAddMenuEntry("Polygon Point Mode", POLYGON_MODE_POINT);
	glutAddMenuEntry("Polygon Line Mode", POLYGON_MODE_LINE);
	glutAddMenuEntry("Polygon Fill Mode", POLYGON_MODE_FILL);
	glutAddMenuEntry("Polygon Line and Fill Mode", POLYGON_MODE_LINE_FILL);
	glutAddMenuEntry("Brother Blender Mesh", MESH_BROTHER_BLENDER);
	glutAddMenuEntry("Octahedron Mesh", MESH_OCTAHEDRON);
	glutAddMenuEntry("Sample Mesh", MESH_SAMPLE);
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
		_xdiff_rotate += 0.3f;
		_ydiff_rotate += 0.4f;
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
		}
		_xtransform = x;
		_ytransform = _current_height - y;
		_xgrid_mouse = -(_current_width / 2.0 - x);
		_ygrid_mouse = -(_current_height / 2.0 - (_current_height - y));
		glutPostRedisplay();
}

int readPolygonMesh(const char* file) {

	int num, n, m;
	int a, b, c, d;
	float x, y, z;
	char line[256];
	FILE *fin;
	if ((fin = fopen(file, "r")) == NULL) {
		printf("read error...\n");
		exit(0);
	};
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'O' && line[1] == 'F' && line[2] == 'F') /* OFF format */
			break;
	}
	fscanf(fin, "%d %d %d\n", &m, &n, &num);
	_surfmesh = (SurFaceMesh*) malloc(sizeof(SurFaceMesh));
	_surfmesh->nv = m;
	_surfmesh->nf = n;
	_surfmesh->vertex = (FLTVECT *) malloc(sizeof(FLTVECT) * _surfmesh->nv);
	_surfmesh->face = (INT3VECT *) malloc(sizeof(INT3VECT) * _surfmesh->nf);
	for (n = 0; n < _surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		_surfmesh->vertex[n].x = x;
		_surfmesh->vertex[n].y = y;
		_surfmesh->vertex[n].z = z;
	}
	for (n = 0; n < _surfmesh->nf; n++) {
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		_surfmesh->face[n].a = b;
		_surfmesh->face[n].b = c;
		_surfmesh->face[n].c = d;
		if (a != 3)
			printf("Errors: reading mesh .... \n");
	}
	fclose(fin);
	return 0;
}

int readRawMesh(const char* file, RawMesh** triangular_mesh) {
	int count;
	float x, y, z, x1, y1, z1, x2, y2, z2;
	char line[256];
	FILE *fin;
	if ((fin = fopen(file, "r")) == NULL) {
		printf("read error...\n");
		exit(0);
	};
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'R' && line[1] == 'A' && line[2] == 'W') /* RAW format */
			break;
	}
	fscanf(fin, "%d\n", &count);
	*triangular_mesh = (RawMesh*) malloc(sizeof(RawMesh));
	(*triangular_mesh)->count = count;

	(*triangular_mesh)->list = (TRIANGLE*) malloc(sizeof(TRIANGLE) * count);

	for (int n = 0; n < count; n++) {
		fscanf(fin, "%f %f %f %f %f %f %f %f %f\n", &x, &y, &z, &x1, &y1, &z1,
				&x2, &y2, &z2);
		(*triangular_mesh)->list[n].p1.x = x;
		(*triangular_mesh)->list[n].p1.y = y;
		(*triangular_mesh)->list[n].p1.z = z;
		(*triangular_mesh)->list[n].p2.x = x1;
		(*triangular_mesh)->list[n].p2.y = y1;
		(*triangular_mesh)->list[n].p2.z = z1;
		(*triangular_mesh)->list[n].p3.x = x2;
		(*triangular_mesh)->list[n].p3.y = y2;
		(*triangular_mesh)->list[n].p3.z = z2;
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
		_mesh_current = value;
		if (value == MESH_OCTAHEDRON) {
			free(_surfmesh->face);
			free(_surfmesh->vertex);
			free(_surfmesh);
			readPolygonMesh("inputmesh.off");
			_render_brother_blender = false;
		} else if (value == MESH_SAMPLE) {
			free(_surfmesh->face);
			free(_surfmesh->vertex);
			free(_surfmesh);
			readPolygonMesh("inputmesh_sample.off");
			_render_brother_blender = false;
		} else {
			_render_brother_blender = true;
		}
	}
}

void drawMeshWithoutColor(SurFaceMesh *surfmesh) {
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
		glVertex3f(mesh->list[i].p1.x, mesh->list[i].p1.y, mesh->list[i].p1.z);
		glVertex3f(mesh->list[i].p2.x, mesh->list[i].p2.y, mesh->list[i].p2.z);
		glVertex3f(mesh->list[i].p3.x, mesh->list[i].p3.y, mesh->list[i].p3.z);
		glEnd();
	}
}

void setUpFaceNormal(FLTVECT p1, FLTVECT p2, FLTVECT p3) {
	GLfloat normal[3];
	GLfloat v1[3];
	GLfloat v2[3];
	v1[0] = p2.x - p1.x;
	v1[1] = p2.y - p1.y;
	v1[2] = p2.z - p1.z;
	v2[0] = p3.x - p1.x;
	v2[1] = p3.y - p1.y;
	v2[2] = p3.z - p1.z;
	normal[0] = v1[1] * v2[2] - v1[2] * v2[1];
	normal[1] = v1[2] * v2[0] - v2[2] * v1[0];
	normal[2] = v1[0] * v2[1] - v1[1] * v2[0];
	glNormal3fv(normal);
}

void setUpFaceNormal(TRIANGLE triangle) {
	setUpFaceNormal(triangle.p1, triangle.p2, triangle.p3);
}

void drawMesh(SurFaceMesh *surfmesh) {
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
		setUpFaceNormal(surfmesh->vertex[surfmesh->face[i].a], surfmesh->vertex[surfmesh->face[i].b], surfmesh->vertex[surfmesh->face[i].c]);
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

void drawRawMesh(RawMesh * mesh, float* rgb) {
	float currentColor = 0.0;
	for (int i = 0; i < mesh->count; i++) {
		if (currentColor >= 1.0) {
			currentColor = 0.0;
		}
		//glColor3f(currentColor, currentColor - currentColor / 6.0, 0);
		currentColor += 0.05;
		glColor3f(rgb[0], rgb[1], rgb[2]);
		setUpFaceNormal(mesh->list[i]);
		glBegin(GL_TRIANGLES);
		glVertex3f(mesh->list[i].p1.x, mesh->list[i].p1.y, mesh->list[i].p1.z);
		glVertex3f(mesh->list[i].p2.x, mesh->list[i].p2.y, mesh->list[i].p2.z);
		glVertex3f(mesh->list[i].p3.x, mesh->list[i].p3.y, mesh->list[i].p3.z);
		glEnd();
	}
}

void drawSampleMesh(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat ambient[4] = {0.3, 0.0, 0.0, 1.0};
	GLfloat diffuse[4] = {0.0, 0.4, 0.0, 1.0};
	GLfloat shininess[4] = {0.5, 0.5, 0.5, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	glTranslatef(-5.0, -5.0, 0.0);
	glScalef(0.05, 0.05, 0.05);
	if (withColor) {
		drawMesh(_surfmesh);
	} else {
		drawMeshWithoutColor(_surfmesh);
	}
	glPopMatrix();
}

void drawBrotherBlender(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat ambient[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat shininess[4] = {1.0, 1.0, 1.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 15.0);
	if (withColor) {
		float rgb[3] = {0.1, 0.1, 0.1};
		drawRawMesh(_brother_blender_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_brother_blender_mesh);
	}
	glPopMatrix();
}

void drawBlenderMonkey(bool withColor) {
	glPushMatrix();
	GLfloat emission[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat ambient[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat shininess[4] = {0.0, 0.0, 0.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	if (withColor) {
		float rgb[3] = {0.5, 0.5, 0.5};
		drawRawMesh(_blender_monkey_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_blender_monkey_mesh);
	}
	glPopMatrix();
}

void drawRoomWalls(bool withColor) {
	glPushMatrix();

	GLfloat emission[4] = {0.1, 0.1, 0.1, 1.0};
	GLfloat ambient[4] = {0.2, 0.2, 0.0, 1.0};
	GLfloat diffuse[4] = {0.0, 0.3, 0.0, 1.0};
	GLfloat shininess[4] = {0.1, 0.0, 0.4, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);

	if (withColor) {
		float rgb[3] = {0.15, 0.15, 0.85};
		drawRawMesh(_room_walls_mesh, (float*) rgb);
	} else {
		drawRawMeshWithoutColor(_room_walls_mesh);
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

void drawAll(bool withColor, bool separate) {
	if (separate) {
		glPushMatrix();
		glScalef(3.0, 3.0, 3.0);
		glRotatef(-80, 1.0, 0.0, 0.0);
		drawRoomWalls(withColor);
		drawBrotherBlender(withColor);
		drawBlenderMonkey(withColor);
		drawSampleMesh(true);
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
	GLfloat emission[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat ambient[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat shininess[4] = {0.0, 0.0, 0.0, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	glTranslatef(0.0, 20.0, 0.0);
	glutSolidSphere(1, 10, 10);
	glPopMatrix();
}

void drawOrigin() {
	glPushMatrix();
	GLfloat emission[4] = {1.0, 0.8, 0.0, 1.0};
	GLfloat ambient[4] = {0.3, 0.0, 0.0, 1.0};
	GLfloat diffuse[4] = {0.0, 0.4, 0.0, 1.0};
	GLfloat shininess[4] = {0.5, 0.5, 0.5, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shininess);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0);
	glutSolidSphere(1, 10, 10);
	glPopMatrix();
}

void drawObjects() {
	glPushMatrix();
	glTranslatef(_xdiff_translate / 10.0, _ydiff_translate / 10.0, _zdiff_translate);
	glScalef(_radius_diff_scale, _radius_diff_scale, _radius_diff_scale);
	glRotatef(-_ydiff_rotate, 1.0f, 0.0f, 0.0f);
	glRotatef(_xdiff_rotate, 0.0f, 1.0f, 0.0f);
	glRotatef(_zdiff_rotate, 0.0f, 0.0f, 1.0f);
	drawAllWithMode();
	glPopMatrix();
}

void display() {
	setUpDisplay();
	drawObjects();
	drawOrigin();
	drawLightSource1();
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

void readAll() {
	readBrotherBlender();
	readBlenderMonkey();
	readRoomWalls();
	readPolygonMesh("inputmesh_sample.off");
}

void setUpFog() {
	glEnable (GL_FOG);
	GLfloat atmoColor [4] = {0.66f, 0.66f, 0.66f, 0.66f};
	glFogfv (GL_FOG_COLOR, atmoColor);
	glFogi(GL_FOG_MODE, GL_EXP);
}

void setUpLight0() {
	glEnable(GL_LIGHT0);
	GLfloat pos[4] = {5.0, 5.0, 40.0, 0.0};
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	GLfloat ambient[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat specular[4] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	//GLfloat direction[3] = {0.0, 0.0, 0.0};
	//glLightfv (GL_LIGHT0, GL_SPOT_DIRECTION, direction);
}

void setUpLight1() {
	glEnable(GL_LIGHT1);
	GLfloat pos[4] = {0.0, 20.0, -10.0, 1.0};
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	GLfloat ambient[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat specular[4] = {1.0, 1.0, 1.0, 1.0};
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	GLfloat direction[3] = {0.0, -1.0, 0.0};
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);
}

void setUpLighting() {
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHTING);
	//setUpLight0();
	setUpLight1();
}

void setUpShading() {
	glShadeModel(GL_FLAT);
	glEnable(GL_NORMALIZE);
}

int main(int argc, char *argv[]) {
	myGlutInit(argc, argv);
	if (!glInit()) {
		return 1;
	}
	readAll();
	setUpLighting();
	setUpShading();
	glutMainLoop();
	return 0;
}

