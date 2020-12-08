#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
struct vector3D
{
	float x = 0.0f; 
	float y = 0.0f; 
	float z = 0.0f; 
	float w = 1.0f;

	vector3D operator- (const vector3D& vector)
	{
		vector3D _result = { 0.0f, 0.0f, 0.0f , 1.0f};
		_result.x = this->x - vector.x;
		_result.y = this->y - vector.y;
		_result.z = this->z - vector.z;

		return _result;
	}

	vector3D operator+ (const vector3D& vector)
	{
		vector3D _result = { 0.0f, 0.0f, 0.0f , 1.0f };
		_result.x = this->x + vector.x;
		_result.y = this->y + vector.y;
		_result.z = this->z + vector.z;

		return _result;
	}

	vector3D operator/ (float divisor)
	{
		vector3D _result = { 0.0f, 0.0f, 0.0f , 1.0f };
		_result.x = this->x / divisor;
		_result.y = this->y / divisor;
		_result.z = this->z / divisor;

		return _result;
	}

	vector3D operator* (float factor)
	{
		vector3D _result = { 0.0f, 0.0f, 0.0f , 1.0f };
		_result.x = this->x * factor;
		_result.y = this->y * factor;
		_result.z = this->z * factor;

		return _result;
	}

};

struct triangle
{
	vector3D p[3];
	int32_t Color_Info[4] = { 255 };
};

struct mesh
{
	std::vector<triangle> triangles;

	bool LoadFromObjFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		std::vector<vector3D> verts_cache;
		std::string line;
		

		while (std::getline(f, line))
		{
			char junk;
			std::stringstream sstream;
			sstream << line;

			if (line[0] == 'v' && line[1] == ' ')
			{
				
				
				vector3D vector;

				sstream >> junk >> vector.x >> vector.y >> vector.z;
				verts_cache.push_back(vector);
			}

			if (line[0] == 'f')
			{
				size_t vert1, vert2, vert3;

				sstream >> junk >> vert1;
				sstream.ignore(256, ' ');
				sstream >> vert2;
				sstream.ignore(256, ' ');
				sstream >> vert3;

				//std::cout << vert1 << vert2 << vert3;

				triangles.push_back({ verts_cache[vert1 - 1], verts_cache[vert2 - 1], verts_cache[vert3 - 1] });
				
			}
		}
		return true;
	}
};

static float DotProduct(const vector3D& vec1, const vector3D& vec2)
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

static vector3D NormalizeVector(const vector3D& vector)
{
	float length = sqrtf(DotProduct(vector, vector));
	vector3D result = { vector.x / length, vector.y / length, vector.z / length };
	return result;
}

static vector3D CrossProduct(const vector3D& vec1, const vector3D& vec2)
{
	vector3D normal = { 0.0f, 0.0f, 0.0f };
	normal.x = vec1.y * vec2.z - vec1.z * vec2.y;
	normal.y = vec1.z * vec2.x - vec1.x * vec2.z;
	normal.z = vec1.x * vec2.y - vec1.y * vec2.x;

	/*float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

	normal.x /= length; normal.y /= length; normal.z /= length;*/

	return normal;
}

static vector3D vec_intersect_plane(vector3D& plane_v, vector3D& plane_n, vector3D& line_s, vector3D& line_e)
{
	plane_n = NormalizeVector(plane_n);
	float plane_d = -DotProduct(plane_n, plane_v);
	float ad = DotProduct(line_s, plane_n);
	float bd = DotProduct(line_e, plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	vector3D line = line_e - line_s;
	vector3D line_to_intersect = line * t;
	return line_s + line_to_intersect;
}

static int polygons_clipping(vector3D plane_p, vector3D plane_n, triangle& in_poly, triangle& out_poly_1, triangle& out_poly_2)
{
	plane_n = NormalizeVector(plane_n);

	// Return shortest distance from point to plane, plane normal must be normalized
	auto dist = [&](vector3D& p)
	{
		vector3D n = NormalizeVector(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - DotProduct(plane_n, plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vector3D* inside_points[3]; int nInsidePointCount = 0;
	vector3D* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(in_poly.p[0]);
	float d1 = dist(in_poly.p[1]);
	float d2 = dist(in_poly.p[2]);

	if (d0 >= 0) inside_points[nInsidePointCount++] = &in_poly.p[0];
	else outside_points[nOutsidePointCount++] = &in_poly.p[0];
	if (d1 >= 0) inside_points[nInsidePointCount++] = &in_poly.p[1];
	else outside_points[nOutsidePointCount++] = &in_poly.p[1];
	if (d2 >= 0) inside_points[nInsidePointCount++] = &in_poly.p[2];
	else outside_points[nOutsidePointCount++] = &in_poly.p[2];

	if (nInsidePointCount == 0) 
	{
		return 0;
	}

	if (nInsidePointCount == 3)
	{
		out_poly_1 = in_poly;
		return 1;
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		out_poly_1.Color_Info[0] = in_poly.Color_Info[0];
		out_poly_1.Color_Info[1] = in_poly.Color_Info[1];
		out_poly_1.Color_Info[2] = in_poly.Color_Info[2];
		out_poly_1.Color_Info[3] = in_poly.Color_Info[3];

		out_poly_1.p[0] = *inside_points[0];

		out_poly_1.p[1] = vec_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		out_poly_1.p[2] = vec_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

		return 1;
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		out_poly_1.Color_Info[0] = in_poly.Color_Info[0];
		out_poly_1.Color_Info[1] = in_poly.Color_Info[1];
		out_poly_1.Color_Info[2] = in_poly.Color_Info[2];
		out_poly_1.Color_Info[3] = in_poly.Color_Info[3];

		out_poly_2.Color_Info[0] = in_poly.Color_Info[0];
		out_poly_2.Color_Info[1] = in_poly.Color_Info[1];
		out_poly_2.Color_Info[2] = in_poly.Color_Info[2];
		out_poly_2.Color_Info[3] = in_poly.Color_Info[3];

		out_poly_1.p[0] = *inside_points[0];
		out_poly_1.p[1] = *inside_points[1];
		out_poly_1.p[2] = vec_intersect_plane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

		out_poly_2.p[1] = *inside_points[1];
		out_poly_2.p[0] = out_poly_1.p[2];
		out_poly_2.p[2] = vec_intersect_plane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

		return 2;
	}
	return 0;
}
