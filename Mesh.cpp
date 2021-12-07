#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"


#include "Primitive.hpp"
#include "polyroots.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace std;
 
std::vector<glm::vec3> Mesh::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
	float closest = 1000000000;
	int faceIndex = 0;
    for (int i = 0; i < m_faces.size(); i++){
		glm::vec3 a =  m_vertices[m_faces[i].v1];
		glm::vec3 b = m_vertices[m_faces[i].v2];
		glm::vec3 c = m_vertices[m_faces[i].v3];
		glm::mat3 leftHandSide = glm::mat3(a-b, a-c, d);
		glm::vec3 rightHandSide = a - o;

		glm::vec3 answer = glm::inverse(leftHandSide) * rightHandSide;

		float gamma = 1- (answer.x + answer.y);
		if (answer.z < closest && answer.z > 0.1 && answer.x > 0 && answer.x < 1 && answer.y>0 && answer.y <1 && gamma > 0 && gamma < 1){
			closest = answer.z;
			if (storeNormal)  matchedFace = i;  	//Store variables that'll help us calculate the normal later on
			faceIndex = i;
		}
	}

	if (closest == 1000000000) throw "no match";
	

	glm::vec3 intersect = o + d*closest;
	glm::vec3 u = m_vertices[m_faces[faceIndex].v2] - intersect;
	glm::vec3 v = m_vertices[m_faces[faceIndex].v3] - intersect;
	return std::vector<glm::vec3> {glm::vec3(closest),glm::normalize(glm::cross(u,v))};
}
glm::vec3  Mesh::normal(glm::vec3 intersect){
	glm::vec3 u = m_vertices[m_faces[matchedFace].v2] - intersect;
	glm::vec3 v = m_vertices[m_faces[matchedFace].v3] - intersect;
	return glm::normalize(glm::cross(u,v));
}


Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}
