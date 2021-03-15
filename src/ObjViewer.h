#pragma once
#include "AppBase.h"

class ObjViewer : public mvk::AppBase
{
	std::vector<mvk::Vertex> vertices;
	std::vector<uint16_t> indices;

	void loadObjFile();
	
public:
	void setup(mvk::Context context, vk::SurfaceKHR surface)
	override;
};
