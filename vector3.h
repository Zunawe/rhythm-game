typedef struct{
	double x;
	double y;
	double z;
} vector3;

vector3 v3cross(vector3 v1, vector3 v2);
vector3 v3diff(vector3 v1, vector3 v2);
vector3 v3normalize(vector3 v);
double v3length(vector3 v);
