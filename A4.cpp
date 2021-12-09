#include <glm/ext.hpp>

#include "PhongMaterial.hpp"
#include "A4.hpp"
#include <vector>
#include <glm/gtx/io.hpp>
#include <map>
#include <thread>

using namespace std;


std::vector<GeometryNode *> objectGlobal;
std::map<string, Image> mappings = {};
Image resultImage;
std::list<const Light *> globalLights;
glm::vec3 globalAmbient;
int globalSamples = 5;

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
		objectGlobal.push_back(newNode);
		for (SceneNode * child : node->children) {
			// child->scale(node->scaleStored);
			applyRecursiveTransforms( child, newTrans);
		}

}

std::vector<glm::vec3> hit(glm::vec3 eye, glm::vec3 direction, GeometryNode *&closestObject, bool breakEarly,std::vector<GeometryNode *> &objects){
	double near  = 100000000;
	std::vector<glm::vec3> interSectionInfo;
	for (int i = 0; i < objects.size(); i++){
		if (objects[i]->m_nodeType != NodeType::GeometryNode)
			continue;
		glm::vec3 transformedEye =  xyz(objects[i]->invtrans * glm::vec4(eye, 1.0));
		glm::vec3 transformedDirection = xyz(objects[i]->invtrans * (glm::vec4(direction, 0.0)));
		try{
			std::vector<glm::vec3> result = objects[i]->m_primitive->intersection(transformedEye, transformedDirection);
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

// glm::vec3 uniformHSample(){
// 	float r1= glm::linearRand(0,1);
// 	float r2 = glm::linearRand(0,1);

// 	float sinTheta = glm::sqrt(1 - r1 * r1); 
//     float phi = 2 * glm::pi<float>() * r2; 
//     float x = sinTheta * glm::cos(phi); 
//     float z = sinTheta * glm::sin(phi); 
//     return glm::vec3(x, u1, z); 
// }
glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction, int maxBounces,std::vector<GeometryNode *> &objects);

glm::vec3 shade(glm::vec3 intersection, glm::vec3 normal,glm::vec3 eye, PhongMaterial *pMaterial, 
			int maxBounces, glm::vec3 &uvCoordinates, glm::vec3 &t1, glm::vec3 &t2, std::vector<GeometryNode *> objects
			){
	float lightIntensity = 100.7;
	glm::vec3 finalColour = glm::vec3(0.0);

	for(const auto& light : globalLights){
		GeometryNode * inBetween = NULL;
		glm::vec3 betweenLightAndPoint = glm::normalize( light->position - intersection);
		hit(intersection, betweenLightAndPoint, inBetween, false, objects);
		if (inBetween != NULL) continue; //Point is in Shadow
		
		// TEXTURE MAPPING
		if (pMaterial->textureMapping != ""){
			int x = (int)(1024*uvCoordinates.x);
			int y = (int)(1024*uvCoordinates.y);
			finalColour += glm::vec3(mappings[pMaterial->textureMapping](x,y, 0), mappings[pMaterial->textureMapping](x,y, 1),
									mappings[pMaterial->textureMapping](x,y, 2));
		}
		// BUMP MAPPING
		if (pMaterial->normalMapping != ""){
			int x = ((int)(mappings[pMaterial->normalMapping].width()*uvCoordinates.x));
			int y = ((int)(mappings[pMaterial->normalMapping].height()*uvCoordinates.y)) ;
			glm::vec3 col = glm::vec3(mappings[pMaterial->normalMapping](x,y, 0), mappings[pMaterial->normalMapping](x,y, 1),
									mappings[pMaterial->normalMapping](x,y, 2));
			normal = glm::normalize(normal + col.x * glm::cross(t1, normal) + col.y*glm::cross(t2, normal));
		} 

		if (pMaterial->m_shininess == 6.666 && maxBounces!=0) { //special shiny value to make an object reflective
			glm::vec3 reflectingRay = 2 * (glm::dot(betweenLightAndPoint, normal) * normal) + betweenLightAndPoint;
			maxBounces--;
			finalColour +=  iterate(intersection, reflectingRay,  maxBounces, objects);
		}



		float distance = glm::distance(intersection, light->position);
		glm::vec3 diffuse= (1/(distance*distance))*light->colour * lightIntensity*pMaterial->m_kd  * glm::max(0.0f,glm::dot(normal,  betweenLightAndPoint));

		glm::vec3 h = glm::normalize(betweenLightAndPoint + (eye - intersection));
		glm::vec3 specular = light->colour * pMaterial->m_ks * glm::pow(glm::max(0.0f, glm::dot(normal, h)), pMaterial->m_shininess);

		glm::vec3 pathTracing = glm::vec3(0);
		for (int i = 0; i < globalSamples; i++){
			// glm:: vec3 random = uniformHSample();
			// pathTracing += iterate(intersection, random, maxBounces-1, objects );
		}

		finalColour += diffuse;
		finalColour += specular;
	}
	return finalColour;
}

glm::vec3 iterate(glm::vec3 eye, glm::vec3 direction,  int maxBounces,std::vector<GeometryNode *> &objects){
	GeometryNode * closestObject =NULL;
	std::vector<glm::vec3> stuff = hit(eye, direction, closestObject, false, objects);

	double near = stuff[0].x;
	glm::vec3 objNormal = stuff[1];
	glm::vec3 colour = glm::vec3(0.0);
	if(closestObject != NULL) {
		glm::vec3 intersection = eye+  direction *near;
		glm::vec3 normal = glm::normalize(glm::transpose(glm::mat3(closestObject->invtrans))* objNormal);
		PhongMaterial* pMaterial = static_cast<PhongMaterial*>(closestObject->m_material);

		glm::vec3 uvCoordinates = glm::vec3(0);
		glm::vec3 t1 = glm::vec3(0);
		glm::vec3 t2 = glm::vec3(0);
		if (stuff.size() >= 3) {
			uvCoordinates = stuff[2];
			t1 = glm::normalize(glm::transpose(glm::mat3(closestObject->invtrans)) * stuff[3]);
			t2 = glm::normalize( glm::transpose(glm::mat3(closestObject->invtrans)) *  stuff[4]);
		}
		colour+= globalAmbient + shade(intersection, normal, eye, pMaterial, maxBounces, uvCoordinates, t1, t2, objects);
	}
	return colour;
}

//Takes in x (column we operate on) and y (rows  operate on)
void multiThreadedIterate(int start, int end, int width, int height, int imageWidth, 
				int imageHeight, int d, bool antiAliasing, int superSample, glm::vec3 eye, glm::vec3 u, glm::vec3 v, glm::vec3 w,
				std::vector<GeometryNode *> objects){
	for (uint y = start; y < end; ++y) {	
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

						colour += iterate(eye, direction,2, objects);
					}
				}
				colour /= superSample*superSample;
			} else {
				float xOffset = x+ 0.5;
				float yOffset = y+ 0.5;
				glm::vec3 pixel = glm::vec3(imageWidth * ((float) (xOffset/width)  -0.5), imageHeight * ((float)(yOffset /height)-0.5),d/2);
				glm::vec3 pixelInWorld = pixel.x * u + pixel.y * v + pixel.z * w + eye;
				glm::vec3 direction = glm::normalize(eye-pixelInWorld) ;
				colour = iterate(eye, direction,  2, objects);
			}
			resultImage(width-x, y, 0) = (double)colour.x;
			resultImage(width-x, y, 1) = (double)colour.y;
			resultImage(width-x, y, 2) = (double)colour.z;
		}
	}
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
		const std::list<Light *> & lights,
		int superSample,
		int numberOfThreads, int pathTracingSamples
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
		globalLights.push_back(light);
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t height = image.height();
	size_t width = image.width();

	resultImage = Image(height, width);
	globalAmbient = glm::vec3(ambient.x, ambient.y, ambient.z);
	fovy = glm::radians(fovy);
	float d = glm::distance(eye, view);
	
	glm::vec3 w = (eye - view) / glm::length(eye -view);
	glm::vec3 u = glm::cross(up, w) / glm::length(glm::cross(up, w));
	glm::vec3 v = glm::cross(w, u);
	applyRecursiveTransforms(root, glm::mat4(1.0));

  	const double imageHeight = d/2* glm::tan(fovy);
  	const double imageWidth = (width / height) * imageHeight;

	globalSamples = pathTracingSamples;
	bool antiAliasing = true;
	if (superSample == 1) antiAliasing = false;

	std::vector<string> listOfImages = {"brick_texture.png", "brick_normal.png", "wood_texture.png", "wood_texture.png"};
	for (int i = 0; i < listOfImages.size(); i++){
		Image k = Image(1024,1024);
		k.loadPng(listOfImages[i]);
		mappings.insert ( std::pair<string, Image>(listOfImages[i],  k));
	}
	std::vector<std::thread> threads;

	for (uint index = 1; index <= numberOfThreads; index++ ){
		// std::vector<GeometryNode*> objectsNew;
		// for (int i = 0; i < objectGlobal.size();i++){
		// 	GeometryNode *node = new GeometryNode(objectGlobal[i]->name, objectGlobal[i]->name);
		// 	objectsNew.push_back(node);
		// }
		threads.push_back(std::thread(
			multiThreadedIterate, (index-1) * height/numberOfThreads, index *height/numberOfThreads,
								width, height, imageWidth, imageHeight, d, antiAliasing, superSample, eye, u, v, w, objectGlobal
								));
	}
	for (auto& th : threads)  th.join();
	image = resultImage;
}
