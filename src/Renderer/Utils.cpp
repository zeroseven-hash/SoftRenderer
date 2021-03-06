#include"Utils.h"


#include<vector>
namespace Utils
{
	//static object
	static std::vector<CubeVertex> s_cube_vs =
	{
		{{ -1.0f, -1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},
		{{  1.0f,  1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},
		{{  1.0f, -1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},
		{{  1.0f,  1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},
		{{ -1.0f, -1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},
		{{ -1.0f,  1.0f, -1.0f,1.0f},{0.0f,0.0f,-1.0f}},

		{{ -1.0f, -1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{  1.0f, -1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{  1.0f,  1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{  1.0f,  1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{ -1.0f,  1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},
		{{ -1.0f, -1.0f,  1.0f,1.0f},{0.0f,0.0f,1.0f}},

		{{ -1.0f,  1.0f,  1.0f,1.0f},{-1.0f,0.0f,0.0f}},
		{{ -1.0f,  1.0f, -1.0f,1.0f},{-1.0f,0.0f,0.0f}},
		{{ -1.0f, -1.0f, -1.0f,1.0f},{-1.0f,0.0f,0.0f}},
		{{ -1.0f, -1.0f, -1.0f,1.0f},{-1.0f,0.0f,0.0f}},
		{{ -1.0f, -1.0f,  1.0f,1.0f},{-1.0f,0.0f,0.0f}},
		{{ -1.0f,  1.0f,  1.0f,1.0f},{-1.0f,0.0f,0.0f}},

		{{ 1.0f,  1.0f,  1.0f,1.0f},{1.0f,0.0f,0.0f}},
		{{ 1.0f, -1.0f, -1.0f,1.0f},{1.0f,0.0f,0.0f}},
		{{ 1.0f,  1.0f, -1.0f,1.0f},{1.0f,0.0f,0.0f}},
		{{ 1.0f, -1.0f, -1.0f,1.0f},{1.0f,0.0f,0.0f}},
		{{ 1.0f,  1.0f,  1.0f,1.0f},{1.0f,0.0f,0.0f}},
		{{ 1.0f, -1.0f,  1.0f,1.0f},{1.0f,0.0f,0.0f}},

		{{-1.0f, -1.0f, -1.0f,1.0f},{0.0f,-1.0f,0.0f}},
		{{ 1.0f, -1.0f, -1.0f,1.0f},{0.0f,-1.0f,0.0f}},
		{{ 1.0f, -1.0f,  1.0f,1.0f},{0.0f,-1.0f,0.0f}},
		{{ 1.0f, -1.0f,  1.0f,1.0f},{0.0f,-1.0f,0.0f}},
		{{-1.0f, -1.0f,  1.0f,1.0f},{0.0f,-1.0f,0.0f}},
		{{-1.0f, -1.0f, -1.0f,1.0f},{0.0f,-1.0f,0.0f}},

		{{-1.0f,  1.0f, -1.0f,1.0f},{0.0f,1.0f,0.0f}},
		{{ 1.0f,  1.0f,  1.0f,1.0f},{0.0f,1.0f,0.0f}},
		{{ 1.0f,  1.0f, -1.0f,1.0f},{0.0f,1.0f,0.0f}},
		{{ 1.0f,  1.0f,  1.0f,1.0f},{0.0f,1.0f,0.0f}},
		{{-1.0f,  1.0f, -1.0f,1.0f},{0.0f,1.0f,0.0f}},
		{{-1.0f,  1.0f,  1.0f,1.0f},{0.0f,1.0f,0.0f}},
	};
	static std::vector<uint32_t> s_cube_indices =
	{
		0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35
	};


	//const VertexArrayBuffer<SphereVertex>& GetSphereBuffer()
	//{
	//	static Sphere sphere;
	//	return sphere.vao_;
	//}

	
	const VertexArrayBuffer<CubeVertex>& GetCubeBuffer()
	{
		static VertexArrayBuffer<CubeVertex> cubebuffer(s_cube_vs, s_cube_indices);
		return cubebuffer;
	}

	/*Sphere::Sphere()
	{
		
	}*/

}