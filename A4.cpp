#include <glm/ext.hpp>

#include "PhongMaterial.hpp"
#include "A4.hpp"
#include <vector>
#include <glm/gtx/io.hpp>
using namespace std;


std::vector<GeometryNode *> objects;

//Cause the glm version isn't working =(
glm::vec3  xyz(glm::vec4 thing){
	return glm::vec3(thing.x, thing.y, thing.z);
}

glm::vec3  xyzdw(glm::vec4 thing){
	return glm::vec3(thing.x, thing.y, thing.z)/thing.w;
}
void applyRecursiveTransforms(SceneNode * node, glm::mat4 trans){
		// glm::mat4 newTrans = trans;
		// newTrans = glm::rotate(newTrans, node->rotationStored.x, glm::vec3(1.f,0.f,0.f));
		// newTrans = glm::rotate(newTrans, node->rotationStored.y, glm::vec3(0.f,1.f,0.f));
		// newTrans = glm::rotate(newTrans, node->rotationStored.z, glm::vec3(0.f,0.f,1.f));
		// newTrans = glm::translate(newTrans,node->translationStored);
		// node->set_transform( glm::scale(newTrans,node->scaleStored));

		glm::mat4 newTrans = trans * node->get_transform();

		node->set_transform( newTrans  );
		// newTrans = glm::scale(newTrans, glm::vec3(1/node->scaleStored.x, 1/node->scaleStored.y, 1/node->scaleStored.z));
		GeometryNode *newNode= static_cast<GeometryNode *> (node);
		objects.push_back(newNode);
		for (SceneNode * child : node->children) {
			// child->scale(node->scaleStored);
			applyRecursiveTransforms( child, newTrans);
		}
}

double hit(glm::vec3 eye, glm::vec3 direction, GeometryNode *&closestObject, bool breakEarly){
	double near  = 100000000;
	for (int i = 0; i < objects.size(); i++){
		if (objects[i]->m_nodeType != NodeType::GeometryNode)
			continue;
		glm::vec3 transformedEye =  xyz(objects[i]->invtrans * glm::vec4(eye, 1.0));
		glm::vec3 transformedDirection = xyz(objects[i]->invtrans * (glm::vec4(direction, 0.0)));
		try{
			double t = objects[i]->m_primitive->intersection(transformedEye, transformedDirection,! breakEarly);
			if (t < near ) {
				near = t;
				closestObject = objects[i];
				if(breakEarly) return t;
			}
		} catch (...){
		}
	}
	return near;
}

glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction, const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces);

glm::vec3 shade(glm::vec3 intersection, glm::vec3 normal, PhongMaterial *pMaterial,const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces){
	float lightIntensity = 0.7;
	glm::vec3 finalColour = glm::vec3(0.0);

	for(const auto& light : lights){
		GeometryNode * inBetween = NULL;
		glm::vec3 betweenLightAndPoint = glm::normalize(light->position - intersection);
		hit(intersection, betweenLightAndPoint, inBetween, false);
		if (inBetween != NULL) continue; //Hit something so dont calculate the colour

		if (pMaterial->m_shininess == 6.666 && maxBounces!=0) { //special shiny value to make an object reflective
			glm::vec3 reflectingRay = 2 * (glm::dot(betweenLightAndPoint, normal) * normal) + betweenLightAndPoint;
			maxBounces--;
			finalColour += 0.4 * iterate(intersection, reflectingRay, lights, ambient, maxBounces);
		}
		float distance = glm::distance(intersection, light->position);
		glm::vec3 tmp= light->colour * lightIntensity*pMaterial->m_kd  * glm::max(0.0f,glm::dot(normal,  betweenLightAndPoint));
		finalColour += tmp;
	}
	return finalColour;
}

glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction, const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces){
	GeometryNode * closestObject =NULL;
	float near = hit(eye, direction, closestObject, false);

	glm::vec3 colour = glm::vec3(0.0);
	if(closestObject != NULL) {
		glm::vec3 intersection = eye+  direction *near;
		glm::vec3 normal = glm::normalize(glm::transpose(glm::mat3(closestObject->invtrans))* closestObject->m_primitive->normal(xyz(closestObject->invtrans*glm::vec4(intersection, 1.0))));
		PhongMaterial* pMaterial = static_cast<PhongMaterial*>(closestObject->m_material);

		colour+= ambient + shade(intersection, normal, pMaterial, lights, ambient, maxBounces);
		// cout << colour << endl << endl;
	}
	return colour;
}
void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t height = image.height();
	size_t width = image.width();
	fovy = glm::radians(fovy);
	float d = glm::distance(eye, view);
	
	glm::vec3 w = (eye - view) / glm::length(eye -view);
	glm::vec3 u = glm::cross(up, w) / glm::length(glm::cross(up, w));
	glm::vec3 v = glm::cross(w, u);
	applyRecursiveTransforms(root, glm::mat4(1.0));

  	const double imageHeight = d/2* glm::tan(fovy);
  	const double imageWidth = (width / height) * imageHeight;

	Image textureMapping= Image(1024,1024);
	textureMapping.loadPng("wood_texture.png");

	
	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			glm::vec3 pixel = glm::vec3(imageWidth * ((float) ((x + 0.5)/width)  -0.5), imageHeight * ((float)((y + 0.5)/height)-0.5),d/2);
			glm::vec3 pixelInWorld = pixel.x * u + pixel.y * v + pixel.z * w + eye;
			glm::vec3 direction = glm::normalize(eye-pixelInWorld) ;
			glm::vec3 colour = iterate(eye, direction, lights, ambient, 2);

			image(width-x, y, 0) = (double)colour.x;
			// Green: 
			image(width-x, y, 1) = (double)colour.y;
			// Blue: 
			image(width-x, y, 2) = (double)colour.z;
		}
	}
}
