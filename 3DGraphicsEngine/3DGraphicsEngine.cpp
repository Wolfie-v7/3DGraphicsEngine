//
//
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "MatrixLibrary.h"


class my3DEngine : public olc::PixelGameEngine
{
public:

	my3DEngine() 
	{
		sAppName = "Demo";

	}

private:

	ProjectionMatrix pMatrix;

	mesh cube;
	mesh anotherCube;
	mesh model;

	vector3D camera;
	vector3D lookDir_vector;
	vector3D up_vector;
	vector3D right_vector;

	float theta;
	float fYaw;
	float fPitch;
	float fRoll;

	enum drawType
	{
		Fill = 0, 
		Wireframe,
		Both
	} drawingType;

	olc::Pixel GetColor(float lum)
	{
		if (lum <= 0) lum = 0.0f;
		if (lum > 0.9) lum = 0.9f;
		olc::Pixel color = olc::PixelF(0.1f + lum, 0.1f + lum, 0.1f + lum);


		return color;
	}

	

public:

	bool OnUserCreate() override 
	{
		cube.triangles = {
			//Front
			{ 0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f,	1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,	0.0f, 1.0f, 0.0f },

			//Right
			{ 1.0f, 0.0f, 0.0f,    1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

			//Back
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },

			//Left
			{ 0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,	0.0f, 1.0f, 1.0f },

			//Top
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f,	1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,	0.0f, 1.0f, 1.0f },

			//Bottom
			{ 0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,	1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f },
		};

		pMatrix = ProjectionMatrix((float)ScreenHeight() / (float)ScreenWidth());
		model.LoadFromObjFile("tree.obj");
		

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override 
	{
		
		if (GetKey(olc::UP).bHeld) camera.y -= 30.0f * fElapsedTime;
		if (GetKey(olc::DOWN).bHeld) camera.y += 30.0f * fElapsedTime;
		vector3D strafe_velocity_vec = right_vector * (30.0f * fElapsedTime);
		if (GetKey(olc::A).bHeld) camera = camera - strafe_velocity_vec;
		if (GetKey(olc::D).bHeld) camera = camera + strafe_velocity_vec;
		if (GetKey(olc::RIGHT).bHeld) fYaw += 1.1f * fElapsedTime;
		if (GetKey(olc::LEFT).bHeld) fYaw -= 1.1f * fElapsedTime;
		if (GetKey(olc::E).bHeld) fRoll -= 1.1f * fElapsedTime;
		if (GetKey(olc::Q).bHeld) fRoll += 1.1f * fElapsedTime;

		vector3D velocity_vector = lookDir_vector * (30.0f * fElapsedTime);
		if (GetKey(olc::W).bHeld) camera = camera + velocity_vector;
		if (GetKey(olc::S).bHeld) camera = camera - velocity_vector;
		if (GetKey(olc::P).bPressed)
		{
			if (drawingType == 2) drawingType = Fill;
			else drawingType = (drawType)(drawingType + 1);
		}

		Clear(olc::BLACK);

		//theta += 1.0f * fElapsedTime;
		xRotationMatrix x_rMatrix = xRotationMatrix(1.5f * theta);
		zRotationMatrix z_rMatrix = zRotationMatrix(theta);
		TranslationMatrix tMatrix = TranslationMatrix(0.0f, 0.0f, 3.0f); // Change Z value to move object closer or further from camera at time of creation
		ScalingMatrix sMatrix = ScalingMatrix(-1.0f, -1.0f, 1.0f);

		mat4x4 worldMat = IdentityMatrix();
		worldMat = multiplyMatrixByMatrix(z_rMatrix, x_rMatrix);
		worldMat = multiplyMatrixByMatrix(tMatrix, worldMat);
		worldMat = multiplyMatrixByMatrix(sMatrix, worldMat);

		up_vector = { 0.0f, 1.0f, 0.0f };
		vector3D target_vector = { 0.0f, 0.0f, 1.0f };
		mat4x4 cameraRotationMatrix = yRotationMatrix(fYaw);
		mat4x4 zRotMat = zRotationMatrix(fRoll);
		lookDir_vector = multiplyMatrixByVector(target_vector, cameraRotationMatrix);
		right_vector = CrossProduct(lookDir_vector, up_vector);
		right_vector = multiplyMatrixByVector(right_vector, zRotMat);
		up_vector = CrossProduct(right_vector, lookDir_vector);
		
		target_vector = camera + lookDir_vector;

		mat4x4 cameraMatrix = PointAtMat(camera, target_vector, up_vector);
		mat4x4 viewMatrix = invertMatrix(cameraMatrix);

		std::vector<triangle> tris_raster;

		for (const auto& poly : model.triangles)
		{
			triangle triProjected, triTransformed, triViewed;

			triTransformed.p[0] = multiplyMatrixByVector(poly.p[0], worldMat);
			triTransformed.p[1] = multiplyMatrixByVector(poly.p[1], worldMat);
			triTransformed.p[2] = multiplyMatrixByVector(poly.p[2], worldMat);

			// calculate normals
			vector3D vec1, vec2, normal;
			vec1 = triTransformed.p[1] - triTransformed.p[0];
			vec2 = triTransformed.p[2] - triTransformed.p[0];
			normal = NormalizeVector(CrossProduct(vec1, vec2));

			

			if (DotProduct(triTransformed.p[0] - camera, normal) < 0.0f) {

				//mat4x4 scaleMat = scalingMatrix(1.0f, 1.0f, -1.0f);
				//vector3D directional_light = multiplyMatrixByVector(camera, scaleMat);
				vector3D directional_light = { 0.0f, 0.0f, -1.0f };
				//vector3D directional_light2 = { 0.0f, 0.0f, 1.0f };
				

				directional_light = NormalizeVector(directional_light);

				//float lum = std::max(DotProduct(normal, directional_light2), DotProduct(normal, directional_light));
				float lum = DotProduct(normal, lookDir_vector * -1.0f);

				//std::cout << std::to_string(lum) << std::endl;

				olc::Pixel color = GetColor(lum);
				//olc::Pixel color = olc::Pixel(255, 255, 255);
				triTransformed.Color_Info[0] = color.r;
				triTransformed.Color_Info[1] = color.g;
				triTransformed.Color_Info[2] = color.b;
				triTransformed.Color_Info[3] = color.a;

				triViewed.p[0] = multiplyMatrixByVector(triTransformed.p[0], viewMatrix);
				triViewed.p[1] = multiplyMatrixByVector(triTransformed.p[1], viewMatrix);
				triViewed.p[2] = multiplyMatrixByVector(triTransformed.p[2], viewMatrix);

				// Clip triangles
				int nClippledTriangles = 0;
				triangle clipped[2];
				nClippledTriangles = polygons_clipping({ 0.0f,0.0f,0.1f }, { 0.0f,0.0f,1.0f }, triViewed, clipped[0], clipped[1]);
				for (int n = 0; n < nClippledTriangles; n++)
				{
					// Projection
					for (size_t i = 0; i < 3; i++)
					{
						triProjected.p[i] = multiplyMatrixByVector(clipped[n].p[i], pMatrix);

					}

					triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
					triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
					triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

					/*triProjected.p[0].x = triProjected.p[0].x * -1.0f;
					triProjected.p[1].x = triProjected.p[1].x * -1.0f;
					triProjected.p[2].x = triProjected.p[2].x * -1.0f;
					triProjected.p[0].y = triProjected.p[0].y * -1.0f;
					triProjected.p[1].y = triProjected.p[1].y * -1.0f;
					triProjected.p[2].y = triProjected.p[2].y * -1.0f;*/


					// Offset to middle of screen
					vector3D offset = { 1.0f, 1.0f, 0.0f };
					triProjected.p[0] = triProjected.p[0] + offset;
					triProjected.p[1] = triProjected.p[1] + offset;
					triProjected.p[2] = triProjected.p[2] + offset;

					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					triProjected.Color_Info[0] = color.r;
					triProjected.Color_Info[1] = color.g;
					triProjected.Color_Info[2] = color.b;
					triProjected.Color_Info[3] = color.a;
					//t_normal.x *= 0.5f * (float)ScreenWidth();
					//t_normal.y *= 0.5f * (float)ScreenHeight();

					tris_raster.push_back(triProjected);
				}
			}
		}

		std::sort(tris_raster.begin(), tris_raster.end(), [](triangle& poly1, triangle& poly2)
			{
				float z1 = (poly1.p[0].z + poly1.p[1].z + poly1.p[2].z) / 3.0f;
				float z2 = (poly2.p[0].z + poly2.p[1].z + poly2.p[2].z) / 3.0f;
				return z1 < z2;
			});

		for (const auto& triToRaster : tris_raster)
		{
			triangle sclipped[2];
			std::list<triangle> listTriangles;

			listTriangles.push_back(triToRaster);
			int newTriangles = 1;
			for (int p = 0; p < 4; p++)
			{
				int trisToAdd = 0;
				while (newTriangles > 0)
				{
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					newTriangles--;

					switch (p)
					{
					case 0: trisToAdd = polygons_clipping({ 0.0f,0.0f,0.0f }, { 0.0f, 1.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 1: trisToAdd = polygons_clipping({ 0.0f,(float)ScreenHeight() - 1.0f,0.0f }, { 0.0f,-1.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 2: trisToAdd = polygons_clipping({ 0.0f,0.0f,0.0f }, { 1.0f, 0.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 3: trisToAdd = polygons_clipping({ (float)ScreenWidth() - 1,0.0f,0.0f }, { -1.0f, 0.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					}

					for (int w = 0; w < trisToAdd; w++)
					{
						listTriangles.push_back(sclipped[w]);
					}
					
				}
				newTriangles = listTriangles.size();
			}

			for (auto& triTranslated : listTriangles)
			{


				//DrawLine(triTranslated.p[0].x, triTranslated.p[0].y, t_normal.x, t_normal.y, olc::YELLOW);
				switch (drawingType)
				{
				case Fill:
					FillTriangle(triTranslated.p[0].x, triTranslated.p[0].y,
						triTranslated.p[1].x, triTranslated.p[1].y,
						triTranslated.p[2].x, triTranslated.p[2].y,
						olc::Pixel(triTranslated.Color_Info[0],
							triTranslated.Color_Info[1],
							triTranslated.Color_Info[2],
							triTranslated.Color_Info[3]));
					break;
				case Wireframe:
					DrawTriangle(triTranslated.p[0].x, triTranslated.p[0].y,
						triTranslated.p[1].x, triTranslated.p[1].y,
						triTranslated.p[2].x, triTranslated.p[2].y,
						olc::BLUE);
					break;
				case Both:
					FillTriangle(triTranslated.p[0].x, triTranslated.p[0].y,
						triTranslated.p[1].x, triTranslated.p[1].y,
						triTranslated.p[2].x, triTranslated.p[2].y,
						olc::Pixel(triTranslated.Color_Info[0],
							triTranslated.Color_Info[1],
							triTranslated.Color_Info[2],
							triTranslated.Color_Info[3]));
					DrawTriangle(triTranslated.p[0].x, triTranslated.p[0].y,
						triTranslated.p[1].x, triTranslated.p[1].y,
						triTranslated.p[2].x, triTranslated.p[2].y,
						olc::BLUE);
					break;
				default:
					break;
				}
				

				
				
			}
		}
			
		std::string frontVec = "forward x: " + std::to_string(lookDir_vector.x) + "\n"
			+ "forward y: " + std::to_string(lookDir_vector.y) + "\n"
			+ "forward z: " + std::to_string(lookDir_vector.z);
		DrawString(50, 50, frontVec);

		std::string camera_pos = "camera x: " + std::to_string(camera.x) + "\n"
			+ "camera y: " + std::to_string(camera.y) + "\n"
			+ "camera z: " + std::to_string(camera.z);
		DrawString(50, 100, camera_pos);

		std::string nPolygonCount = "Triangles: " + std::to_string(model.triangles.size());
		DrawString(50, 150, nPolygonCount);
		
		return true;
	}

	bool OnUserDestroy() override
	{
		return true;
	}

};


int main() 
{

	my3DEngine app;

	if (app.Construct(750, 750, 1, 1))
		app.Start();

	return 0;
}