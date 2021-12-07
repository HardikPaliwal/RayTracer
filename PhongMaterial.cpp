#include "PhongMaterial.hpp"
#include <iostream>

PhongMaterial::PhongMaterial(
	const glm::vec3& kd, const glm::vec3& ks, double shininess , std::string textureMapping, std::string normalMapping)
	: m_kd(kd)
	, m_ks(ks)
	, m_shininess(shininess) 
	, textureMapping(textureMapping)
	, normalMapping(normalMapping)
{
}

// void PhongMaterial::setUp(){
	// if (normalMapping != ""){
	// 	// normalMappingIm = Image(1024, 1024);
	// 	normalMappingIm.loadPng(normalMapping);
	// 	// normalMappingIm = map;
	// }
	// if (textureMapping != ""){
	// 	// textureMappingIm = Image(1024, 1024);
	// 	textureMappingIm.loadPng(textureMapping);
	// 	// textureMappingIm = map2;
	// }
// }
PhongMaterial::~PhongMaterial()
{
}

