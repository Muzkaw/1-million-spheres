#pragma once

const char* const sphereClusterVertexShaderSource = R"(
#version 430

//Attributes
layout(location = 0) in vec3 position;
layout(location = 1) in float radius;
layout(location = 2) in vec3 color;

//Outputs
out vec3 vColor;
out float vRadius;
out vec3 vSpherePos;

//Uniforms
uniform mat4 model;

void main() 
{ 
	vColor = color;
	vRadius = radius;
	vSpherePos = vec3(model*vec4(position,1.0));
}
)";

const char* const sphereClusterGeometryShaderSource = R"(
#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColor[];
in float vRadius[];
in vec3 vSpherePos[];

smooth out vec3 fColor;
smooth out vec3 fFragPos;
flat out vec3 fSpherePos;
flat out float fSphereRadiusSq;

//Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 viewinv;

void main()
{
	fColor = vColor[0];
	fSpherePos = vSpherePos[0];
	fSphereRadiusSq = vRadius[0]*vRadius[0];

	vec3 direction = vSpherePos[0] - viewinv[3].xyz; //sphere center - cam position

	vec3 Y = normalize(cross(viewinv[0].xyz,direction)); //cross cam X, direction
	vec3 X = normalize(cross(Y,direction)); //cross cam Y, direction
	
	float L2 = dot(direction,direction);
	float Rp = sqrt(L2)*vRadius[0]/sqrt(L2-fSphereRadiusSq);

	fFragPos = vSpherePos[0] + Rp*(-X-Y);
	gl_Position = proj*view * vec4(fFragPos, 1.0);
	EmitVertex();

	fFragPos = vSpherePos[0] + Rp*(-X+Y);
	gl_Position = proj*view * vec4(fFragPos, 1.0);
	EmitVertex();

	fFragPos = vSpherePos[0] + Rp*(+X-Y);
	gl_Position = proj*view * vec4(fFragPos, 1.0);
	EmitVertex();

	fFragPos = vSpherePos[0] + Rp*(+X+Y);
	gl_Position = proj*view * vec4(fFragPos, 1.0);
	EmitVertex();
	
	EndPrimitive();
}
)";


const char* const sphereClusterFragmentShaderSource = R"(
#version 430

//Attributes = Ouputs from vertex shader
smooth in vec3 fColor;
smooth in vec3 fFragPos;
flat in vec3 fSpherePos;
flat in float fSphereRadiusSq;

//Outputs
out vec4 outColor;

//Uniforms
uniform vec3 lightPos;
uniform mat4 viewinv;

void main() 
{
	vec3 camPos = viewinv[3].xyz;
	vec3 camZ = viewinv[2].xyz;
	
	vec3 ray = normalize(fFragPos - camPos);

	vec3 L = fSpherePos-camPos;
	float tca = dot(ray,L);

	if(tca<0) discard;
	else
	{
		float d2 = dot(L,L) - tca*tca;
		if(d2 >= fSphereRadiusSq) discard;
		else
		{
			float thc = sqrt(fSphereRadiusSq - d2);
	
			vec3 hitpos = ray * (tca-thc) + camPos;

			vec3 lightColor = vec3(1.0,1.0,1.0);
			float ambientStrength = 0.4;
			vec3 ambient = ambientStrength*lightColor*fColor;

			vec3 norm = normalize(hitpos - fSpherePos);
			vec3 lightDir = normalize(hitpos - lightPos);
			float diff = 0.6*max(dot(-norm, lightDir),0.0);
			vec3 diffuse = diff*lightColor*fColor;

			vec3 viewDir = normalize(camPos - hitpos);
			vec3 reflectDir = reflect(lightDir, norm); 
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec3 specular = 1.0 * spec * lightColor * fColor;  

			outColor = vec4((ambient + diffuse + specular), 1.0); //not a special variable -> needs to be linked with glBindFragDataLocation

			gl_FragDepth = ((dot(hitpos-camPos, -camZ)-gl_DepthRange.near)/gl_DepthRange.diff)/10.0;
		}
	}
}
)";

const char* const sphereClusterFragmentShaderSource2 = R"(
#version 430

//Attributes = Ouputs from vertex shader
smooth in vec3 fColor;
smooth in vec3 fFragPos;
flat in vec3 fSpherePos;
flat in float fSphereRadiusSq;

//Outputs
out vec4 outColor;

//Uniforms
uniform vec3 lightPos;
uniform mat4 viewinv;

void main() 
{
	vec3 camPos = viewinv[3].xyz;
	vec3 camZ = viewinv[2].xyz;
	
	vec3 ray = normalize(fFragPos - camPos);

	vec3 L = camPos-fSpherePos;
	float a = 1;
	float b = 2*dot(ray,L);
	float c = dot(L,L)-fSphereRadiusSq;
	float delta = b*b - 4*a*c;
	if(delta <= 0) discard;
	else
	{
		float t = (-b-sqrt(delta))/2;
		
		vec3 hitpos = ray * t + camPos;

		vec3 lightColor = vec3(1.0,1.0,1.0);
		float ambientStrength = 0.4;
		vec3 ambient = ambientStrength*lightColor;

		vec3 norm = normalize(hitpos - fSpherePos);
		vec3 lightDir = normalize(hitpos - lightPos);
		float diff = 0.6*max(dot(-norm, lightDir),0.0);
		vec3 diffuse = diff*lightColor;

		outColor = vec4((ambient + diffuse), 1.0); //not a special variable -> needs to be linked with glBindFragDataLocation

		gl_FragDepth = ((dot(hitpos-camPos, -camZ)-gl_DepthRange.near)/gl_DepthRange.diff)/10.0;
	}
}
)";