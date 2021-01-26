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
uniform mat4 u_ViewProjectionMatrix;

out vec3 GridCoordinates;
out vec4 clipSpaceGrid;
out vec4 clipSpaceReal;

out vec2 ix0;
out vec2 ix1;
out vec3 viewVec;

void main()
{
	// MESH INDICES
	vec2 X = index * Ls;
	ix0 = X / L0;
	ix1 = X / L1;

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
	dx/=2;
	dz/=2;

	// GL GEOMETRY
	gl_Position = vec4(X.x - dx, dy * 0.2, X.y - dz, 1.0) * Lr / Ls;
	gl_Position.a = 1.0;

	
	// LIGHTING
	viewVec = gl_Position.xyz - u_ViewPosition.xyz;

	// REFLECTION AND REFRACTION MAP COORDINATES
	GridCoordinates = gl_Position.xyz;//;vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
	clipSpaceReal = u_ViewProjectionMatrix * vec4(gl_Position.xyz, 1.0);
	clipSpaceGrid = u_ViewProjectionMatrix * vec4(gl_Position.x, 0.0, gl_Position.z,1.0);
	gl_Position = u_ViewProjectionMatrix * gl_Position;
}

#type fragment
#version 430

layout(location = 0) out vec4 fragColor;

layout(binding = 12) uniform sampler2D ReflectionTexture;
layout(binding = 13) uniform sampler2D RefractionTexture;
layout(binding = 14) uniform sampler2D DepthTextureRefraction;

layout(binding = 0) uniform sampler2D X0;
layout(binding = 2) uniform sampler2D Z0;
layout(binding = 3) uniform sampler2D X1;
layout(binding = 5) uniform sampler2D Z1;

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

in vec2 ix0;
in vec2 ix1;
in vec3 viewVec;

float FRESNEL_REFLECTANCE_BASE = 0.2;

float toLinearDepth(float zDepth){
	return 2.0 * z_Near * z_Far / (z_Far + z_Near - (2.0 * zDepth - 1.0) * (z_Far - z_Near));
}

void main (void) 
{	
	// NORMALS
	float sx0 = texture(X0, ix0).b;
	float sz0 = texture(Z0, ix0).b;
	float sx1 = texture(X1, ix1).b;
	float sz1 = texture(Z1, ix1).b;
	vec3 Normal = normalize(vec3(-sx0-sx1, 3.0, -sz0-sz1));
	vec3 View = normalize(viewVec);
	//////////////////////////
	// BASE WATER COLOURING //
	//////////////////////////
	
	float d = -dot(Normal, View);
	fragColor = C_Ambient + C_Diffuse * max(d, 0.0);

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
	float normalDotEye = abs(d);
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