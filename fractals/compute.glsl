#version 430 core
precision highp float;

layout(local_size_x = 20, local_size_y = 20) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

layout(location = 2) uniform float max_p = 1;
layout(location = 3) uniform int min_p = 10;

//define x^3 - 1
//define x = vec2
int n = 8;
layout(std430, binding = 1) buffer rx
{
	float datax[];
};
layout(std430, binding = 2) buffer ry
{
	float datay[];
};

vec2 multiply(vec2 c1, vec2 c2) {
	return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c2.x * c1.y);
}

vec2 NewtonRhapson(vec2 pt) {
	for (int i = 0; i < min_p; i++) {
		vec2 x = pt;
		vec2 xdot = vec2(0, 0);
		for (int j = 0; j < n; j++) {
			x = multiply(x, vec2(datax[j] - pt.x, datay[j] - pt.y));
		}
		for (int j = 0; j < n; j++) {
			vec2 y = pt;
			for (int k = 0; k < n; k++) {
				if (k != j) y = multiply(y, vec2(datax[k] - pt.x, datay[k] - pt.y));
			}
			xdot = xdot + y;
		}
		xdot.y *= -1;
		pt = pt + multiply(x, xdot) / (xdot.x * xdot.x + xdot.y * xdot.y);
	}
	return pt;
}

float distance(vec2 lhs, vec2 rhs) {
	return (lhs - rhs).x * (lhs - rhs).x + (lhs - rhs).y * (lhs - rhs).y;
}

float which_root(vec2 pt) {
	float min = 0xFFFFFF;
	int index = -1;
	for (int i = 0; i < n; i++) {
		float dst = distance(vec2(datax[i], datay[i]), pt);
		
		if (dst < min) {
			min = dst;
			index = i;
		}
	}
	
	return index * 0.1f;
}

int color_roots(vec2 start) {
	float min = 0xFFFFFF;
	int index = -1;
	for (int i = 0; i < n; i++) {
		float dst = distance(vec2(datax[i], datay[i]), start);
		if (dst < 0.001f) {
			return -1;
		}
	}
	return 0;
}
vec4 color_map(float data) {
	data = abs(data);
	if (data < 0.166f) {
		return vec4(0.0, data * 6, 0.0, 1.0f);
	}
	if (data < 0.333f) {
		return vec4(0.0, 1.0f, (data - 0.1666f) * 6, 1.0);
	}
	if (data < 0.5f) {
		return vec4(0.0, 1.0 - (data - 0.33f) * 6, 1.0, 1.0);
	}
	if (data < 0.666f) {
		return vec4((data - 0.5f) * 6, 0.0, 1.0f, 1.0);
	}
	if (data < 0.8333f) {
		return vec4(1.0, 0.0f, 1.0 - (data - 0.66f) * 6, 1.0);
	}
	if (data < 1.0f) {
		return vec4(1.0 - (data - 0.833f) * 6, 0.0f, 0.0, 1.0);
	}
	return vec4(0, 0, 1, 1);
}
void main() {
	vec2 pt =  max_p * vec2((gl_GlobalInvocationID.x * 1.0f - 600) * 0.001666f , (gl_GlobalInvocationID.y * 1.0f - 600) * 0.001666f);
	vec4 col;
	if (color_roots(pt) == -1) {
		col = vec4(1, 1, 1, 1);
	}
	else {
		pt = NewtonRhapson(pt);
		float id = which_root(pt);
		col = color_map(id * 1.1f);
	}
	
	imageStore(img_output, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), col);
}