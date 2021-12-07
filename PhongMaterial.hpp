#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"
#include "string"
#include "Image.hpp"
#include <ostream>

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess, std::string textureMapping, std::string normalMapping);
  virtual ~PhongMaterial();
  glm::vec3 m_kd;
  glm::vec3 m_ks;

  std::string normalMapping; 
  std::string textureMapping;

  Image normalMappingIm =Image(1024, 1024);
  Image textureMappingIm=Image(1024, 1024);

  double m_shininess;
};

