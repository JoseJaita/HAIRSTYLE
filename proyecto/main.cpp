
#include "sketch2D.h"
#include "Angel-yjc.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <glm/glm.hpp>
#include <numeric>

//#define PI 3.14159265358979323846
//const float pi = acos(-1);

using namespace std;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec2  point2;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);


GLuint shader_1;       /* shader for single color */
GLuint shader_2;


// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.001, zFar = 105.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(0.0, 0.0, 3.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position
vec4 init_at(0.0, 0.0, 0.0, 1.0);
vec4 at = init_at;

float radio = 3.0;
float angle1 = 0.0;


//------------Head model variables--------------------
GLuint VAO_head;
GLuint VBO_head;
int n_v_head;
glm::vec3 head_color(0.56, 0.56, 0.56);
vector<glm::vec3> vertices_head;
vector<int> idx_vert_head;
vector<glm::vec3> points_head;

//------------ Hairstyle------------------------------
string filename_sketch;
vector<vector<glm::vec3>> hairstyle;
vector<glm::vec3> points_hairstyle;
char* file_hairstyle;
int n_v_hairstyle;
GLuint VAO_hairstyle;
GLuint VBO_hairstyle;
glm::vec3 hairstyle_color(1.0, 1.0, 0.0);

//------------- axis variables------------------------
GLuint VAO_axis;
GLuint VBO_axis_data;
const int axis_NumVertices = 6;
point3 axis_points[axis_NumVertices];
point3 axis_colors[axis_NumVertices];
point3 vertices_for_axis[] = {
  point3(0.0, 0.0, 0.0),
  point3(100.0, 0.0, 0.0),
  point3(0.0, 100.0, 0.0),
  point3(0.0, 0.0, 100.0)};
//-------------------------------------------------------
// RGBA colors
color3 vertex_colors[8] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0)   // cyan
};
void setPointsColorsAxis(){
  axis_points[0] = vertices_for_axis[0];
  axis_points[1] = vertices_for_axis[1]; // x
  axis_points[2] = vertices_for_axis[0];
  axis_points[3] = vertices_for_axis[2]; // y
  axis_points[4] = vertices_for_axis[0];
  axis_points[5] = vertices_for_axis[3]; // z
  
  axis_colors[0] = vertex_colors[1];
  axis_colors[1] = vertex_colors[1];
  axis_colors[2] = vertex_colors[5];
  axis_colors[3] = vertex_colors[5];
  axis_colors[4] = vertex_colors[4];
  axis_colors[5] = vertex_colors[4];  
}

/***************************************************************/
bool loadObj(const string& filename, vector<glm::vec3>& vertices,
	     vector<int>& idx_vertices){
  ifstream in (filename, ios::in);
  if(!in.is_open()){
    cout << "No se puedo leer el obj" << "\n";
    return false;
  }
 
  string line;
  getline(in,line); // header
  cout << line << "\n";
  while (getline(in, line)) {
    if (line.substr(0,2) == "v "){
      istringstream s (line.substr(2));
      glm::vec3 v;
      s >> v.x; s >> v.y; s >> v.z;
      //v.x *= 10; v.y *= 10; v.z *= 10; 
      v.y -= 1.7;
      v.x *= 10; v.y *= 10; v.z *= 10; 
      vertices.push_back(v);
      
    }
    else if (line.substr(0,2) == "f "){
      istringstream s (line.substr(2));
      string token;
      while (getline(s, token,' ')) {
	istringstream s1 (token);
	string s_v;
	getline(s1, s_v, '/'); //for now only read vertice for triangle
	idx_vertices.push_back(stoi(s_v)-1);
      }
    }
  }
}
//--------------------------------------------------------------------
bool readBinary(const char* filename, vector<vector<glm::vec3>>& strands){
  FILE *f = fopen(filename, "rb");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    return false; 
  }
  
  int nstrands = 0;
  if (!fread(&nstrands, 4, 1, f)) {
    fprintf(stderr, "Couldn't read number of strands\n");
    fclose(f);
    return false; 
  }
  strands.resize(nstrands);
  
  for (int i = 0; i < nstrands; i++) {
    int nverts = 0;
    if (!fread(&nverts, 4, 1, f)) {
      fprintf(stderr, "Couldn't read number of vertices\n");
      fclose(f);
      return false; 
    }
    if (nverts == 1){
      strands[i].resize(nverts);
      if (!fread(&strands[i][0][0], 12, 1, f)) {
	fprintf(stderr, "Couldn't read %d-th vertex in strand %d\n", 0, i);
	fclose(f);
	return false; 
      }
      continue;
    }

    strands[i].resize(nverts);
    for (int j = 0; j < nverts; j++) {
      if (!fread(&strands[i][j][0], 12, 1, f)) {
	fprintf(stderr, "Couldn't read %d-th vertex in strand %d\n", j, i);
	fclose(f);
	return false; 
      }
      //zoom to thread
      strands[i][j].y -= 1.7;
      strands[i][j].x *= 10; strands[i][j].y *= 10; strands[i][j].z *= 10;
    }
    /*
    strands[i].resize(nverts + nverts-2);

    for (int j = 0; j < nverts; j++) {
      int k;
      if (j == 0) k =0;
      else k = 2*(j-1)+1;
      
      if (!fread(&strands[i][k][0], 12, 1, f)) {
	fprintf(stderr, "Couldn't read %d-th vertex in strand %d\n", j, i);
	fclose(f);
	return false; 
      }
      // zoom to thread
      strands[i][k].y -= 1.7;
      strands[i][k].x *= 10; strands[i][k].y *= 10; strands[i][k].z *= 10;
      // complete points to draw
      if (j>0 && j<(nverts-1)){
	strands[i][k+1] = strands[i][k];
      }
      }*/
  }
  
  fclose(f);
  return true;
}

// read binary file .data
bool readHairStyle(const char* filename, vector<vector<glm::vec3>>& strands){
  if(!readBinary(filename, strands))
    return false;
  return true;
}

//--------------------------------------------------------------------
void setupHead(){
  n_v_head = idx_vert_head.size();
  for (int idx : idx_vert_head) {
    points_head.push_back(vertices_head[idx]);
  }
  for (auto i : idx_vert_head) {
    //cout << i.x <<","<<i.y<<","<<i.z << "\n";
    //cout << i << "\n";
  }
}

//---------------------------------------------------------------------
void applyHelixCurvature(const waves_t& waves){
  int n_waves = waves.size();

  for (auto& strand : hairstyle){
    if (strand.size()==1) continue;
  
    //auto& strand = hairstyle[0];
    int n_v = strand.size();
    vector<int> samples;
    int sample = int(n_v/n_waves);
    //cout << "sample size: "<<sample << "\n";
    for (int i = 0; i < n_waves; i++) {
      if (i == (n_waves-1))
	samples.push_back(n_v-accumulate(samples.begin(),samples.end(),0));
      else
	samples.push_back(sample);
    }
    glm::vec3 p1 = strand[0];
    for (int i = 0; i < samples.size(); i++) {
      int offset = sample*i;
      int s = samples[i]; // number of vertices
      // parametric equ 
      float a = waves[i].first;
      float b = waves[i].second;
      // pi*b in opengl world
      float d = strand[offset].y-strand[offset+s-1].y;
      // compute a but in opengl world
      float a_gl = a*d/(pi*b);
      float delta_t = pi/s;

      //cout << "------------------ "<< i<< "\n";
      //cout << a <<" "<<b<<" "<< a_gl<<" "<<delta_t<<""<<d << "\n";

      for (int j = 0; j < s; j++) {
	float add_x = a_gl*sin(delta_t*j); // TODO: a*sin(t)
	float add_z = a_gl*cos(delta_t*j);

	strand[offset+j].x += add_x;
	strand[offset+j].z += add_z;
      }
    }
    // reestablecer al punto inicial
    glm::vec3 p2 = strand[0];
    for (auto& v : strand)
      v = v + (p1-p2);
    
  }
}

//--------------------------------------------------------------------
void addDuplicate(){
  for (auto& strand : hairstyle) {
    if (strand.size()==1) continue;
    auto aux = strand;
    int n_v = strand.size();
    strand.clear();
    strand.resize(n_v*2-2);
    for (int i = 0; i < n_v; i++) {
      int k = (i==0) ? 0 : 2*(i-1)+1;
      strand[k] = aux[i];
      if (i>0 && i<(n_v-1))
	strand[k+1] = strand[k];
    }
  }
}

//---------------------------------------------------------------------
void setupHairStyle(){
  for (auto& strand : hairstyle) {
    if (strand.size() == 1){
      cout << "1" << "\n";
      
    }else{
      for (auto v : strand)
	points_hairstyle.push_back(v);
    }
  }
  n_v_hairstyle = points_hairstyle.size();
}


// OpenGL initialization
void init(){
  // loading shaders
  shader_1 = InitShader("shaders/vshader1.glsl", "shaders/fshader1.glsl");
  shader_2 = InitShader("shaders/vshader2.glsl", "shaders/fshader2.glsl");
  
  //-------------model head-------------
  loadObj("data/head_model.obj",vertices_head,idx_vert_head);
  setupHead();
  glGenVertexArrays(1, &VAO_head);         
  
  glGenBuffers(1, &VBO_head);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_head);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*n_v_head, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3)*n_v_head, &points_head[0]); // copy vertices
  //glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3)*floor_NumVertices, sizeof(point2)*floor_NumVertices, text_coord);
  
  glBindVertexArray(VAO_head);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //position
  glEnableVertexAttribArray(0);
  //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(point3)*floor_NumVertices)); //text coord
  //glEnableVertexAttribArray(1);
  
  
  //-----------------hairstyle---------------
  Sketch2D sketch(filename_sketch);
  waves_t waves;
  sketch.processing();
  waves = sketch.getWaves();

  readHairStyle(file_hairstyle, hairstyle);
  applyHelixCurvature(waves);
  addDuplicate();
  setupHairStyle();
  glGenVertexArrays(1, &VAO_hairstyle);
  glGenBuffers(1, &VBO_hairstyle);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_hairstyle);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*n_v_hairstyle, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(glm::vec3)*n_v_hairstyle, &points_hairstyle[0]);
  glBindVertexArray(VAO_hairstyle);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //position
  glEnableVertexAttribArray(0);
  //-----------------------------------------

  //-----------------axis-------------------
  setPointsColorsAxis();
  glGenVertexArrays(1, &VAO_axis);
  glGenBuffers(1, &VBO_axis_data);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_axis_data);
  glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points)+sizeof(axis_colors), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors), axis_colors);
  glBindVertexArray(VAO_axis);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0); // link positions
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(axis_points));
  glEnableVertexAttribArray(1); // link colors
  //----------------------------------------------
  
  // opengl configuration  
  glEnable( GL_DEPTH_TEST );
  glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
  glLineWidth(2.0);
}


void display( void ){
  GLuint  modelView_id;
  GLuint  projection_id;
  
  //set background color
  glClearColor(0.529,0.807,0.92,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  // matrix perpective and matrix view
  mat4 p = Perspective(fovy, aspect, zNear, zFar); //matrix perpective
  //vec4 at(0.0, 0.0, 0.0, 1.0);
  vec4 up(0.0, 1.0, 0.0, 0.0);
  mat4 view = LookAt(eye, at, up);                   //matrix view

  
  //-----------------draw head model ---------------------------
  glUseProgram(shader_1); // Use the shader program
  modelView_id = glGetUniformLocation(shader_1, "model_view" );
  projection_id = glGetUniformLocation(shader_1, "projection" );
  
  glUniform3fv(glGetUniformLocation(shader_1,"vert_color"),1, &head_color[0]);
  glUniformMatrix4fv(modelView_id, 1, GL_TRUE, view);
  glUniformMatrix4fv(projection_id, 1, GL_TRUE, p);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBindVertexArray(VAO_head);
  glDrawArrays(GL_TRIANGLES, 0, n_v_head);

  //----------------- draw hairstyle----------------------------
  glUniform3fv(glGetUniformLocation(shader_1,"vert_color"),1, &hairstyle_color[0]);
  glBindVertexArray(VAO_hairstyle);
  //glDrawArrays(GL_LINES, 0, n_v_hairstyle);
  glDrawArrays(GL_LINES, 0, 198);
  
  //------------------draw axis---------------------------------
  glUseProgram(shader_2); // Use the shader program
  glUniformMatrix4fv(glGetUniformLocation(shader_2, "projection"), 1, GL_TRUE, p);
  glUniformMatrix4fv(glGetUniformLocation(shader_2, "model_view"), 1, GL_TRUE, view);
  glBindVertexArray(VAO_axis);
  glDrawArrays(GL_LINES, 0, axis_NumVertices);
  
  glutSwapBuffers();

}
//---------------------------------------------------------------------------
void idle (void){
  //angle += 0.02;
  //angle += delta_angle;  //YJC: change this value to adjust the cube rotation speed.
  
  glutPostRedisplay();
}

//---------------------------------------------------------------------------


//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y){
  switch(key) {
  case 033: // Escape Key
  case 'q': case 'Q':
    exit( EXIT_SUCCESS );
    break;
    
  case 'X':
    //eye[0] -= 1;
    angle1 += 1.0;
    eye[2] = radio*cos(angle1*pi/180);
    eye[0] = radio*sin(angle1*pi/180);
    break;
  case 'x':
    //eye[0] += 1;
    angle1 -= 1.0;
    eye[2] = radio*cos(angle1*pi/180);
    eye[0] = radio*sin(angle1*pi/180);
    break;

  case 'Y':
    //eye[1] -= 1;
    radio -= 0.2;
    eye[2] = radio*cos(angle1*pi/180);
    eye[0] = radio*sin(angle1*pi/180);
    break;
  case 'y':
    // eye[1] += 1;
    radio += 0.2;
    eye[2] = radio*cos(angle1*pi/180);
    eye[0] = radio*sin(angle1*pi/180);
    break;

  case 'Z': eye[2] -= 1; break;
  case 'z': eye[2] += 1; break;
    

    
  }
  glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char **argv){
  if (argc < 2)
    return -1;
  file_hairstyle = argv[1];
  filename_sketch = argv[2];
  
  int err;
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(600, 600);
  // glutInitContextVersion(3, 2);
  // glutInitContextProfile(GLUT_CORE_PROFILE);
  glutCreateWindow("Color Cube");
  
  /* Call glewInit() and error checking */
  err = glewInit();
  if (GLEW_OK != err){
    printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
    exit(1);
  }
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(NULL);
  glutKeyboardFunc(keyboard);
  
  init();
  glutMainLoop();
  return 0;
}
