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

	olc::Pixel GetColor(float lum)
	{
		if (lum < 0) lum = 0.0f;
		olc::Pixel color = olc::PixelF(0.0f + lum, 0.0f + lum, 0.0f + lum);


		return color;
	}

	

public:

	bool OnUserCreate() override 
	{
		anotherCube.polygons = {
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
		model.LoadFromObjFile("Model.obj");
		


		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override 
	{
		
		if (GetKey(olc::UP).bHeld) camera.y -= 10.0f * fElapsedTime;
		if (GetKey(olc::DOWN).bHeld) camera.y += 10.0f * fElapsedTime;
		vector3D strafe_velocity_vec = right_vector * (10.0f * fElapsedTime);
		if (GetKey(olc::A).bHeld) camera = camera - strafe_velocity_vec;
		if (GetKey(olc::D).bHeld) camera = camera + strafe_velocity_vec;
		if (GetKey(olc::RIGHT).bHeld) fYaw += 1.1f * fElapsedTime;
		if (GetKey(olc::LEFT).bHeld) fYaw -= 1.1f * fElapsedTime;
		if (GetKey(olc::E).bHeld) fRoll += 1.1f * fElapsedTime;
		if (GetKey(olc::Q).bHeld) fRoll -= 1.1f * fElapsedTime;

		vector3D velocity_vector = lookDir_vector * (10.0f * fElapsedTime);
		if (GetKey(olc::W).bHeld) camera = camera + velocity_vector;
		if (GetKey(olc::S).bHeld) camera = camera - velocity_vector;

		Clear(olc::BLACK);

		//theta += 1.0f * fElapsedTime;
		xRotationMatrix x_rMatrix = xRotationMatrix(1.5f * theta);
		zRotationMatrix z_rMatrix = zRotationMatrix(theta);
		TranslationMatrix tMatrix = TranslationMatrix(0.0f, 0.0f, 50.0f);
		ScalingMatrix sMatrix = ScalingMatrix(-1.0f, -1.0f, 1.0f);

		mat4x4 worldMat = IdentityMatrix();
		worldMat = multiplyMatrixByMatrix(z_rMatrix, x_rMatrix);
		worldMat = multiplyMatrixByMatrix(sMatrix, worldMat);
		worldMat = multiplyMatrixByMatrix(tMatrix, worldMat);

		up_vector = { 0.0f, 1.0f, 0.0f };
		vector3D target_vector = { 0.0f, 0.0f, 1.0f };
		mat4x4 cameraRotationMatrix = yRotationMatrix(fYaw);
		mat4x4 zRotMat = zRotationMatrix(fRoll);
		lookDir_vector = multiplyMatrixByVector(target_vector, cameraRotationMatrix);
		right_vector = CrossProduct(lookDir_vector, up_vector);
		right_vector = multiplyMatrixByVector(right_vector, zRotMat);
		target_vector = camera + lookDir_vector;

		mat4x4 cameraMatrix = PointAtMat(camera, target_vector, up_vector);
		mat4x4 viewMatrix = invertMatrix_special(cameraMatrix);

		std::vector<polygon> polys_raster;

		for (const auto& poly : model.polygons)
		{
			polygon polyProjected, polyTransformed, polyViewed;

			polyTransformed.p[0] = multiplyMatrixByVector(poly.p[0], worldMat);
			polyTransformed.p[1] = multiplyMatrixByVector(poly.p[1], worldMat);
			polyTransformed.p[2] = multiplyMatrixByVector(poly.p[2], worldMat);

			// calculate normals
			vector3D vec1, vec2, normal;
			vec1 = polyTransformed.p[1] - polyTransformed.p[0];
			vec2 = polyTransformed.p[2] - polyTransformed.p[0];
			normal = NormalizeVector(CrossProduct(vec1, vec2));

			

			if (DotProduct(polyTransformed.p[0] - camera, normal) < 0.0f) {

				//mat4x4 scaleMat = scalingMatrix(1.0f, 1.0f, -1.0f);
				//vector3D directional_light = multiplyMatrixByVector(camera, scaleMat);
				vector3D directional_light = { 0.0f, 0.0f, -1.0f };
				vector3D directional_light2 = { 0.0f, 0.0f, 1.0f };
				

				directional_light = NormalizeVector(directional_light);

				float lum = std::max(DotProduct(normal, directional_light2), DotProduct(normal, directional_light));

				//std::cout << std::to_string(lum) << std::endl;

				olc::Pixel color = GetColor(lum);
				//olc::Pixel color = olc::Pixel(255, 255, 255);
				polyTransformed.Color_Info[0] = color.r;
				polyTransformed.Color_Info[1] = color.g;
				polyTransformed.Color_Info[2] = color.b;
				polyTransformed.Color_Info[3] = color.a;

				polyViewed.p[0] = multiplyMatrixByVector(polyTransformed.p[0], viewMatrix);
				polyViewed.p[1] = multiplyMatrixByVector(polyTransformed.p[1], viewMatrix);
				polyViewed.p[2] = multiplyMatrixByVector(polyTransformed.p[2], viewMatrix);

				// Clip polygons
				int nClippledPolygons = 0;
				polygon clipped[2];
				nClippledPolygons = polygons_clipping({ 0.0f,0.0f,0.1f }, { 0.0f,0.0f,1.0f }, polyViewed, clipped[0], clipped[1]);
				for (int n = 0; n < nClippledPolygons; n++)
				{
					// Projection
					for (size_t i = 0; i < 3; i++)
					{
						polyProjected.p[i] = multiplyMatrixByVector(clipped[n].p[i], pMatrix);

					}

					polyProjected.p[0] = polyProjected.p[0] / polyProjected.p[0].w;
					polyProjected.p[1] = polyProjected.p[1] / polyProjected.p[1].w;
					polyProjected.p[2] = polyProjected.p[2] / polyProjected.p[2].w;

					/*polyProjected.p[0].x = polyProjected.p[0].x * -1.0f;
					polyProjected.p[1].x = polyProjected.p[1].x * -1.0f;
					polyProjected.p[2].x = polyProjected.p[2].x * -1.0f;
					polyProjected.p[0].y = polyProjected.p[0].y * -1.0f;
					polyProjected.p[1].y = polyProjected.p[1].y * -1.0f;
					polyProjected.p[2].y = polyProjected.p[2].y * -1.0f;*/


					// Offset to middle of screen
					vector3D offset = { 1.0f, 1.0f, 0.0f };
					polyProjected.p[0] = polyProjected.p[0] + offset;
					polyProjected.p[1] = polyProjected.p[1] + offset;
					polyProjected.p[2] = polyProjected.p[2] + offset;

					polyProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					polyProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					polyProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					polyProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					polyProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					polyProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					polyProjected.Color_Info[0] = color.r;
					polyProjected.Color_Info[1] = color.g;
					polyProjected.Color_Info[2] = color.b;
					polyProjected.Color_Info[3] = color.a;
					//t_normal.x *= 0.5f * (float)ScreenWidth();
					//t_normal.y *= 0.5f * (float)ScreenHeight();

					polys_raster.push_back(polyProjected);
				}
			}
		}

		std::sort(polys_raster.begin(), polys_raster.end(), [](polygon& poly1, polygon& poly2)
			{
				float z1 = (poly1.p[0].z + poly1.p[1].z + poly1.p[2].z) / 3.0f;
				float z2 = (poly2.p[0].z + poly2.p[1].z + poly2.p[2].z) / 3.0f;
				return z1 < z2;
			});

		for (const auto& polyToRaster : polys_raster)
		{
			polygon sclipped[2];
			std::list<polygon> listPolygons;

			listPolygons.push_back(polyToRaster);
			int newPolygons = 1;
			for (int p = 0; p < 4; p++)
			{
				int polysToAdd = 0;
				while (newPolygons > 0)
				{
					polygon test = listPolygons.front();
					listPolygons.pop_front();
					newPolygons--;

					switch (p)
					{
					case 0: polysToAdd = polygons_clipping({ 0.0f,0.0f,0.0f }, { 0.0f, 1.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 1: polysToAdd = polygons_clipping({ 0.0f,(float)ScreenHeight() - 1.0f,0.0f }, { 0.0f,-1.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 2: polysToAdd = polygons_clipping({ 0.0f,0.0f,0.0f }, { 1.0f, 0.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					case 3: polysToAdd = polygons_clipping({ (float)ScreenWidth() - 1,0.0f,0.0f }, { -1.0f, 0.0f,0.0f }, test, sclipped[0], sclipped[1]); break;
					}

					for (int w = 0; w < polysToAdd; w++)
					{
						listPolygons.push_back(sclipped[w]);
					}
					
				}
				newPolygons = listPolygons.size();
			}

			for (auto& polyTranslated : listPolygons)
			{


				//DrawLine(polyTranslated.p[0].x, polyTranslated.p[0].y, t_normal.x, t_normal.y, olc::YELLOW);
				FillTriangle(polyTranslated.p[0].x, polyTranslated.p[0].y,
					polyTranslated.p[1].x, polyTranslated.p[1].y,
					polyTranslated.p[2].x, polyTranslated.p[2].y,
					olc::Pixel(polyTranslated.Color_Info[0], 
						polyTranslated.Color_Info[1], 
						polyTranslated.Color_Info[2], 
						polyTranslated.Color_Info[3]));

				DrawTriangle(polyTranslated.p[0].x, polyTranslated.p[0].y,
					polyTranslated.p[1].x, polyTranslated.p[1].y,
					polyTranslated.p[2].x, polyTranslated.p[2].y,
					olc::BLUE);

				//FillCircle(target_vector.x, target_vector.y, 100, olc::RED);
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

		std::string nPolygonCount = "Polygons: " + std::to_string(model.polygons.size());
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