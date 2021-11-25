#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include "JointNode.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
	m_nodeType(NodeType::SceneNode),
	trans(mat4(1.0)),
	isSelected(false),
	m_nodeId(nodeInstanceCount++)
{	
}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
	: m_nodeType(other.m_nodeType),
	  m_name(other.m_name),
	  trans(other.trans),
	  invtrans(other.invtrans)
{
	for(SceneNode * child : other.children) {
		this->children.push_front(new SceneNode(*child));
	}
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
	for(SceneNode * child : children) {
		delete child;
	}
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
	trans = m;
	invtrans = m;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
	return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
	return invtrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {

	child->parent = this;

	children.push_back(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
	children.remove(child);
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
	vec3 rot_axis;

	switch (axis) {
		case 'x':
			rot_axis = vec3(1,0,0);
			xRotAngle += degreesToRadians(angle);
			break;
		case 'y':
			rot_axis = vec3(0,1,0);
			yRotAngle += degreesToRadians(angle);
	        break;
		case 'z':
			rot_axis = vec3(0,0,1);
			zRotAngle += degreesToRadians(angle);
	        break;
		default:
			break;
	}
	mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
	trans = rot_matrix * trans;
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
	scaleStored = amount * scaleStored;
	set_transform( glm::scale(amount) * trans );
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
	trans = glm::translate(amount) * trans;
	translationStored = amount + translationStored;
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
	return nodeInstanceCount;
}


//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

	//os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
	switch (node.m_nodeType) {
		case NodeType::SceneNode:
			os << "SceneNode";
			break;
		case NodeType::GeometryNode:
			os << "GeometryNode";
			break;
		case NodeType::JointNode:
			os << "JointNode";
			break;
	}
	os << ":[";

	os << "name:" << node.m_name << ", ";
	os << "id:" << node.m_nodeId;
	os << "]";

	return os;
}

void SceneNode::execute(float angle, char axis){
	if (parent != nullptr){
		const JointNode * newParent =  static_cast<const JointNode *> (parent);
		if (axis == 'x'){
			if (angle > newParent->m_joint_x.max) angle = newParent->m_joint_x.max;
			if (angle < newParent->m_joint_x.min) angle = newParent->m_joint_x.min;

			
			parent->xRotAngle = degreesToRadians(angle);
			currentJointAngle.x = angle;
		}
		else if (axis == 'y'){
			if (angle > newParent->m_joint_y.max) angle = newParent->m_joint_y.max;
			if (angle < newParent->m_joint_y.min) angle = newParent->m_joint_y.min;

			parent->yRotAngle = degreesToRadians(angle);
			currentJointAngle.y = angle;
		}
	}

}

