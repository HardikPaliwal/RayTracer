#include <iostream>
#include <fstream>

#include <glm/ext.hpp>
#include <glm/gtx/io.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"


#include "Primitive.hpp"
#include "polyroots.hpp"
#include <glm/glm.hpp>
#include <vector>

using namespace std;
 
std::vector<glm::vec3> Mesh::intersection(glm::vec3 o, glm::vec3 d){
	float closest = 1000000000;
	int faceIndex = 0;
	glm::vec3 weights = glm::vec3(weights);
	vector<Triangle> &faces = m_faces;
	// if (box.intersect(o, d) || m_vertices.size()==4){
		// vector<Triangle> tmp = bvh.find(o, d);
		// if (tmp.size() != 0){
		// 	faces = tmp;
			for (int i = 0; i < faces.size(); i++){
				glm::vec3 a =  m_vertices[faces[i].v1];
				glm::vec3 b = m_vertices[faces[i].v2];
				glm::vec3 c = m_vertices[faces[i].v3];
				glm::mat3 leftHandSide = glm::mat3(a-b, a-c, d);
				glm::vec3 rightHandSide = a - o;
		
				glm::vec3 answer = glm::inverse(leftHandSide) * rightHandSide;
		
				float gamma = 1- (answer.x + answer.y);
				if (answer.z < closest && answer.z > 0.1 && answer.x > 0 && answer.x < 1 && answer.y>0 && answer.y <1 && gamma > 0 && gamma < 1){
					closest = answer.z;
					faceIndex = i;
					weights = glm::vec3(answer.x, answer.y, gamma);
				}
			}
		// }
	// }
	// if (closest == 1000000000) throw "no match";
	

	glm::vec3 intersect = o + d*closest;
	glm::vec3 t1 = m_vertices[faces[faceIndex].v2] - intersect;
	glm::vec3 t2 = m_vertices[faces[faceIndex].v3] - intersect;

	glm::vec3 normal = faceNormals[faceIndex];
	if (isInterpolated){
		normal = weights.x* vertexNormals[faces[faceIndex].v2] +weights.y* vertexNormals[faces[faceIndex].v3] +
				weights.z* vertexNormals[faces[faceIndex].v1];
	} 
	intersect.x = intersect.x+1;
	intersect.y = intersect.z +1;
	intersect.z = 0;

	return std::vector<glm::vec3> {glm::vec3(closest),normal, intersect/2, t1, t2};
}

Mesh::Mesh( const std::string& fname, bool isInterpolated )
	: m_vertices()
	, m_faces(), isInterpolated(isInterpolated)
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


	// Stores Face normals
	for (int i = 0; i < m_faces.size(); i++){
		glm::vec3 u = m_vertices[m_faces[i].v2] - m_vertices[m_faces[i].v1];
		glm::vec3 v = m_vertices[m_faces[i].v3] - m_vertices[m_faces[i].v1];
		faceNormals.push_back(glm::normalize(glm::cross(u,v)));
	}

	// Average out face normals adjacent to vertices to determine vertex normals
	for (int i = 0; i < m_vertices.size(); i++){
		box.addPoint(m_vertices[i]);
		glm::vec3 average = glm::vec3(0);
		for(int k = 0; k < m_faces.size(); k++){
			if (m_faces[k].contains(i)) average += faceNormals[k];
		}
		vertexNormals.push_back(glm::normalize(average));
	}

	//Construct BVM
	// bvh.initialize(m_vertices, m_faces, box);
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
