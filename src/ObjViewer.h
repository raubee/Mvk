#pragma once
#include "AppBase.h"

class ObjViewer : public mvk::AppBase
{
	void loadObjFile(std::vector<mvk::Vertex>& vertices,
		std::vector<uint16_t>& indices) const;
	
public:
	ObjViewer(mvk::Context context, vk::SurfaceKHR surface);
};
