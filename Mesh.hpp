#pragma once

#include <vector>
#include <iosfwd>
#include <string>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "Ray.hpp"
#include <glm/gtx/io.hpp>

#include <algorithm>
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
	bool contains(size_t o1){
		bool f1 = v1 == o1 || v2 == o1 || v3 == o1;
		return f1 ;
	}
};
//This is the bounding volume hierarchy Code. For some reason it does not work.
struct BoundingBox{
	glm::vec3 min;
	glm::vec3 max;
	BoundingBox(){
		min = glm::vec3(10000000,10000000, 100000000);
		max = glm::vec3 (-1000000, -10000000, -100000000);
	}
	BoundingBox(glm::vec3 min, glm::vec3 max): min(min), max(max){}
	void addTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3){
		min.x = std::min({p1.x, p2.x,p3.x, min.x});
		min.y = std::min({p1.y, p2.y,p3.y, min.y});
		min.z = std::min({p1.z, p2.z,p3.z, min.z});

		max.x = std::max({p1.x, p2.x,p3.x, max.x});
		max.y = std::max({p1.y, p2.y,p3.y, max.y});
		max.z = std::max({p1.z, p2.z,p3.z, max.z});
	}
	void addPoint(glm::vec3 p1){
		min.x = std::min({p1.x, min.x});
		min.y = std::min({p1.y, min.y});
		min.z = std::min({p1.z, min.z});

		max.x = std::max({p1.x, max.x});
		max.y = std::max({p1.y, max.y});
		max.z = std::max({p1.z, max.z});
	}
	bool contains(glm::vec3 p1){
		return p1.x > min.x && p1.x < max.x && p1.y > min.y && p1.y < max.y && p1.z > min.z && p1.z < max.z;
	}

	bool intersect(glm::vec3 o, glm::vec3 d){
		float tmin = (min.x - o.x) / d.x; 
    	float tmax = (max.x - o.x) / d.x; 
	
    	if (tmin > tmax) std::swap(tmin, tmax); 
	
    	float tymin = (min.y - o.y) / d.y; 
    	float tymax = (max.y - o.y) / d.y; 
	
    	if (tymin > tymax) std::swap(tymin, tymax); 
	
    	if ((tmin > tymax) || (tymin > tmax)) 
    	    return false; 
	
    	if (tymin > tmin) 
    	    tmin = tymin; 
	
    	if (tymax < tmax) 
    	    tmax = tymax; 
	
    	float tzmin = (min.z - o.z) / d.z; 
    	float tzmax = (max.z - o.z) / d.z; 
	
    	if (tzmin > tzmax) std::swap(tzmin, tzmax); 
	
    	if ((tmin > tzmax) || (tzmin > tmax))  return false; 
	
    	if (tzmin > tmin) 
    	    tmin = tzmin; 
	
    	if (tzmax < tmax) 
    	    tmax = tzmax; 
	
    	return true; 
	}

	std::vector<glm::vec3> split_box(){
		glm::vec3 d = max -min;
		float greatest = std::max({d.x, d.y,d.z});
		glm::vec3 newMin = min;
		glm::vec3 newMax = max;
		if (greatest == d.x){
			newMax.x = newMin.x + greatest/2;
			newMin.x = newMin.x + greatest/2;
		} else if(greatest == d.y){
			newMax.y = newMin.y + greatest/2;
			newMin.y = newMin.y + greatest/2;

		} else{
			newMax.z = newMin.z + greatest/2;
			newMin.z = newMin.z + greatest/2;
		}

		return std::vector<glm::vec3>({min, newMax, newMin, max});
	}
};

struct BVH{
	BVH *left = nullptr;
	BVH *right = nullptr;
	BoundingBox bbox;
	BVH(){
		
	}
	~BVH(){
		delete left;
		delete right;
	}
	void initialize(std::vector<glm::vec3>& m_vertices, std::vector<Triangle> m_faces, BoundingBox box){
		bbox = box;
		if(m_faces.size() <= 10){
			faces = m_faces;
			return;
		}
		std::vector<glm::vec3> newBox = box.split_box();
		BoundingBox b1 = BoundingBox(newBox[0], newBox[1]);
		BoundingBox b2 = BoundingBox(newBox[2], newBox[3]);

		std::vector<Triangle> b1Faces;
		std::vector<Triangle> b2Faces;

		//If a triangle is in between 2 boundry boxes, just add it to both of them. No harm done =)
		for(int i = 0; i < m_faces.size(); i++){
			if (b1.contains(m_vertices[m_faces[i].v1]) || b1.contains(m_vertices[m_faces[i].v2]) 
					|| b1.contains(m_vertices[m_faces[i].v3]))
				b1Faces.push_back(m_faces[i]);

			if (b2.contains(m_vertices[m_faces[i].v1]) || b2.contains(m_vertices[m_faces[i].v2]) 
					|| b2.contains(m_vertices[m_faces[i].v3]))
				b2Faces.push_back(m_faces[i]);
		}

		// std::cout << m_faces.size() << " " << b1Faces.size() << " " << b2Faces.size() << std::endl;
		left = new BVH();
		left->initialize(m_vertices, b1Faces, b1);
		right = new BVH();
		right->initialize(m_vertices, b2Faces, b2);
	}

	//Return the leaf node that intersects with this ray
	std::vector<Triangle> find(glm::vec3 eye, glm::vec3 direction){
		if (bbox.intersect(eye, direction)){
			if (left == nullptr && right==nullptr) return faces;
			if(left == nullptr) return right->find(eye, direction);
			if(right == nullptr) return left->find(eye, direction);
			else{
				std::vector<Triangle> r = right->find(eye, direction);
				std::vector<Triangle> l = left->find(eye, direction);
				if (r.size() == 0) return l;
				return r;
			}
		} else {
			return std::vector<Triangle> ({});
		}
	}
	std::vector<Triangle> faces;
	
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname, bool isInterpolated );
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray);
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec3> faceNormals;
	bool isInterpolated = true;
	BoundingBox box = BoundingBox();
	BVH bvh = BVH();
    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);

};
