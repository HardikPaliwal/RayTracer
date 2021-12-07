#include <glm/ext.hpp>

#include "PhongMaterial.hpp"
#include "A4.hpp"
#include <vector>
#include <glm/gtx/io.hpp>
#include <map>

using namespace std;


std::vector<GeometryNode *> objects;

std::map<string, Image> mappings = {};
//Cause the glm version isn't working =(
glm::vec3  xyz(glm::vec4 thing){
	return glm::vec3(thing.x, thing.y, thing.z);
}

glm::vec3  xyzdw(glm::vec4 thing){
	return glm::vec3(thing.x, thing.y, thing.z)/thing.w;
}
void applyRecursiveTransforms(SceneNode * node, glm::mat4 trans){
		glm::mat4 newTrans = trans;
		newTrans = glm::translate(newTrans,node->translationStored);
		newTrans = glm::rotate(newTrans, node->rotationStored.x, glm::vec3(1.f,0.f,0.f));
		newTrans = glm::rotate(newTrans, node->rotationStored.y, glm::vec3(0.f,1.f,0.f));
		newTrans = glm::rotate(newTrans, node->rotationStored.z, glm::vec3(0.f,0.f,1.f));
		node->set_transform( glm::scale(newTrans,node->scaleStored));


		// glm::mat4 newTrans = trans * node->get_transform();

		// node->set_transform( newTrans  );
		// newTrans = glm::scale(newTrans, glm::vec3(1/node->scaleStored.x, 1/node->scaleStored.y, 1/node->scaleStored.z));
		GeometryNode *newNode= static_cast<GeometryNode *> (node);
		// if (newNode->m_material != nullptr){
		// 	PhongMaterial* pMaterial = static_cast<PhongMaterial*>(newNode->m_material);
		// 	if(pMaterial->textureMapping != ""){
		// 		// Image i = Image(1024, 1024);
		// 		// i.loadPng(pMaterial->textureMapping);
		// 	}
		// 	if (pMaterial->normalMapping != ""){
		// 		// Image i = Image(1024, 1024);
		// 		// i.loadPng(pMaterial->normalMapping);
		// 	}

		// }

		objects.push_back(newNode);
		for (SceneNode * child : node->children) {
			// child->scale(node->scaleStored);
			applyRecursiveTransforms( child, newTrans);
		}

}

std::vector<glm::vec3> hit(glm::vec3 eye, glm::vec3 direction, GeometryNode *&closestObject, bool breakEarly){
	double near  = 100000000;
	std::vector<glm::vec3> interSectionInfo;
	for (int i = 0; i < objects.size(); i++){
		if (objects[i]->m_nodeType != NodeType::GeometryNode)
			continue;
		glm::vec3 transformedEye =  xyz(objects[i]->invtrans * glm::vec4(eye, 1.0));
		glm::vec3 transformedDirection = xyz(objects[i]->invtrans * (glm::vec4(direction, 0.0)));
		try{
			std::vector<glm::vec3> result = objects[i]->m_primitive->intersection(transformedEye, transformedDirection,!breakEarly);
			double t = result[0].x;
			if (t < near) {
				near = t;
				interSectionInfo = result;
				closestObject = objects[i];
				if(breakEarly) return interSectionInfo;
			}
		} catch (...){
		}
	}
	return interSectionInfo;
}

glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction, const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces);

glm::vec3 shade(glm::vec3 intersection, glm::vec3 normal, PhongMaterial *pMaterial,const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces){
	float lightIntensity = 250.7;
	glm::vec3 finalColour = glm::vec3(0.0);

	for(const auto& light : lights){
		GeometryNode * inBetween = NULL;
		glm::vec3 betweenLightAndPoint = glm::normalize( light->position - intersection);
		hit(intersection, betweenLightAndPoint, inBetween, false);
		if (inBetween != NULL) continue; //Hit something so dont calculate the colour

		if (pMaterial->m_shininess == 6.666 && maxBounces!=0) { //special shiny value to make an object reflective
			glm::vec3 reflectingRay = 2 * (glm::dot(betweenLightAndPoint, normal) * normal) + betweenLightAndPoint;
			maxBounces--;
			finalColour += 0.4 * iterate(intersection, reflectingRay, lights, ambient, maxBounces);
		}
		
		if (pMaterial->textureMapping != ""){

		}
		if (pMaterial->normalMapping != ""){
			
		}

		float distance = glm::distance(intersection, light->position);
		glm::vec3 tmp= (1/(distance*distance))*light->colour * lightIntensity*pMaterial->m_kd  * glm::max(0.0f,glm::abs(glm::dot(normal,  betweenLightAndPoint)));

		finalColour += tmp;
	}
	return finalColour;
}

glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction, const std::list<Light *> & lights, glm::vec3 ambient, int maxBounces){
	GeometryNode * closestObject =NULL;
	std::vector<glm::vec3> stuff = hit(eye, direction, closestObject, false);

	double near = stuff[0].x;
	glm::vec3 objNormal = stuff[1];
	glm::vec3 colour = glm::vec3(0.0);
	if(closestObject != NULL) {
		glm::vec3 intersection = eye+  direction *near;
		glm::vec3 normal = glm::normalize(glm::transpose(glm::mat3(closestObject->invtrans))* objNormal);
		PhongMaterial* pMaterial = static_cast<PhongMaterial*>(closestObject->m_material);

		colour+= ambient + shade(intersection, normal, pMaterial, lights, ambient, maxBounces);
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

	bool antiAliasing = false;
	float superSample = 2;
	std::vector<string> listOfImages = {"brick_texture.png", "brick_normal.png", "wood_texture.png", "wood_texture.png"};
	for (int i = 0; i < listOfImages.size(); i++){
		Image k = Image(1024,1024);
		k.loadPng(listOfImages[i]);
		mappings.insert ( std::pair<string, Image>(listOfImages[i],  k));
	}

	for (uint y = 0; y < height; ++y) {
		for (uint x = 0; x < width; ++x) {
			glm::vec3 colour;
			if (antiAliasing){
				for (int i = 0; i < superSample; i++){
					float xOffset = x+ i/superSample;
					for (int j = 0; j< superSample; j++){
						float yOffset = y+ i/superSample;

						glm::vec3 pixel = glm::vec3(imageWidth * ((float) (xOffset/width)  -0.5), imageHeight * ((float)(yOffset /height)-0.5),d/2);
						glm::vec3 pixelInWorld = pixel.x * u + pixel.y * v + pixel.z * w + eye;
						glm::vec3 direction = glm::normalize(eye-pixelInWorld) ;
						colour += iterate(eye, direction, lights, ambient, 2);
					}
				}
				colour /= superSample*superSample;
			} else {
				float xOffset = x+ 0.5;
				float yOffset = y+ 0.5;
				glm::vec3 pixel = glm::vec3(imageWidth * ((float) (xOffset/width)  -0.5), imageHeight * ((float)(yOffset /height)-0.5),d/2);
				glm::vec3 pixelInWorld = pixel.x * u + pixel.y * v + pixel.z * w + eye;
				glm::vec3 direction = glm::normalize(eye-pixelInWorld) ;
				colour = iterate(eye, direction, lights, ambient, 2);
			}

			image(width-x, y, 0) = (double)colour.x;
			// Green: 
			image(width-x, y, 1) = (double)colour.y;
			// Blue: 
			image(width-x, y, 2) = (double)colour.z;
		}
	}
}
