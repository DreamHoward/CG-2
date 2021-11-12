/*

	CG Homework2 - Shadow Mapping & Dissolve Effects

	Objective - learning Shadow Implmentation and Dissolve Effects

	Overview:

		1. Render the model with Shadow using shadow mapping

		2. Implement dissolve effect

	!!!IMPORTANT!!! 

	1. Make sure to change the window name to your student ID!
	2. You are allow to use glmDraw this time.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "../shader_lib/shader.h"
#include "glm/glm.h"
//my include
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include "../shader_lib/shader_s.h"


extern "C"
{
	#include "glm_helper.h"
}

/*you may need to do something here
you may use the following struct type to perform your single VBO method,
or you can define/declare multiple VBOs for VAO method.
Please feel free to modify it*/
struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};
typedef struct Vertex Vertex;

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void draw_light_bulb(void);
void camera_light_ball_move();
GLuint loadTexture(char* name, GLfloat width, GLfloat height);


namespace
{
	char *obj_file_dir = "../Resources/Ball.obj";
	char *obj_file_dir2 = "../Resources/bunny.obj";
	char *main_tex_dir = "../Resources/Stone.ppm";
	char *floor_tex_dir = "../Resources/WoodFine.ppm";
	char *plane_file_dir = "../Resources/Plane.obj";
	char *noise_tex_dir = "../Resources/noise.ppm";
	
	GLfloat light_rad = 0.05; //radius of the light bulb
	float eyet = -5.59; //theta in degree
	float eyep = 83.2; //phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;

	int mousex = 0;
	int mousey = 0;
	
	
}

// You can modify the moving/rotating speed if it's too fast/slow for you
const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

//you may need to use some of the following variables in your program 

// No need for model texture, 'cause glmModel() has already loaded it for you.
// To use the texture, check glmModel documentation.
GLuint mainTextureID; // TA has already loaded this texture for you
GLuint floorTextureID;
GLuint noiseTextureID;

GLMmodel *model; // TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)
GLMmodel *planeModel;
GLMmodel *subModel;

float eyex = -3.291;
float eyey = 1.57;
float eyez = 11.89;

GLfloat light_pos[] = { 1.1, 3.5, 1.3 };
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };
GLfloat plane_pos[] = { 0.0, -5.0, 0.0 };
GLfloat plane_rot[] = { 0.0, 0.0, 0.0 };
GLfloat subModel_pos[] = { -2.295, -5.0, -2.0 };
GLfloat subModel_rot[] = { 0.0, 0.0, 0.0 };

//my global var
std::string shaderSource;
const GLchar* vss;  //vertexShaderSource
const GLchar* fss;  //fragmentShaderSource
GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;
GLuint shaderProgram1;
Vertex *vertices;
GLuint vaoHandle_4;
GLuint vbo_ids_4;
GLuint vaoHandle_5;
GLuint vbo_ids_5;
GLuint vaoHandle_6;
GLuint vbo_ids_6;
GLuint vaoHandle_1;
GLuint vbo_ids_1;
GLuint vaoHandle_2;
GLuint vbo_ids_2;
GLuint vaoHandle_3;
GLuint vbo_ids_3;
//GLuint framebuffer, renderedTexture;
GLuint depthMapFBO , depthMap;
GLfloat dissolveFactor=0.0;
GLfloat ky=0.0;

void shaderReader(std::string path)
{
	std::ifstream file;
	file = std::ifstream(path);
	std::string lineBuffer;
	std::string buffer;
	if (!file.good())
	{
		std::cout << path << " dose not exists." << std::endl;
		return;
	}
	else
	{
		std::cout << path << " is OK." << std::endl;
	}
	while (std::getline(file, buffer))
	{
		lineBuffer += buffer + "\n";
	}
	shaderSource = lineBuffer;
}
void S_create(GLMmodel *model, GLuint &vaoHandle, GLuint &vbo_ids) {
	//頂點著色
	shaderReader("../shader/screen.vs");
	vss = shaderSource.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vss, NULL);
	glCompileShader(vertexShader);
	//測試有沒有成功
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//片段著色     
	shaderReader("../shader/screen.frag");
	fss = shaderSource.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fss, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//shader Program

	shaderProgram1 = glCreateProgram();
	glAttachShader(shaderProgram1, vertexShader);
	glAttachShader(shaderProgram1, fragmentShader);
	glLinkProgram(shaderProgram1);
	//測試有沒有成功
	glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram1, 512, NULL, infoLog);
	}
	glUseProgram(shaderProgram1);

	//deleteShader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//-----------------------------------VAO VBO----------------------------
	unsigned size = model->numtriangles * 3 * sizeof(Vertex); //大小
	vertices = (Vertex*)malloc(size);
	for (int a = 0; a < model->numtriangles; a++) {
		//拿到每個頂點
		int vIndex[3] = { model->triangles[a].vindices[0], model->triangles[a].vindices[1] ,model->triangles[a].vindices[2] };
		int tIndex[3] = { model->triangles[a].tindices[0], model->triangles[a].tindices[1] ,model->triangles[a].tindices[2] };
		int nIndex[3] = { model->triangles[a].nindices[0], model->triangles[a].nindices[1], model->triangles[a].nindices[2] };
		//點的八個值
		for (int j = 0; j < 3; j++) {
			vertices[a * 3 + j].position[0] = model->vertices[vIndex[j] * 3];
			vertices[a * 3 + j].position[1] = model->vertices[vIndex[j] * 3 + 1];
			vertices[a * 3 + j].position[2] = model->vertices[vIndex[j] * 3 + 2];

			vertices[a * 3 + j].texcoord[0] = model->texcoords[tIndex[j] * 2];
			vertices[a * 3 + j].texcoord[1] = model->texcoords[tIndex[j] * 2 + 1];

			vertices[a * 3 + j].normal[0] = model->normals[nIndex[j] * 3];
			vertices[a * 3 + j].normal[1] = model->normals[nIndex[j] * 3 + 1];
			vertices[a * 3 + j].normal[2] = model->normals[nIndex[j] * 3 + 2];
		}
	}

	glGenVertexArrays(1, &vaoHandle);                 //創建並綁定VAO
	glBindVertexArray(vaoHandle);
	glGenBuffers(1, &vbo_ids);                          //創建並綁定VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ids);

	glBufferData(GL_ARRAY_BUFFER,                     //分配空間傳送數據
		size,
		vertices,
		GL_STATIC_DRAW);       //GL_STREAM_DRAW, GL_DYNAMIC_DRAW      

	//指定GPU解析數據的方式
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	glVertexAttribPointer(0, 3,                     // # of components ex: (x,y,z) 
		GL_FLOAT,              // type of components 
		GL_FALSE,              // normalized
		sizeof(Vertex),                     // stride 
		(void*)0 // offset 
	);

	glVertexAttribPointer(1, 3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(offsetof(Vertex, normal))
	);
	glVertexAttribPointer(2, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(offsetof(Vertex, texcoord))
	);
	glBindVertexArray(0);
}
void V_create(GLMmodel *model, GLuint &vaoHandle, GLuint &vbo_ids) {
	//頂點著色
	shaderReader("../shader/vertShader.txt");
	vss = shaderSource.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vss, NULL);
	glCompileShader(vertexShader);
	//測試有沒有成功
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	//片段著色     
	shaderReader("../shader/fragShader.txt");
	fss = shaderSource.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fss, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//shader Program

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	//測試有沒有成功
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	}
	glUseProgram(shaderProgram);

	//deleteShader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//-----------------------------------VAO VBO----------------------------
	unsigned size = model->numtriangles * 3 * sizeof(Vertex); //大小
	vertices = (Vertex*)malloc(size);
	for (int a = 0; a < model->numtriangles; a++) {
		//拿到每個頂點
		int vIndex[3] = { model->triangles[a].vindices[0], model->triangles[a].vindices[1] ,model->triangles[a].vindices[2] };
		int tIndex[3] = { model->triangles[a].tindices[0], model->triangles[a].tindices[1] ,model->triangles[a].tindices[2] };
		int nIndex[3] = { model->triangles[a].nindices[0], model->triangles[a].nindices[1], model->triangles[a].nindices[2] };
		//點的八個值
		for (int j = 0; j < 3; j++) {
			vertices[a * 3 + j].position[0] = model->vertices[vIndex[j] * 3];
			vertices[a * 3 + j].position[1] = model->vertices[vIndex[j] * 3 + 1];
			vertices[a * 3 + j].position[2] = model->vertices[vIndex[j] * 3 + 2];

			vertices[a * 3 + j].texcoord[0] = model->texcoords[tIndex[j] * 2];
			vertices[a * 3 + j].texcoord[1] = model->texcoords[tIndex[j] * 2 + 1];

			vertices[a * 3 + j].normal[0] = model->normals[nIndex[j] * 3];
			vertices[a * 3 + j].normal[1] = model->normals[nIndex[j] * 3 + 1];
			vertices[a * 3 + j].normal[2] = model->normals[nIndex[j] * 3 + 2];
		}
	}
	
	glGenVertexArrays(1, &vaoHandle);                 //創建並綁定VAO
	glBindVertexArray(vaoHandle);
	glGenBuffers(1, &vbo_ids);                          //創建並綁定VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ids);

	glBufferData(GL_ARRAY_BUFFER,                     //分配空間傳送數據
		size,
		vertices,
		GL_STATIC_DRAW);       //GL_STREAM_DRAW, GL_DYNAMIC_DRAW      

	//指定GPU解析數據的方式
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);


	glVertexAttribPointer(0, 3,                     // # of components ex: (x,y,z) 
		GL_FLOAT,              // type of components 
		GL_FALSE,              // normalized
		sizeof(Vertex),                     // stride 
		(void*)0 // offset 
	);

	glVertexAttribPointer(1, 3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(offsetof(Vertex, normal))
	);
	glVertexAttribPointer(2, 2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void*)(offsetof(Vertex, texcoord))
	);
	glBindVertexArray(0);
}

// Generates a texture that is suited for attachments to a framebuffer
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
	// What enum to use?
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGB;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!depth && !stencil)
		glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, 512, 512, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 512, 512, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	// remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("CG_HW2_0856641");
	glutReshapeWindow(512, 512);

	glewInit();

	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	
	glEnable(GL_CULL_FACE);

	mainTextureID = loadTexture(main_tex_dir, 1024, 1024);
	floorTextureID = loadTexture(floor_tex_dir, 512, 512);
	noiseTextureID = loadTexture(noise_tex_dir, 320, 320);

	model = glmReadOBJ(obj_file_dir);
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	planeModel = glmReadOBJ(plane_file_dir);
	glmFacetNormals(planeModel);
	glmVertexNormals(planeModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(planeModel);	

	subModel = glmReadOBJ(obj_file_dir2);
	glmFacetNormals(subModel);
	glmVertexNormals(subModel, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(subModel);

	// you may need to do something here(create shaders/program(s) and create vbo(s)/vao from GLMmodel model)

	/*glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLuint textureColorbuffer = generateAttachmentTexture(true, false);// Generates a texture that is suited for attachments to a framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureColorbuffer, 0);*/
	V_create(model, vaoHandle_1, vbo_ids_1);
	V_create(planeModel, vaoHandle_2, vbo_ids_2);
	V_create(subModel, vaoHandle_3, vbo_ids_3);
	S_create(model, vaoHandle_4, vbo_ids_4);
	S_create(planeModel, vaoHandle_5, vbo_ids_5);
	S_create(subModel, vaoHandle_6, vbo_ids_6);

	glViewport(0, 0, 512, 512);
	glEnable(GL_DEPTH_TEST);

	//Set texture samples
	GLint dt = glGetUniformLocation(shaderProgram1, "diffuseTexture");
	glUniform1i(dt, 4);
	GLint sm = glGetUniformLocation(shaderProgram, "shadowMap");
	glUniform1i(sm, 3);

	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,512, 512, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	
}
	

	//--------------------
	// APIs for creating shaders and creating shader programs have been done by TAs
	// following is an example for creating a shader program using given vertex shader and fragment shader
	/*
		GLuint vert = createShader("Shaders/phong.vert", "vertex");
		GLuint frag = createShader("Shaders/phong.frag", "fragment");
		GLuint program = createProgram(vert, frag);
	*/

void shadow() {
	
	GLfloat near_plane = 0.01f, far_plane = 11.0f;

	glUseProgram(shaderProgram1);

	//拿P
	glMatrixMode(GL_PROJECTION);
	GLfloat projection[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	GLint projLoc = glGetUniformLocation(shaderProgram1, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	//拿lightProjection
		glLoadIdentity();
		glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		GLfloat lightProjection[16];
		glGetFloatv(GL_PROJECTION_MATRIX, lightProjection);
		GLint lp = glGetUniformLocation(shaderProgram1, "lightProjection");
		glUniformMatrix4fv(lp, 1, GL_FALSE, lightProjection);
	glPopMatrix();

	//拿lightView
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		gluLookAt(
			light_pos[0],light_pos[1],light_pos[2],
			0.0 , 0.0 , 0.0,
			0.0 , 1.0 , 0.0);
		GLfloat lightView[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, lightView);
		GLint lv = glGetUniformLocation(shaderProgram1, "lightView");
		glUniformMatrix4fv(lv, 1, GL_FALSE, lightView);
	glPopMatrix();
		
		glEnable(GL_TEXTURE_2D);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		gluLookAt(
			eyex,
			eyey,
			eyez,
			eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180),
			eyey + sin(eyet*M_PI / 180),
			eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
			0.0,
			1.0,
			0.0);
	
		//拿view
		GLfloat view[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, view);
		GLint vLoc = glGetUniformLocation(shaderProgram1, "view");
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, view);
	glPopMatrix();
		//model
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);

		GLfloat Model[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, Model);
		GLint modelLoc = glGetUniformLocation(shaderProgram1, "Model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, Model);

		GLint lightPos = glGetUniformLocation(shaderProgram1, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram1, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);

		
		GLint loc2 = glGetUniformLocation(shaderProgram1, "shadowMap");
		GLint loc3 = glGetUniformLocation(shaderProgram1, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		GLint loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, dissolveFactor);
		

		glBindVertexArray(vaoHandle_4);
		glDrawArrays(GL_TRIANGLES, 0, model->numtriangles * 3);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);
		
		
	glPopMatrix();
		//planeModel
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(plane_pos[0], plane_pos[1], plane_pos[2]);
		glRotatef(plane_rot[0], 1, 0, 0);
		glRotatef(plane_rot[1], 0, 1, 0);
		glRotatef(plane_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);

		Model[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, Model);
		modelLoc = glGetUniformLocation(shaderProgram1, "Model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, Model);

		lightPos = glGetUniformLocation(shaderProgram1, "lightPos");
		viewPosLoc = glGetUniformLocation(shaderProgram1, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);

		loc2 = glGetUniformLocation(shaderProgram1, "shadowMap");
		loc3 = glGetUniformLocation(shaderProgram1, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, 0.0);
		


		glBindVertexArray(vaoHandle_5);
		glDrawArrays(GL_TRIANGLES, 0, planeModel->numtriangles * 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);

	glPopMatrix();
		//subModel
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(subModel_pos[0], subModel_pos[1], subModel_pos[2]);
		glRotatef(subModel_rot[0], 1, 0, 0);
		glRotatef(subModel_rot[1], 0, 1, 0);
		glRotatef(subModel_rot[2], 0, 0, 1);

		Model[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, Model);
		modelLoc = glGetUniformLocation(shaderProgram1, "Model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, Model);

		lightPos = glGetUniformLocation(shaderProgram1, "lightPos");
		viewPosLoc = glGetUniformLocation(shaderProgram1, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);

		loc2 = glGetUniformLocation(shaderProgram1, "shadowMap");
		loc3 = glGetUniformLocation(shaderProgram1, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, 0.0);
		

		glBindVertexArray(vaoHandle_6);
		glDrawArrays(GL_TRIANGLES, 0, subModel->numtriangles * 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);
	glPopMatrix();
		
		glUseProgram(NULL);

}
	

void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadow();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0 + 0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + 1);
	glDisable(GL_TEXTURE_2D);
	
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		eyex,
		eyey,
		eyez,
		eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180),
		eyey + sin(eyet*M_PI / 180),
		eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
		0.0,
		1.0,
		0.0);
		glColor3f(1, 1, 1);
		draw_light_bulb();
	glPushMatrix();
	glUseProgram(shaderProgram);
	//you may need to do something here(declare some local variables you need and maybe load Model matrix here...)
	glActiveTexture(GL_TEXTURE0 + 0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0 + 1);
	glEnable(GL_TEXTURE_2D);



	
	//please try not to modify the following block of code(you can but you are not supposed to)

		//拿view
		GLfloat view[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, view);
		GLint vLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, view);
	
	//拿P
	glMatrixMode(GL_PROJECTION);
	GLfloat projection[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
	glPushMatrix();

	//拿lightProjection
	glLoadIdentity();
	glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 11.0f);
	GLfloat lightProjection[16];
	glGetFloatv(GL_PROJECTION_MATRIX, lightProjection);
	GLint lp = glGetUniformLocation(shaderProgram, "lightProjection");
	glUniformMatrix4fv(lp, 1, GL_FALSE, lightProjection);
	glPopMatrix();

	//拿lightView
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		light_pos[0], light_pos[1], light_pos[2],
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
	GLfloat lightView[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, lightView);
	GLint lv = glGetUniformLocation(shaderProgram, "lightView");
	glUniformMatrix4fv(lv, 1, GL_FALSE, lightView);
	glPopMatrix();

		//model
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
		glRotatef(ball_rot[0], 1, 0, 0);
		glRotatef(ball_rot[1], 0, 1, 0);
		glRotatef(ball_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);
		
		GLfloat modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		GLint modelLoc = glGetUniformLocation(shaderProgram, "modelview");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelview);

		GLint lightPos = glGetUniformLocation(shaderProgram, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);

		
		GLint loc = glGetUniformLocation(shaderProgram, "MyTexture_1");
		GLint loc2 = glGetUniformLocation(shaderProgram, "shadowMap");
		GLint loc3 = glGetUniformLocation(shaderProgram, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, mainTextureID);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc, 0);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		//dissolve
		GLint loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, dissolveFactor);
		GLint loc5 = glGetUniformLocation(shaderProgram, "ky");
		glUniform1f(loc5, ky);


		glBindVertexArray(vaoHandle_1);
		glDrawArrays(GL_TRIANGLES, 0, model->numtriangles * 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);
		// you may need to do something here(pass uniform variable(s) to shader and render the model)
		//glmDraw(model, GLM_TEXTURE);
	glPopMatrix();
	//planemodel
	
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(plane_pos[0], plane_pos[1], plane_pos[2]);
		glRotatef(plane_rot[0], 1, 0, 0);
		glRotatef(plane_rot[1], 0, 1, 0);
		glRotatef(plane_rot[2], 0, 0, 1);
		glColor3f(1, 1, 1);

		modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		modelLoc = glGetUniformLocation(shaderProgram, "modelview");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelview);

		lightPos = glGetUniformLocation(shaderProgram, "lightPos");
		viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);

		
		loc = glGetUniformLocation(shaderProgram, "MyTexture_1");
		loc2 = glGetUniformLocation(shaderProgram, "shadowMap");
		loc3 = glGetUniformLocation(shaderProgram, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, floorTextureID);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc, 1);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		//dissolve
		loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, 0.0);
		loc5 = glGetUniformLocation(shaderProgram, "ky");
		glUniform1f(loc5, ky);

		glBindVertexArray(vaoHandle_2);
		glDrawArrays(GL_TRIANGLES, 0, planeModel->numtriangles * 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);
		
		// you may need to do something here(pass uniform variable(s) to shader and render the model)
		//glmDraw(planeModel, GLM_TEXTURE);
	glPopMatrix();
	//submodel
	
	glPushMatrix();
	//重置
		glLoadIdentity();
		glTranslatef(subModel_pos[0], subModel_pos[1], subModel_pos[2]);
		glRotatef(subModel_rot[0], 1, 0, 0);
		glRotatef(subModel_rot[1], 0, 1, 0);
		glRotatef(subModel_rot[2], 0, 0, 1);	
		
		modelview[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		modelLoc = glGetUniformLocation(shaderProgram, "modelview");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelview);

		lightPos = glGetUniformLocation(shaderProgram, "lightPos");
		viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
		glUniform3f(viewPosLoc, eyex, eyey, eyez);
		glUniform3f(lightPos, light_pos[0], light_pos[1], light_pos[2]);


		loc = glGetUniformLocation(shaderProgram, "MyTexture_1");
		loc2 = glGetUniformLocation(shaderProgram, "shadowMap");
		loc3 = glGetUniformLocation(shaderProgram, "noiseTextureID");
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, mainTextureID);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, noiseTextureID);
		glUniform1i(loc, 0);
		glUniform1i(loc2, 3);
		glUniform1i(loc3, 2);
		//dissolve
		loc4 = glGetUniformLocation(shaderProgram, "dissolveFactor");
		glUniform1f(loc4, 0.0);
		loc5 = glGetUniformLocation(shaderProgram, "ky");
		glUniform1f(loc5, ky);

		glBindVertexArray(vaoHandle_3);
		glDrawArrays(GL_TRIANGLES, 0, subModel->numtriangles * 3);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, NULL);
	glPopMatrix();
		// you may need to do something here(pass uniform variable(s) to shader and render the model)
		//glmDraw(subModel, GLM_TEXTURE);


	
	glUseProgram(NULL);

	glutSwapBuffers();
	camera_light_ball_move();
}

// please implement mode increase/decrease dissolve threshold in case '-' and case '=' (lowercase)
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	// ESC
		break;
	}
	case '-': // increase dissolve threshold
	{
		// you may need to do somting here
		dissolveFactor+=0.05;
		if (dissolveFactor > 1)
			dissolveFactor = 1.0;
		break;
	}
	case '=': // decrease dissolve threshold
	{
		// you may need to do somting here
		dissolveFactor -= 0.05;
		if (dissolveFactor < 0)
			dissolveFactor = 0.0;
		break;
	}
	case 'd':
	{
		right = true;
		break;
	}
	case 'a':
	{
		left = true;
		break;
	}
	case 'w':
	{
		forward = true;
		break;
	}
	case 's':
	{
		backward = true;
		break;
	}
	case 'q':
	{
		up = true;
		break;
	}
	case 'e':
	{
		down = true;
		break;
	}
	case 't':
	{
		lforward = true;
		break;
	}
	case 'g':
	{
		lbackward = true;
		break;
	}
	case 'h':
	{
		lright = true;
		break;
	}
	case 'f':
	{
		lleft = true;
		break;
	}
	case 'r':
	{
		lup = true;
		break;
	}
	case 'y':
	{
		ldown = true;
		break;
	}
	case 'i':
	{
		bforward = true;
		break;
	}
	case 'k':
	{
		bbackward = true;
		break;
	}
	case 'l':
	{
		bright = true;
		break;
	}
	case 'j':
	{
		bleft = true;
		break;
	}
	case 'u':
	{
		bup = true;
		break;
	}
	case 'o':
	{
		bdown = true;
		break;
	}
	case '7':
	{
		bx = true;
		break;
	}
	case '8':
	{
		by = true;
		break;
	}
	case '9':
	{
		bz = true;
		break;
	}
	case '4':
	{
		brx = true;
		break;
	}
	case '5':
	{
		bry = true;
		break;
	}
	case '6':
	{
		brz = true;
		break;
	}
	case 'm':
	{
		ky+=1.0;
		ky = (ky > 1.0) ? 0.0 : 1.0;
		break;
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		light_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180);
		light_pos[1] = eyey + sin(eyet*M_PI / 180);
		light_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180);
		break;
	}
	case 'x'://move ball to front of camera
	{
		ball_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet*M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		light_pos[0] = 1.1;
		light_pos[1] = 3.5;
		light_pos[2] = 1.3;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = -3.291;
		eyey = 1.57;
		eyez = 11.89;
		eyet = -5.59; //theta in degree
		eyep = 83.2; //phi in degree
		break;
	}
	default:
	{
		break;
	}
	}
}

//no need to modify the following functions
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	if (mleft)
	{
		eyep -= (x-mousex)*0.1;
		eyet -= (y - mousey)*0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}

void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz=0;
	if(left|| right || forward || backward || up || down)
	{ 
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		eyey += dy*sin(eyet*M_PI / 180);
		eyez += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if(lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		light_pos[1] += dy*sin(eyet*M_PI / 180);
		light_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if(ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy*cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx*sin(eyep*M_PI / 180);
		ball_pos[1] += dy*sin(eyet*M_PI / 180);
		ball_pos[2] += dy*(-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx*cos(eyep*M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if(bx||by||bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}

void draw_light_bulb()
{
	GLUquadric *quad;
	quad = gluNewQuadric();
	glPushMatrix();
	glColor3f(0.4, 0.5, 0);
	glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
	gluSphere(quad, light_rad, 40, 20);
	glPopMatrix();
}

void keyboardup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
	{
		right =false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}

void idle(void)
{
	subModel_rot[1] += 1;
	glutPostRedisplay();
}

GLuint loadTexture(char* name, GLfloat width, GLfloat height)
{
	return glmLoadTexture(name, false, true, true, true, &width, &height);
}
