#include "graphics_utils.h"

vector3 v3cross(vector3 v1, vector3 v2){
	vector3 result = {(v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z), (v1.x * v2.y) - (v1.y * v2.x)};
	return result;
}

double v3dot(vector3 v1, vector3 v2){
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

vector3 v3diff(vector3 v1, vector3 v2){
	vector3 result = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
	return result;
}

vector3 v3normalize(vector3 v){
	double len = v3length(v);
	vector3 result = {v.x / len, v.y / len, v.z / len};
	return result;
}

double v3length(vector3 v){
	return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

vector3 v3scale(vector3 v, double s){
	vector3 result = {v.x * s, v.y * s, v.z * s};
	return result;
}

vector3 v3sum(vector3 v1, vector3 v2){
	vector3 result = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
	return result;
}
