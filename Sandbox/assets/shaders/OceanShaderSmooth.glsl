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
uniform int L;

uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

out vec3 GridCoordinates;
out vec4 clipSpaceGrid;
out vec4 clipSpaceReal;
out float RefractiveFactor;

out vec3 lightVec;
out vec3 normalVec;
out vec3 halfwayVec;

void main()
{
	// MESH INDICES
	vec2 X = index;
	vec2 ix0 = X / L0;
	vec2 ix1 = X / L1;
	vec2 ix2 = X / L2;
	// GEOMETRY
	float dx = 1.0 * texture(X0, ix0).r + 1.0 * texture(X1, ix1).r + 1.0 * texture(X2, ix2).r;
	float dy = 1.0 * texture(Y0, ix0).r + 1.0 * texture(Y1, ix1).r + 1.0 * texture(Y2, ix2).r;
	float dz = 1.0 * texture(Z0, ix0).r + 1.0 * texture(Z1, ix1).r + 1.0 * texture(Z2, ix2).r;
	// NORMALS
	float sx0 = 1.0 * texture(X0, ix0).b;// / (1 + texture(N0, ix0).r);
	float sz0 = 1.0 * texture(Z0, ix0).b;// / (1 + texture(N0, ix0).b);
	float sx1 = 1.0 * texture(X1, ix1).b;// / (1 + texture(N1, ix1).r);
	float sz1 = 1.0 * texture(Z1, ix1).b;// / (1 + texture(N1, ix1).b);
	float sx2 = 1.0 * texture(X2, ix2).b;// / (1 + texture(N2, ix2).r);
	float sz2 = 1.0 * texture(Z2, ix2).b;// / (1 + texture(N2, ix2).b);
	//float sx0 =  texture(N0, ix0).r;
	//float sz0 = texture(N0, ix0).b;
	//float sx1 =  texture(N1, ix1).r;
	//float sz1 = texture(N1, ix1).b;
	//float sx2 =  texture(N2, ix2).r;
	//float sz2 = texture(N2, ix2).b;
	
	//vec3 n0 = normalize(vec3(-sx0, 1.0, -sz0));
	//vec3 n1 = normalize(vec3(-sx1, 1.0, -sz1));
	//vec3 n2 = normalize(vec3(-sx2, 1.0, -sz2));
	
	// GL GEOMETRY
	gl_Position = vec4(X.x - dx, dy, X.y - dz, 1.0);

	
	// LIGHTING
	vec4 v = u_ViewMatrix * gl_Position;
	vec3 Normal = normalize(vec3(-sx0-sx1-sx2, 3.0, -sz0-sz1-sz2));
	//lightVec = normalize((u_ViewMatrix * vec4(u_LightPosition, 1.0)).xyz - v.xyz);
	lightVec = normalize(u_LightPosition);
	normalVec = Normal;
	halfwayVec = normalize(lightVec + normalize(u_ViewPosition - gl_Position.xyz));
	//normalVec = (inverse(transpose(u_ViewMatrix)) * vec4(Normal, 0.0)).xyz;
	//halfwayVec = lightVec + normalize(-v.xyz);
	

	// REFLECTION AND REFRACTION MAP COORDINATES
	GridCoordinates = vec3(gl_Position[0], gl_Position[1], gl_Position[2]);
	gl_Position = u_ProjectionMatrix * u_ViewMatrix * gl_Position;
	clipSpaceReal = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates, 1.0);
	clipSpaceGrid = u_ProjectionMatrix * u_ViewMatrix * vec4(GridCoordinates.x, 0.0, GridCoordinates.z,1.0);
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
	float d = dot(normal1, light_vector1);
	bool facing = d > 0.0;
	
	fragColor = C_Ambient +
				C_Diffuse * max(d, 0) +
				(facing ?
					C_Specular * max(pow(dot(normal1, halfway_vector1), 120.0), 0.0) :
					vec4(0.0, 0.0, 0.0, 0.0)) +
				(facing ?
					C_Emissive * max(pow(dot(normal1, light_vector1), 10.0), 0.0) :
					vec4(0.0, 0.0, 0.0, 0.0));

	///////////////////////////////
	// REFLECTION AND REFRACTION //
	///////////////////////////////

	//vec2 ndc = (clipSpaceGrid.xy/clipSpaceGrid.w) / 2.0 + 0.5;
	//ndc = clamp(ndc, 0.001, 0.999);
	//vec2 ndcReal = (clipSpaceReal.xy/clipSpaceReal.w) / 2.0 + 0.5;
	//ndcReal = clamp(ndc, 0.001, 0.999);
	//vec2 refractTexCoords = vec2(ndc.x, ndc.y);
	//vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);

	//float floorDistance = toLinearDepth(texture(DepthTexture, refractTexCoords).r);
	//float waterDistance = toLinearDepth(gl_FragCoord.z);
	//float depth = clamp((floorDistance - waterDistance) / u_Murkiness, 0.0, 1.0);
	//vec3 _refractColor = texture(RefractionTexture, refractTexCoords).rgb;
	//vec3 refractColor = mix(_refractColor, fragColor.rgb, depth);
	

	//vec3 reflectColor = texture(ReflectionTexture, reflectTexCoords).rgb +
	//					((Reflection>0.0) ?
	//						C_Specular.rgb * pow(Reflection,120) : vec3(0.0, 0.0, 0.0));
	//vec3 _finalColor = mix(reflectColor, refractColor,pow(RefractiveFactor,0.5));
	//vec4 finalColor = vec4(_finalColor, 1.0);

	
	///////////////////
	// MIXING IMAGES //
	///////////////////

	//fragColor = mix(fragColor, finalColor, 1.0);
	fragColor.a = 1.0;

}