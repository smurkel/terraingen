#type vertex
#version 430

layout(location = 0) in vec2 index;
layout(binding = 0) uniform sampler2D X0;
layout(binding = 1) uniform sampler2D Y0;
layout(binding = 2) uniform sampler2D Z0;
layout(binding = 3) uniform sampler2D X1;
layout(binding = 4) uniform sampler2D Y1;
layout(binding = 5) uniform sampler2D Z1;

uniform int L0;
uniform int L1;
uniform int Ls;
uniform float Lr;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out vec3 GridCoordinates;
out vec4 clipSpaceGrid;
out vec4 clipSpaceReal;

out vec3 viewVec;
out vec3 lightVec;
out vec3 normalVec;
out vec3 halfwayVec;
out vec3 normalAlt;

void main()
{
	// MESH INDICES
	vec2 X = index * Ls;
	vec2 ix0 = X / L0;
	vec2 ix1 = X / L1;

	vec2 XNx0 = texture(X0, ix0).rb;
	vec2 XNx1 = texture(X1, ix1).rb;
	vec2 ZNz0 = texture(Z0, ix0).rb;
	vec2 ZNz1 = texture(Z1, ix1).rb;
	float Y0 = texture(Y0, ix0).r;
	float Y1 = texture(Y1, ix1).r;
	// GEOMETRY
	float dx = XNx0.r + XNx1.r;

	float dy = Y0 + Y1;
	float dz = ZNz0.r + ZNz1.r;
	// NORMALS
	float sx0 = XNx0.g;
	float sz0 = ZNz0.g;
	float sx1 = XNx1.g;
	float sz1 = ZNz1.g;
	
	//if (NORMAL == 1)
	//{
	//	sx0 = texture(X0, ix0).b / (1 + (texture(N0, ix0).r));
	//	sx1 = texture(X1, ix1).b / (1 + (texture(N1, ix1).r));
	//	sz0 = texture(Z0, ix0).b / (1 + (texture(N0, ix0).b));
	//	sz1 = texture(Z1, ix1).b / (1 + (texture(N1, ix1).b));
	//}
	
	// GL GEOMETRY
	gl_Position = vec4(X.x - dx, dy * 0.2, X.y - dz, 1.0) * Lr / Ls;
	gl_Position.a = 1.0;

	
	// LIGHTING
	vec3 Normal = normalize(vec3(-sx0-sx1, 2.0, -sz0-sz1));
	vec4 v = u_ViewMatrix * gl_Position;
	viewVec = v.xyz;
	lightVec = normalize((u_ViewMatrix * vec4(u_LightPosition, 1.0)).xyz - viewVec);
	normalVec = (inverse(transpose(u_ViewMatrix)) * vec4(Normal, 0.0)).xyz;
	halfwayVec = lightVec + normalize(-viewVec);
	normalAlt = Normal;

	// REFLECTION AND REFRACTION MAP COORDINATES
	GridCoordinates = vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
	gl_Position = u_ProjectionMatrix * u_ViewMatrix * gl_Position;
	clipSpaceReal = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates, 1.0);
	clipSpaceGrid = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates.x, 0.0, GridCoordinates.z,1.0);
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

layout(binding = 12) uniform sampler2D ReflectionTexture;
layout(binding = 13) uniform sampler2D RefractionTexture;
layout(binding = 14) uniform sampler2D DepthTextureRefraction;

in vec4 clipSpaceGrid;
in vec4 clipSpaceReal;

uniform vec3 u_ViewPosition;

uniform vec4 C_Emissive;
uniform vec4 C_Ambient;
uniform vec4 C_Diffuse;
uniform vec4 C_Specular;

uniform float z_Near;
uniform float z_Far;
uniform float u_Murkiness;

in vec3 lightVec;
in vec3 normalVec;
in vec3 halfwayVec;
in vec3 viewVec;
in vec3 normalAlt;

float FRESNEL_REFLECTANCE_BASE = 0.05;

float toLinearDepth(float zDepth){
	return 2.0 * z_Near * z_Far / (z_Far + z_Near - (2.0 * zDepth - 1.0) * (z_Far - z_Near));
}

void main (void) 
{	

	//////////////////////////
	// BASE WATER COLOURING //
	//////////////////////////
	vec3 normal1 = normalize(normalVec);
	vec3 light_vector1 = normalize(lightVec);
	vec3 halfway_vector1 = normalize(halfwayVec);
	vec3 view_vector1 = normalize(viewVec);

	float d = dot(normal1, light_vector1);
	bool facing = d > 0.0;
	
	fragColor = C_Ambient +
				C_Diffuse * max(d, 0) +
				(facing ?
					C_Specular * max(pow(dot(normal1, halfway_vector1), 200.0), 0.0) :
					vec4(0.0, 0.0, 0.0, 0.0)) +
				(facing ?
					C_Emissive * max(pow(dot(normal1, light_vector1), 10.0), 0.0) :
					vec4(0.0, 0.0, 0.0, 0.0));

	///////////////////////////////
	// REFLECTION AND REFRACTION //
	///////////////////////////////

	// REFLECT/FRACT TEXTURES AND COORDINATES
	vec2 ndc = (clipSpaceGrid.xy/clipSpaceGrid.w) / 2.0 + 0.5;
	ndc = clamp(ndc, 0.001, 0.999);
	vec2 ndcReal = (clipSpaceReal.xy/clipSpaceReal.w) / 2.0 + 0.5;
	ndcReal = clamp(ndc, 0.001, 0.999);
	vec2 refractTexCoords = vec2(ndc.x, ndc.y);
	vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);

	// FRESNEL (Schlicks approximation)
	float normalDotEye = abs(dot(normalAlt, view_vector1));
	float fReflection = FRESNEL_REFLECTANCE_BASE + (1.0 - FRESNEL_REFLECTANCE_BASE) * pow((1.0 - normalDotEye), 5.0);
	float fRefraction = 1.0 - fReflection;
	
	// SET UP REFRACTION
	float floorDistance = toLinearDepth(texture(DepthTextureRefraction, refractTexCoords).r);
	float waterDistance = toLinearDepth(gl_FragCoord.z);
	float depth = clamp((floorDistance - waterDistance) / u_Murkiness, 0.0, 1.0);
	
	vec3 _refractColor = texture(RefractionTexture, refractTexCoords).rgb;
	vec3 refractColor = mix(_refractColor, fragColor.rgb, depth);
	
	// SET UP REFLECTION
	vec3 reflectColor = texture(ReflectionTexture, reflectTexCoords).rgb;
	
	vec3 _finalColor = mix(reflectColor, refractColor, fRefraction);
	vec4 finalColor = vec4(_finalColor, 1.0);

	
	///////////////////
	// MIXING IMAGES //
	///////////////////
	fragColor = mix(fragColor, finalColor, 1.0);
	fragColor.a = 1.0;

}