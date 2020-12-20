#type vertex
#version 430

layout(location = 0) in vec2 index;
layout(binding = 0) uniform sampler2D X0;
layout(binding = 1) uniform sampler2D Y0;
layout(binding = 2) uniform sampler2D Z0;
layout(binding = 3) uniform sampler2D X1;
layout(binding = 4) uniform sampler2D Y1;
layout(binding = 5) uniform sampler2D Z1;
layout(binding = 6) uniform sampler2D X2;
layout(binding = 7) uniform sampler2D Y2;
layout(binding = 8) uniform sampler2D Z2;
layout(binding = 9) uniform sampler2D N0;
layout(binding = 10) uniform sampler2D N1;
layout(binding = 11) uniform sampler2D N2;

uniform int L0;
uniform int L1;
uniform int L2;

out vec3 GridCoordinates;
uniform int L;
uniform float u_Waterlevel;
void main()
{
	// MESH INDICES
	vec2 X = index;
	vec2 ix0 = X / L0;
	vec2 ix1 = X / L1;
	vec2 ix2 = X / L2;
	// GEOMETRY
	float dx = texture(X0, ix0).r + texture(X1, ix1).r + texture(X2, ix2).r;
	float dy = 1.0 * texture(Y0, ix0).r + 1.0 * texture(Y1, ix1).r + 1.0 * texture(Y2, ix2).r;
	float dz = texture(Z0, ix0).r + texture(Z1, ix1).r + texture(Z2, ix2).r;
	gl_Position = vec4(X.x - dx, dy, X.y - dz, 1.0);
	GridCoordinates = vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
}

#type geometry
#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;
uniform int L;

out float Reflection;
out float RefractiveFactor;
out float Fog;

in vec3 GridCoordinates[];
out vec4 clipSpaceGrid;
out vec4 clipSpaceReal;

out vec3 f_Normal;
out vec3 f_Position;


void main()
{

	// calculate normal vector
	vec3 V1 = gl_in[0].gl_Position.xyz;
	vec3 V2 = gl_in[1].gl_Position.xyz;
	vec3 V3 = gl_in[2].gl_Position.xyz;
	vec3 Normal = normalize(cross(V2-V1, V3-V1));

	vec3 P = ((V1 + V2 + V3) / 3).xyz;
	Reflection = dot(normalize(reflect(P - u_LightPosition, Normal)),normalize(u_ViewPosition - P));
	RefractiveFactor = dot(-Normal, normalize(P - u_ViewPosition));
	
	
	gl_Position = u_ViewMatrix * gl_in[0].gl_Position;	
	Fog = pow(length(V1)/50.0, 2);
	gl_Position = u_ProjectionMatrix * gl_Position;
	clipSpaceReal = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[0].xyz,1.0);
	clipSpaceGrid = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[0].x, 0.0, GridCoordinates[0].z,1.0);
	EmitVertex();

	gl_Position = u_ViewMatrix * gl_in[1].gl_Position;	
	Fog = pow(length(V2)/50.0, 2);
	gl_Position = u_ProjectionMatrix * gl_Position;
	clipSpaceReal = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[1].xyz,1.0);
	clipSpaceGrid = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[1].x, 0.0, GridCoordinates[1].z,1.0);
	EmitVertex();

	gl_Position = u_ViewMatrix * gl_in[2].gl_Position;	
	Fog = pow(length(V3)/50.0, 2);
	gl_Position = u_ProjectionMatrix * gl_Position;
	clipSpaceReal = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[2].xyz,1.0);
	clipSpaceGrid = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates[2].x, 0.0, GridCoordinates[2].z,1.0);
	EmitVertex();

}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

layout(binding = 3) uniform sampler2D ReflectionTexture;
layout(binding = 4) uniform sampler2D RefractionTexture;
layout(binding = 5) uniform sampler2D DepthTexture;

in vec4 clipSpaceGrid;
in vec4 clipSpaceReal;

in float Reflection;
in float RefractiveFactor;
in float Fog;

in vec3 f_Normal;
in vec3 f_Position;
uniform vec3 u_ViewPosition;

uniform vec4 C_Emissive;
uniform vec4 C_Ambient;
uniform vec4 C_Diffuse;
uniform vec4 C_Specular;

uniform float z_Near;
uniform float z_Far;
uniform float u_Murkiness;


float toLinearDepth(float zDepth){
	return 2.0 * z_Near * z_Far / (z_Far + z_Near - (2.0 * zDepth - 1.0) * (z_Far - z_Near));
}

void main (void) 
{	

	//////////////////////////
	// BASE WATER COLOURING //
	//////////////////////////

	fragColor = C_Ambient +
				C_Diffuse * max(Reflection, 0) +
				((Reflection>0.0) ?
					C_Specular * pow(Reflection,120) :
					vec4(0.0, 0.0, 0.0, 0.0)) +
				((RefractiveFactor>0.0) ?
					C_Emissive * pow(RefractiveFactor, 2) :
					vec4(0.0, 0.0, 0.0, 0.0));

	///////////////////////////////
	// REFLECTION AND REFRACTION //
	///////////////////////////////

	if (1 == 0)
	{
		vec2 ndc = (clipSpaceGrid.xy/clipSpaceGrid.w) / 2.0 + 0.5;
		ndc = clamp(ndc, 0.001, 0.999);
		vec2 ndcReal = (clipSpaceReal.xy/clipSpaceReal.w) / 2.0 + 0.5;
		ndcReal = clamp(ndc, 0.001, 0.999);
		vec2 refractTexCoords = vec2(ndc.x, ndc.y);
		vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);

		float floorDistance = toLinearDepth(texture(DepthTexture, refractTexCoords).r);
		float waterDistance = toLinearDepth(gl_FragCoord.z);
		float depth = clamp((floorDistance - waterDistance) / u_Murkiness, 0.0, 1.0);
		vec3 _refractColor = texture(RefractionTexture, refractTexCoords).rgb;
		vec3 refractColor = mix(_refractColor, fragColor.rgb, depth);
	

		vec3 reflectColor = texture(ReflectionTexture, reflectTexCoords).rgb +
							((Reflection>0.0) ?
								C_Specular.rgb * pow(Reflection,120) : vec3(0.0, 0.0, 0.0));
		vec3 _finalColor = mix(reflectColor, refractColor,pow(RefractiveFactor,0.5));
		vec4 finalColor = vec4(_finalColor, 1.0);
	}

	
	///////////////////
	// MIXING IMAGES //
	///////////////////

	//fragColor = mix(fragColor, finalColor, 1.0);
	if (Fog > 0.6)
	{
		//float fog = (Fog-0.6)/(1-0.6);
		//fragColor = (1 - fog) * fragColor + fog * vec4(1.0, 1.0, 1.0, 0.0);
		//fragColor = vec4(1.0,1.0,1.0,0.0);
	}
	fragColor.a = 1.0;

}