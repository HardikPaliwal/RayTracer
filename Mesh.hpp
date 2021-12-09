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
	bool contains(size_t o1, size_t o2, size_t o3){
		bool f1 = v1 == o1 || v1 == o2 || v1 == o3;
		bool f2 = v2 == o1 || v2 == o2 || v2 == o3;
		bool f3 = v3 == o1 || v3 == o2 || v3 == o3;
		// return ((f1 && f2 )||(f1&&f3)|| (f3&&f2) )
		return f1 || f2 || f3;
	}
	bool contains(size_t o1){
		bool f1 = v1 == o1 || v2 == o1 || v3 == o1;
		return f1 ;
	}
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray);
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec3> faceNormals;
	bool isInterpolated = true;
    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);

};
