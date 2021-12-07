#pragma once

#include <glm/glm.hpp>
#include <vector>

class Primitive {
public:
  virtual ~Primitive();
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal) =0;
  virtual glm::vec3 normal(glm::vec3 intersect)=0;
  virtual void setOrigin(glm::vec3 origin);

};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
  virtual glm::vec3 normal(glm::vec3 intersect);
  virtual void setOrigin(glm::vec3 origin);

  glm::vec3 m_pos =glm::vec3(0.0);
  double m_radius = 1.0;
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
  virtual glm::vec3 normal(glm::vec3 intersect);
  virtual void setOrigin(glm::vec3 origin);

  glm::vec3 m_pos=  glm::vec3(0.0);
  double m_size = 1.0;
  glm::vec3 normalStored;

};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
  virtual glm::vec3 normal(glm::vec3 intersect);

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual ~NonhierBox();
  virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
  virtual glm::vec3 normal(glm::vec3 intersect);

private:
  glm::vec3 m_pos;
  double m_size;
  glm::vec3 normalStored;
};

// class Plane : public Primitive {
//   public:
//     Plane(const glm::vec3& pos, double size): m_pos(pos), m_size(size)
//   {
//   }
  
//   virtual ~Plane();
//   virtual std::vector<glm::vec3> intersection(glm::vec3 origin, glm::vec3 ray, bool storeNormal);
//   virtual glm::vec3 normal(glm::vec3 intersect);

// private:
//   glm::vec3 m_pos;
//   double m_size;
//   glm::vec3 normalStored;
// };
