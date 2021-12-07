#include "Primitive.hpp"
#include "polyroots.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <glm/ext.hpp>

using namespace std;


std::vector<glm::vec3> sphereIntersection(glm::vec3 o, glm::vec3 d, glm::vec3 sphereOrigin, double sphereRadius){
  double squared = glm::dot(d, d);
  double single = 2*glm::dot(d,o - sphereOrigin );
  double constant = glm::dot(o - sphereOrigin, o-sphereOrigin) - sphereRadius*sphereRadius;
  double roots[2];
  int numRoots = quadraticRoots(squared, single, constant, roots);

  if (
    (numRoots == 0) ||
    (numRoots == 1 && roots[0] < 0.1) ||
    (numRoots == 2 && roots[0] < 0.1 && roots[1] < 0.1)
  ) {
    throw "no match";
  }
  double t = 0;
  if (numRoots == 1) {
    t = roots[0];
  }
  else if (roots[0] < 0) {
    t= roots[1];
  }

  else if (roots[1] < 0) {
    t= roots[0];
  } else {
    t = std::min(roots[0], roots[1]);
  }
  glm::vec3 intersect = t*d + o;
  return std::vector<glm::vec3> {glm::vec3(t), intersect - sphereOrigin};
}


std::vector<glm::vec3> squareIntersection(glm::vec3 o, glm::vec3 d, glm::vec3 so, double sl, glm::vec3 *intersectionNormal){
  //6 Faces of a plane
  //Point on each face and correspounding normal:
  float halfSl = (float) sl;
  float xmin = so.x;
  float xmax = so.x + halfSl;
  float ymin = so.y ;
  float ymax = so.y + halfSl;
  float zmin = so.z ;
  float zmax = so.z + halfSl;

  glm::vec3 p = glm::vec3(xmax, ymax, zmax);
  std::vector<glm::vec3> point = {
                                   glm::vec3(xmax, so.y, so.z),so,
                                   glm::vec3(so.x,ymax, so.z), so,
                                   glm::vec3(so.x,so.y,zmax), so,
                                   };

  std::vector<glm::vec3> normal =  {
                                   glm::vec3(1,0,0)          , glm::vec3(-1,0,0),
                                   glm::vec3(0,1,0)          , glm::vec3(0,-1,0),
                                   glm::vec3(0,0,1)          , glm::vec3(0,0,-1),
                                   };

  float closest = 100000000000;
  glm::vec3 normalA;
  glm::vec3 intersectionA;
  for (int i = 0; i < normal.size(); i++){
    float res = glm::dot(normal[i], d);

    if (res == 0.0f) continue;
    else {
      float t = glm::dot(point[i] - o, normal[i]) / glm::dot(d, normal[i]);

      glm::vec3 intersection =  t *d + o;

      bool isOnCube = intersection.x <= xmax && intersection.x >=xmin && intersection.y <= ymax && intersection.y >=ymin &&intersection.z <= zmax && intersection.z >=zmin;

      if (isOnCube && t < closest && t > 0.1) {
        closest = t;
        intersectionA = intersection;
        normalA = normal[i];
      }
    }
  }

  if (closest == 100000000000)  throw "no match";
  return std::vector<glm::vec3> {glm::vec3(closest), normalA};
}
Primitive::~Primitive()
{

}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Sphere::~Sphere()
{

}

std::vector<glm::vec3> Sphere::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
    return sphereIntersection(o, d, m_pos, 1.0);
}

glm::vec3 Sphere::normal(glm::vec3 intersect){
  return intersect - m_pos;
}

void Sphere::setOrigin(glm::vec3 origin){
  m_pos = origin;
}

void Primitive::setOrigin(glm::vec3 origin){
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Cube::~Cube()
{
}

std::vector<glm::vec3> Cube::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
    glm::vec3 tmp;
    if (storeNormal) return squareIntersection(o, d, m_pos, m_size, &normalStored);
    else return squareIntersection(o, d, m_pos, m_size, &tmp);
}

glm::vec3 Cube::normal(glm::vec3 intersect){
    return normalStored;
}

void Cube::setOrigin(glm::vec3 origin){
  m_pos = origin;
}
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

NonhierSphere::~NonhierSphere()
{

}

std::vector<glm::vec3> NonhierSphere::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
  return sphereIntersection(o, d, m_pos, m_radius);
}

glm::vec3 NonhierSphere::normal(glm::vec3 intersect){
  return (intersect-m_pos)/m_radius;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

NonhierBox::~NonhierBox()
{
}

std::vector<glm::vec3> NonhierBox::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
    glm::vec3 tmp;
    if (storeNormal) return squareIntersection(o, d, m_pos, m_size, &normalStored);
    else return squareIntersection(o, d, m_pos, m_size, &tmp);
}
glm::vec3  NonhierBox::normal(glm::vec3 intersect){
  return normalStored;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Plane::~Plane()
// {
// }

// double Plane::intersection(glm::vec3 o, glm::vec3 d, bool storeNormal){
//     float t = glm::dot(m_pos - o, normal[i]) / glm::dot(d, normal[i]);

//       glm::vec3 intersection =  t *d + o;

//       bool isOnPlane = intersection.x <= xmax && intersection.x >=xmin && intersection.y <= ymax && intersection.y >=ymin &&intersection.z <= zmax && intersection.z >=zmin;

//       if (isOnCube && t < closest && t > 0.1) {
//         closest = t;
//         *intersectionNormal = normal[i];
//     }
// }
// glm::vec3  Plane::normal(glm::vec3 intersect){
//   return normalStored;
// }
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
