#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "Ray.hpp"
// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

struct Triangle
{
	size_t v1;
	size_t v2;
	size_t v3;

	Triangle( size_t pv1, size_t pv2, size_t pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
  virtual glm::vec3 normal(glm::vec3 intersect);
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	int matchedFace;
    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
