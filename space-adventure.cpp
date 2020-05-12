// space-adventure.cpp

#include "Angel.h"

#include <iostream> 
#include <time.h> 

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec3 point3;
typedef Angel::vec4 color4;

const long double TO_RADIANS = 3.141592653589793238L / 180.f;
const long double PI = 3.141592653589793238L;

// vertices and normals
const int TorusNumVertices = 416;
const int TetrahedronNumVertices = 12;
const int SphereNumVertices = 3 * 4096;
const int GroundNumVertices = 153600;
point4 points[TorusNumVertices + TetrahedronNumVertices + SphereNumVertices + GroundNumVertices];
point3 normals[TorusNumVertices + TetrahedronNumVertices + SphereNumVertices + GroundNumVertices];

GLfloat incAngle = 0;
GLdouble score = 0;
GLint attackNo = 0;

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;
GLuint AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition, Shininess;

// simulation variables
GLboolean simulationPaused = FALSE;
GLboolean singleStepMode = FALSE;
GLboolean autoPilotMode = FALSE;
GLboolean autoAttackMode = FALSE;
GLboolean autoTargetChoose = FALSE;

// camera variables
point4 eye;
point4 at;
vec4   up;

// view mode (1- first person, 2- station, 3- third person, 4- world) 
GLint viewMode = 1;

// spaceship variables
GLfloat shipSpeed = 1;
vec3 shipPosition = vec3(105.0, 0.0, 15.0);
vec3 shipVelocity = vec3(0.0, -1.0, 0.0);
GLfloat rotationHeading = 0;

// space station 
vec3 stationPosition = vec3(100.0, 10.0, 10);
GLfloat rotationalSpeed = 0.0;
GLfloat rotationAngel = 0.0;

// coords for each planet
GLfloat planet_coords[][3] = { {30, 30, 30},{30, 170, 15},{80, 110, 25},{70,  60, 12},{90, 150, 13},{120,  80, 17},{150,  40, 15},{160, 170, 22} };
// colors(rgb) for each planet
GLfloat planet_colors[][3] = { {0.30, 0.30, 0.30},{1.00, 0.00, 0.00},{0.00, 1.00, 0.00},{0.00, 0.00, 1.00},{1.00, 1.00, 0.00},{1.00, 0.00, 1.00},{0.00, 1.00, 1.00},{1.00, 1.00, 1.00} };

//npc ship variables
GLfloat npcSpeed = 0.7;
GLfloat npc_heading[] = { 0,0,0,0,0,0,0,0 };
vec3 npc_direction[] = { vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0),vec3(0.0, -1.0, 0.0) };
//coords for each npc ship
vec3 npc_init_positions[] = { {30, 30, 30},{30, 170, 15},{80, 110, 25},{70,  60, 12},{90, 150, 13},{120,  80, 17},{150,  40, 15},{160, 170, 22} };
vec3 npc_positions[] = { {30, 30, 30},{30, 170, 15},{80, 110, 25},{70,  60, 12},{90, 150, 13},{120,  80, 17},{150,  40, 15},{160, 170, 22} };
// colors(rgb) for each npc ship
GLfloat npc_colors[][3] = { {(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX),(double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)},{(double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX)} };

//----------------------------------------------------------------------------

long Index = 0;

void triangle(const point4& a, const point4& b, const point4& c) {
	normals[Index] = vec3(a.x, a.y, a.z);  points[Index] = a;  Index++;
	normals[Index] = vec3(b.x, b.y, b.z);  points[Index] = b;  Index++;
	normals[Index] = vec3(c.x, c.y, c.z);  points[Index] = c;  Index++;
}

point4 triangleArray[4][3];
vec3 triangleArrayNormals[4];
int in = 0;
void incTriangleArray(const point4& a, const point4& b, const point4& c) {
	triangleArray[in][0] = a;
	triangleArray[in][1] = b;
	triangleArray[in][2] = c;

	vec4 u = b - a;
	vec4 v = c - b;
	vec3 normal = cross(u, v);
	triangleArrayNormals[in++] = normal;
}

point4 processedPoints[12];
int processedPointsSize = 0;
void meanNormals() { // calculate mean of normals for Gouraud Shading
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			point4 temp = triangleArray[i][j];
			int sameCount = 0;
			vec3 sameTotalNormal = 0;
			for (int k = 0; k < 4; k++) {
				for (int l = 0; l < 3; l++) {
					bool ok = true;
					for (int z = 0; z < processedPointsSize; z++) {
						if (temp == processedPoints[z]) {
							ok = false;
						}
					}
					if (ok && temp.x == triangleArray[k][l].x && temp.y == triangleArray[k][l].y && temp.z == triangleArray[k][l].z) {
						sameCount++;
						sameTotalNormal += triangleArrayNormals[k];
					}
				}
			}
			processedPoints[processedPointsSize++] = temp;
			normals[Index] = normalize(sameTotalNormal / sameCount);
			points[Index++] = triangleArray[i][j];

		}
	}

}

// create torus points and normals
void makeTorus(int stack, int sector) {
	int i, j, k;
	double x, y, z, tx, ty, tz, sx, sy, sz;

	for (i = 0; i < stack; i++) {
		for (j = 0; j <= sector; j++) {
			for (k = 1; k >= 0; k--) {
				double degreeJ = (j % sector) * 2 * PI / sector;
				double degreeI = ((i + k) % stack + 0.5) * 2 * PI / stack;

				x = (1 + 0.1 * cos(degreeI)) * cos(degreeJ);
				y = (1 + 0.1 * cos(degreeI)) * sin(degreeJ);
				z = 0.1 * sin(degreeI);

				// calculate normal
				point3 normal = normalize(point3((cos(degreeJ)) * (cos(degreeI)) - 0 * (sin(degreeJ) * (-sin(degreeI))), 0 * (cos(degreeJ) * (-sin(degreeI))) - (-sin(degreeJ)) * (cos(degreeI)), (-sin(degreeJ)) * (sin(degreeJ) * (-sin(degreeI))) - (cos(degreeJ)) * (cos(degreeJ) * (-sin(degreeI)))));

				points[Index] = point4(x, y, z, 1);
				normals[Index++] = normal;
			}
		}
	}
}

void makeTetrahedron() {
	incTriangleArray(point4(0.0, 1.0, 0.0, 1), point4(-1, -0.5, 0.0, 1), point4(1, -0.5, 0.0, 1));
	incTriangleArray(point4(0.0, 0.0, 1.0, 1), point4(-1, -0.5, 0.0, 1), point4(1, -0.5, 0.0, 1));
	incTriangleArray(point4(0.0, 0.0, 1.0, 1), point4(1, -0.5, 0.0, 1), point4(0.0, 1.0, 0.0, 1));
	incTriangleArray(point4(0.0, 0.0, 1.0, 1), point4(0.0, 1.0, 0.0, 1), point4(-1, -0.5, 0.0, 1));
	meanNormals();
}

// normalize
point4 unit(const point4& p) {
	float len = p.x * p.x + p.y * p.y + p.z * p.z;
	point4 t;
	if (len > DivideByZeroTolerance) {
		t = p / sqrt(len);
		t.w = 1.0;
	}
	return t;
}

void divide_triangle(const point4& a, const point4& b,
	const point4& c, int count) {
	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

void tetrahedron(int count) {
	point4 v[4] = {
	vec4(0.0, 0.0, 1.0, 1.0),
	vec4(0.0, 0.942809, -0.333333, 1.0),
	vec4(-0.816497, -0.471405, -0.333333, 1.0),
	vec4(0.816497, -0.471405, -0.333333, 1.0)
	};

	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);
}

// create sphere points and normals
void makeSphere() {
	tetrahedron(5);
}

// create chessboard ground
void makeGround() {
	point4 vertices[4];
	for (GLfloat i = 160; i > -160; i = i - 1) {
		for (GLfloat j = -80; j < 80; j = j + 1) {
			if (((int)(i / 1 + j / 1)) % 2 == 0) {
				continue;
			}
			vertices[0] = point4(j, i, 0, 1.0);
			vertices[1] = point4(j + 1, i, 0, 1.0);
			vertices[2] = point4(j + 1, i - 1, 0, 1.0);
			vertices[3] = point4(j, i - 1, 0, 1.0);

			vec4 u = vertices[1] - vertices[0];
			vec4 v = vertices[2] - vertices[1];
			vec3 normal = normalize(cross(u, v));

			normals[Index] = normal; points[Index] = vertices[0]; Index++;
			normals[Index] = normal; points[Index] = vertices[1]; Index++;
			normals[Index] = normal; points[Index] = vertices[2]; Index++;
			normals[Index] = normal; points[Index] = vertices[0]; Index++;
			normals[Index] = normal; points[Index] = vertices[2]; Index++;
			normals[Index] = normal; points[Index] = vertices[3]; Index++;
		}
	}
}

// OpenGL initialization
void init() {
	// initialize points and normals
	makeTorus(8, 25);
	makeTetrahedron();
	makeSphere();
	makeGround();

	// set up camera
	vec3 u = shipPosition + (shipVelocity * 300);
	eye = vec4(shipPosition.x, shipPosition.y, shipPosition.z, 1.0);
	at = vec4(u.x, u.y, u.z, 1.0);
	up = vec4(1.0, 1.0, 300.0, 0.0);

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points),
		sizeof(normals), normals);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader53.glsl", "fshader53.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	// Retrieve shading uniform variable locations
	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	LightPosition = glGetUniformLocation(program, "LightPosition");
	Shininess = glGetUniformLocation(program, "Shininess");

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0); /* white background */
}


void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 model_view = LookAt(eye, at, up);

	// Initialize shader lighting parameters
	point4 light_position(2.0, 3.0, 2.0, 1.0); //wrt camera
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);

	//emerald
	color4 material_ambient(0.633, 0.727811, 0.633, 1.0);
	color4 material_diffuse(0.07568, 0.61424, 0.07568, 1.0);
	color4 material_specular(0.0215, 0.1745, 0.0215, 1.0);
	float  material_shininess = 960.8;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(AmbientProduct, 1, ambient_product);
	glUniform4fv(DiffuseProduct, 1, diffuse_product);
	glUniform4fv(SpecularProduct, 1, specular_product);
	glUniform4fv(LightPosition, 1, light_position);
	glUniform1f(Shininess, material_shininess);

	// create transformation matrix
	mat4 transformation;

	// Ship
	// vertical torus
	transformation = Translate(shipPosition) * RotateZ(rotationHeading) * Scale(10);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TorusNumVertices);
	// horizontal torus
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * RotateX(90));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TorusNumVertices);
	// Tetrahedron
	transformation = transformation * Translate(0, -1.085, 0) * RotateZ(150) * RotateY(80) * RotateX(235) * Scale(0.5);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
	glDrawArrays(GL_TRIANGLE_STRIP, TorusNumVertices, TetrahedronNumVertices);

	// Ground
	color4 material_ambientGround(0.0, 0.0, 0.0, 1.0);
	color4 material_diffuseGround(0.0, 0.0, 0.0, 1.0); // Black
	color4 material_specularGround(0.0, 0.0, 0.0, 1.0);
	material_shininess = 960.8;

	ambient_product = light_ambient * material_ambientGround;
	diffuse_product = light_diffuse * material_diffuseGround;
	specular_product = light_specular * material_specularGround;

	glUniform4fv(AmbientProduct, 1, ambient_product);
	glUniform4fv(DiffuseProduct, 1, diffuse_product);
	glUniform4fv(SpecularProduct, 1, specular_product);
	glUniform4fv(LightPosition, 1, light_position);
	glUniform1f(Shininess, material_shininess);

	transformation = Translate(0, -200, 0) * Scale(15);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
	glDrawArrays(GL_TRIANGLES, TorusNumVertices + TetrahedronNumVertices + SphereNumVertices, GroundNumVertices);

	// Space Station
	color4 material_ambientStation(1.0, 1.0, 1.0, 1.0);
	color4 material_diffuseStation(0.5, 0.5, 0.5, 1.0); // Grey
	color4 material_specularStation(1.0, 1.0, 1.0, 1.0);
	material_shininess = 960.8;

	ambient_product = light_ambient * material_ambientStation;
	diffuse_product = light_diffuse * material_diffuseStation;
	specular_product = light_specular * material_specularStation;

	glUniform4fv(AmbientProduct, 1, ambient_product);
	glUniform4fv(DiffuseProduct, 1, diffuse_product);
	glUniform4fv(SpecularProduct, 1, specular_product);
	glUniform4fv(LightPosition, 1, light_position);
	glUniform1f(Shininess, material_shininess);

	transformation = Translate(stationPosition) * RotateZ(rotationAngel) * Scale(10);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
	glDrawArrays(GL_TRIANGLE_STRIP, TorusNumVertices + TetrahedronNumVertices, SphereNumVertices);

	// Space Station Front
	transformation = Translate(stationPosition) * RotateZ(rotationAngel + 180) * Translate(10, 0, 0) * Scale(2.5);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
	glDrawArrays(GL_TRIANGLE_STRIP, TorusNumVertices + TetrahedronNumVertices, SphereNumVertices);

	// Planets
	for (int i = 0; i < 8; i++) {
		color4 material_ambient(1.0, 1.0, 1.0, 1.0);
		color4 material_diffuse(planet_colors[i][0], planet_colors[i][1], planet_colors[i][2], 1.0);
		color4 material_specular(1.0, 1.0, 1.0, 1.0);
		float  material_shininess = 960.8;

		color4 ambient_product = light_ambient * material_ambient;
		color4 diffuse_product = light_diffuse * material_diffuse;
		color4 specular_product = light_specular * material_specular;

		glUniform4fv(AmbientProduct, 1, ambient_product);
		glUniform4fv(DiffuseProduct, 1, diffuse_product);
		glUniform4fv(SpecularProduct, 1, specular_product);
		glUniform4fv(LightPosition, 1, light_position);
		glUniform1f(Shininess, material_shininess);

		// Sphere
		transformation = Translate(planet_coords[i][0], planet_coords[i][1], planet_coords[i][2]) * RotateZ(incAngle) * Scale(10);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
		glDrawArrays(GL_TRIANGLE_STRIP, TorusNumVertices + TetrahedronNumVertices, SphereNumVertices);

		// Ring
		transformation = Translate(planet_coords[i][0], planet_coords[i][1], planet_coords[i][2]) * RotateZ(incAngle) * RotateY(40) * Scale(12.5);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, TorusNumVertices);
	}

	//NPC Ship
	for (int i = 0; i < 8; i++) {
		color4 material_ambient(1.0, 1.0, 1.0, 1.0);
		color4 material_diffuse(npc_colors[i][0], npc_colors[i][1], npc_colors[i][2], 1.0);
		color4 material_specular(1.0, 1.0, 1.0, 1.0);
		float  material_shininess = 960.8;

		color4 ambient_product = light_ambient * material_ambient;
		color4 diffuse_product = light_diffuse * material_diffuse;
		color4 specular_product = light_specular * material_specular;

		glUniform4fv(AmbientProduct, 1, ambient_product);
		glUniform4fv(DiffuseProduct, 1, diffuse_product);
		glUniform4fv(SpecularProduct, 1, specular_product);
		glUniform4fv(LightPosition, 1, light_position);
		glUniform1f(Shininess, material_shininess);

		transformation = Translate(npc_positions[i][0], npc_positions[i][1], npc_positions[i][2]) * RotateZ(npc_heading[i]) * Scale(10);

		// vertical torus
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, TorusNumVertices);

		// horizontal torus
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation * RotateX(90));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, TorusNumVertices);

		//Tetrahedron
		transformation = transformation * Translate(0, -1.085, 0) * RotateZ(150) * RotateY(80) * RotateX(235) * Scale(0.5);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * transformation);
		glDrawArrays(GL_TRIANGLE_STRIP, TorusNumVertices, TetrahedronNumVertices);
	}

	glutSwapBuffers();
}


bool intersect(point4 a, vec3 b, GLdouble radius) {
	GLdouble distance = sqrt((a.x - b.x) * (a.x - b.x) +
		(a.y - b.y) * (a.y - b.y) +
		(a.z - b.z) * (a.z - b.z));
	return distance < (radius + radius);
}

void step() {

	// npc movements
	for (int i = 0; i < 8; i++) {
		int random = rand() % 100;
		if (random < 5) { // Turns left with 5% probability
			npc_direction[i] -= cross(npc_direction[i], point3(0, 0, 1));
			npc_direction[i] = normalize(npc_direction[i]);
			npc_heading[i] += 45;
		}
		else if (random < 10) {// Turns right with 5% probability
			npc_direction[i] += cross(npc_direction[i], point3(0, 0, 1));
			npc_direction[i] = normalize(npc_direction[i]);
			npc_heading[i] -= 45;
		}
		npc_positions[i] = npc_positions[i] + npc_direction[i] * npcSpeed;

		// if ship collides to other npc ships, score will increase
		if (intersect(shipPosition, npc_positions[i], 10)) {
			npc_positions[i] = npc_init_positions[i];
			cout << "npcShip-" << i << " killed" << endl;
			score += 5 * shipSpeed;
		}
	}

	// if ship collides to planets, score will be 0
	for (int i = 0; i < 8; i++) {
		if (intersect(shipPosition, vec3(planet_coords[i][0], planet_coords[i][1], planet_coords[i][2]), 10)) {
			score = 0;
		}
	}

	// increase angle value
	incAngle += 0.5;
	if (incAngle > 360) incAngle -= 360;

	// update ship's position by ship's speed and ship's velocity
	shipPosition = shipPosition + shipVelocity * shipSpeed;

	// Space Station
	rotationAngel = rotationAngel + rotationalSpeed;
	if (rotationAngel > 360)
		rotationAngel = rotationAngel - 360.0;

	// view mode
	if (viewMode == 1) { // first person view
		vec3 u = shipPosition + (shipVelocity * 300);
		eye = vec4(shipPosition.x, shipPosition.y, shipPosition.z, 1.0);
		at = vec4(u.x, u.y, u.z, 1.0);
		up = vec4(1.0, 1.0, 300.0, 0.0);
	}
	else if (viewMode == 2) { // station view
		eye = vec4(stationPosition.x - cos((rotationAngel + 180) * TO_RADIANS) * 7.5, stationPosition.y - sin((rotationAngel + 180) * TO_RADIANS) * 7.5, stationPosition.z, 1.0);
		at = vec4(cos((rotationAngel + 180) * TO_RADIANS) * 360 + stationPosition.x, sin((rotationAngel - 180) * TO_RADIANS) * 360 + stationPosition.y, +stationPosition.z, 1.0);
		up = vec4(0.0, 0.0, 300.0, 0.0);
	}
	else if (viewMode == 3) { // third person view
		vec3 u = shipPosition + (shipVelocity * 300);
		vec3 v;
		if (shipVelocity.y < 0 && shipVelocity.x < 0)
			v = shipPosition - (shipVelocity * 30) + vec3(14, 14, 20);
		else if (shipVelocity.y < 0 && shipVelocity.x > 0)
			v = shipPosition - (shipVelocity * 30) + vec3(-14, 14, 20);
		else if (shipVelocity.y > 0 && shipVelocity.x < 0)
			v = shipPosition - (shipVelocity * 30) + vec3(14, -14, 20);
		else if (shipVelocity.y > 0 && shipVelocity.x > 0)
			v = shipPosition - (shipVelocity * 30) + vec3(-14, -14, 20);
		else if (shipVelocity.y < 0)
			v = shipPosition - (shipVelocity * 30) + vec3(0, 20, 20);
		else if (shipVelocity.y > 0)
			v = shipPosition - (shipVelocity * 30) + vec3(0, -20, 20);
		else if (shipVelocity.x < 0)
			v = shipPosition - (shipVelocity * 30) + vec3(20, 0, 20);
		else if (shipVelocity.x > 0)
			v = shipPosition - (shipVelocity * 30) + vec3(-20, 0, 20);
		eye = vec4(v.x, v.y, v.z, 1.0);
		at = vec4(u.x, u.y, u.z, 1.0);
		up = vec4(1.0, 1.0, 300.0, 0.0);
	}
}

// print information for single step mode
void printInfo() {
	cout << "score : " << score << endl;
	cout << "" << endl;
	cout << "ship information: " << endl;
	cout << "ship location : (" << shipPosition.x << ", " << shipPosition.y << ", " << shipPosition.z << ")" << endl;
	cout << "ship direction : (" << shipVelocity.x << ", " << shipVelocity.y << ", " << shipVelocity.z << ")" << endl;
	cout << "ship speed : " << shipSpeed << endl;
	cout << "" << endl;
	cout << "station information: " << endl;
	cout << "station location : (" << stationPosition.x << ", " << stationPosition.y << ", " << stationPosition.z << ")" << endl;
	cout << "station rotation speed : " << rotationalSpeed << endl;
	cout << "station rotation angel : " << rotationAngel << endl;
	cout << "\n --------------------------- \n" << endl;
}


void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		simulationPaused = !simulationPaused;
		if (singleStepMode)
			singleStepMode = FALSE;
		if (simulationPaused) {
			singleStepMode = TRUE;
		}
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (singleStepMode) {
			step();
			printInfo();
		}
		if (simulationPaused == FALSE) {
			simulationPaused = TRUE;
			singleStepMode = TRUE;
			autoPilotMode = FALSE;
			autoAttackMode = FALSE;
		}
	}
}

void SpecialInput(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_LEFT:
		shipVelocity -= cross(shipVelocity, point3(0, 0, 1)); //turn left
		shipVelocity = normalize(shipVelocity);
		rotationHeading += 45;
		break;
	case GLUT_KEY_RIGHT:
		shipVelocity += cross(shipVelocity, point3(0, 0, 1)); // turn right
		shipVelocity = normalize(shipVelocity);
		rotationHeading -= 45;
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'p': case 'P':
		simulationPaused = !simulationPaused;
		if (singleStepMode)
			singleStepMode = FALSE;
		if (simulationPaused) {
			singleStepMode = TRUE;
		}
		break;
	case 'a': case 'A':  // speed down
		if (viewMode == 1 || viewMode == 3 || viewMode == 4) {
			shipSpeed -= 0.5;
			if (shipSpeed < 0)
				shipSpeed = 0;
		}
		break;
	case 'd': case 'D': // speed up
		if (viewMode == 1 || viewMode == 3 || viewMode == 4) {
			shipSpeed += 0.5;
		}
		break;
	case 'k': case 'K': // rotational speed down
		rotationalSpeed -= 0.2;
		if (rotationalSpeed < 0)
			rotationalSpeed = 0;
		break;
	case 'j': case 'J': // rotational speed up
		rotationalSpeed += 0.2;
		break;
	case 'c': case 'C':
		// ’the viewer is switched to the control desk
		viewMode = 1;
		break;
	case 's': case 'S':
		// the viewer is switched to the front of the station and looks in the direction of its front.
		viewMode = 2;
		break;
	case 't': case 'T':
		// the viewer is positioned behind and slightly above the spacehip.
		viewMode = 3;
		break;
	case 'w': case 'W':
		// position the viewer way up on z-axis
		viewMode = 4;
		eye = vec4(0, 0, 350, 1.0);
		at = vec4(0, 0, 0, 1.0);
		up = vec4(0.0, 1.0, 0.0, 0.0);
		break;

	case 'f': case 'F':
		// activate/deactivate autopilot mode
		autoPilotMode = !autoPilotMode;
		if (autoPilotMode) {
			cout << "autoPilotMode = True" << endl;
			simulationPaused = false;
			singleStepMode = false;
			autoAttackMode = false;
		}
		else {
			cout << "autoPilotMode = False" << endl;
			shipSpeed = 0;
		}
		break;
	case 'g': case 'G':
		// activate/deactivate autoAttackMode mode
		autoAttackMode = !autoAttackMode; // if autoAttackMode enabled, ship will attack target automatically
		if (autoAttackMode) {
			cout << "autoAttackMode = True" << endl;
			simulationPaused = false;
			singleStepMode = false;
			autoPilotMode = false;
		}
		else {
			cout << "autoAttackMode = False" << endl;
			shipSpeed = 0;
		}
		break;
	case 'h': case 'H': // if autoTargetChoose enabled, ship will find target automatically
		autoTargetChoose = !autoTargetChoose;
		if (autoTargetChoose)
			cout << "autoTargetChoose = True" << endl;
		else
			cout << "autoTargetChoose = False" << endl;
		break;
	case 'm': case 'M':
		cout << "score : " << score << endl;
		break;

	case '0': // target ship 0
		attackNo = 0;
		break;
	case '1': // target ship 1
		attackNo = 1;
		break;
	case '2': // target ship 2
		attackNo = 2;
		break;
	case '3': // target ship 3
		attackNo = 3;
		break;
	case '4': // target ship 4
		attackNo = 4;
		break;
	case '5': // target ship 5
		attackNo = 5;
		break;
	case '6': // target ship 6
		attackNo = 6;
		break;
	case '7': // target ship 7
		attackNo = 7;
		break;

	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);
	GLfloat aspect = GLfloat(width) / height;
	mat4  projection = Perspective(60.0, aspect, 20, 360.0);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
}

// timer callback
void timer(int value) {
	// simulation mode
	if (!simulationPaused) {
		if (!autoPilotMode && !autoAttackMode) {
			step();
		}
		// autopilot mode
		if (autoPilotMode) {
			vec3 u = stationPosition - shipPosition;
			GLfloat distance = sqrt(u.x * u.x + u.y * u.y);
			if (distance > 40) {
				u = stationPosition - shipPosition;
				distance = sqrt(u.x * u.x + u.y * u.y);
				while (!(shipVelocity.x * u.x > 0 && shipVelocity.y * u.y > 0)) {
					shipVelocity -= cross(shipVelocity, point3(0, 0, 1));
					shipVelocity = normalize(shipVelocity);
					rotationHeading += 45;
				}
				step();
			}
			else {
				cout << "Station found - autoPilotMode = False" << endl;
				shipSpeed = 0;
				autoPilotMode = FALSE;
			}

		}
		else if (autoAttackMode) {
			vec3 u = npc_positions[attackNo] - shipPosition; // calculate target-ship direction
			GLfloat distance = sqrt(u.x * u.x + u.y * u.y); // calculate target-ship distance

			vec3 v;
			v.x = planet_coords[attackNo][0] - npc_positions[attackNo].x; 
			v.y = planet_coords[attackNo][0] - npc_positions[attackNo].y; // calculate base planet-target direction
			v.z = planet_coords[attackNo][0] - npc_positions[attackNo].z;
			GLfloat distancePlanet = sqrt(v.x * v.x + v.y * v.y); // calculate base planet-target distance

			if (distance > 5 && distancePlanet > 30) {
				u = npc_positions[attackNo] - shipPosition;
				distance = sqrt(u.x * u.x + u.y * u.y);
				while (!(shipVelocity.x * u.x > 0 && shipVelocity.y * u.y > 0)) {
					shipVelocity -= cross(shipVelocity, point3(0, 0, 1));
					shipVelocity = normalize(shipVelocity);
					rotationHeading += 45;
				}
				step();
			}
			else {
				step();
				if (autoTargetChoose) {
					attackNo++;
					if (attackNo > 7) {
						attackNo = 0;
					}
				}
			}

		}
	}

	glutPostRedisplay();
	glutTimerFunc(1000.0 / 30.0, timer, 0); // set 30 fps
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
#endif

	glutInitWindowSize(1280, 720);
	//    glutInitContextVersion( 3, 2 );
	//    glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow("SpaceShip");
	glewExperimental = GL_TRUE;
	glewInit();

	init();
	srand(time(NULL));

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutSpecialFunc(SpecialInput);

	glutTimerFunc(300, timer, 0);

	glutMainLoop();
	return 0;
}
