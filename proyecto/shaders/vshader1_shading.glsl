#version 120

attribute vec3 vPosition;
attribute vec3 vNormal_flat;
attribute vec3 vNormal_smooth;

varying vec3 vert_color;

uniform int flat_smooth_flag;
uniform int point_spot_flag;

uniform mat4 model_view;
uniform mat4 projection;
uniform mat3 normal_matrix_eye;

uniform vec3 material_ambient_color;
uniform vec3 material_diffuse_color;
uniform vec3 material_specular_color;
uniform float shininess;

// to spot or point light
uniform vec3 light_ambient_color;
uniform vec3 light_diffuse_color;
uniform vec3 light_specular_color;

uniform vec3 light_position;
uniform vec3 light_direction_to;
uniform float spot_angle;
uniform float spot_exp;

uniform float const_atte;  
uniform float linear_atte; 
uniform float quad_atte;   

// to directional light
uniform vec3 global_light_color; // material solo respodera con su componente ambiental
uniform vec3 l_d_ambient_color;
uniform vec3 l_d_diffuse_color;
uniform vec3 l_d_specular_color;
uniform vec3 l_d_direction; //vector


void main() {
  vec3 normal;
  if (flat_smooth_flag == 0)
    normal = normal_matrix_eye * vNormal_flat;
  else
    normal = normal_matrix_eye * vNormal_smooth;
  
  normal = normalize(normal);
  vec3 position = (model_view * vec4(vPosition, 1.0)).xyz;
  
  vec3 a,b,c;
  float diff, spec;

  //global ambient light
  vec3 color_global_light= global_light_color*material_ambient_color;
  
  // directional light
  a = normalize(-l_d_direction);
  b = normalize(-position);
  c = normalize(a+b);
  diff = max(dot(a,normal),0.0);
  spec =  pow(max(dot(normal,c),0.0),shininess);
  
  vec3 ambient = l_d_ambient_color*material_ambient_color;
  vec3 diffuse = l_d_diffuse_color*(diff*material_diffuse_color);
  vec3 specular = l_d_specular_color*(spec*material_specular_color);
  vec3 color_directional = ambient + diffuse + specular;

  // spot or point light
  // computing attenuation
  a = light_position - position;  
  float d = length(a);
  float attenuation = 1/(const_atte + linear_atte*d + quad_atte*d*d);

  a = normalize(a);
  // b already was computed
  c = normalize(a + b);
  diff = max(dot(a,normal),0.0);
  spec =  pow(max(dot(normal,c),0.0),shininess);
  
  ambient = light_ambient_color*material_ambient_color;
  diffuse = light_diffuse_color*(diff*material_diffuse_color);
  specular = light_specular_color*(spec*material_specular_color);

  vec3 color_light_point = attenuation*(ambient + diffuse + specular);

  // spot or point point_spot_flat = 0-> point light
  if (point_spot_flag == 0){
    vert_color = color_global_light + color_directional + color_light_point;
  }
  else{
    a = -a;
    b = normalize(light_direction_to-light_position);
    float k = pow(dot(a,b),spot_exp);
    vec3 color_light_spot = k*color_light_point;
    if (dot(a,b)<spot_angle){
      color_light_spot = vec3(0.0,0.0,0.0);
    }
    vert_color = color_global_light + color_directional + color_light_spot;
  }
  
  gl_Position = projection * vec4(position,1.0);  
} 
