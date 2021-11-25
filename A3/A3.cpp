#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj"),
			getAssetFilePath("dome.obj"),
			getAssetFilePath("torus.obj")

	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	resetPosition();

	setShaderMode(positionMode);
	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}


void A3::applyRecursiveTransforms(SceneNode & node, glm::mat4 trans){
		glm::mat4 newTrans = glm::translate(trans,node.translationStored);
		newTrans = glm::rotate(newTrans, node.xRotAngle, glm::vec3(1.f,0.f,0.f));
		newTrans = glm::rotate(newTrans, node.yRotAngle, glm::vec3(0.f,1.f,0.f));
		newTrans = glm::rotate(newTrans, node.zRotAngle, glm::vec3(0.f,0.f,1.f));

		node.set_transform( glm::scale(newTrans,node.scaleStored) );
		for (SceneNode * child : node.children) {
			applyRecursiveTransforms( *child, newTrans);
		}
}
//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(0.0f, 0.0f, 30.0f);
	m_light.rgbIntensity = vec3(0.4f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.25f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	static int count(1); 

	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
		
		if (ImGui::BeginMenuBar()){
			if(ImGui::BeginMenu("Application")){
				if(ImGui::MenuItem("Reset Position (I)")) resetPosition();
				if(ImGui::MenuItem("Reset Orientation (O)")) resetOrientation();
				if(ImGui::MenuItem("Reset Joints (S)")) resetJoints(*m_rootNode);
				if(ImGui::MenuItem("Reset All (A)")) {
					resetPosition();
					resetJoints(*m_rootNode);
					resetOrientation();
				}
				if( ImGui::Button( "Quit (Q)" ) ) glfwSetWindowShouldClose(m_window, GL_TRUE);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Edit")){
				if(ImGui::MenuItem("Undo (U)")) undo();
				if(ImGui::MenuItem("Redo (R)")) redo();
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("options")){
				if(ImGui::Checkbox("Circle (C)", &showTrackBallCircle)) {
					// resetHandler(0);
				}
				if(ImGui::Checkbox("Z-buffer (Z)",  &depthBuffer)) {
					// resetHandler(0);
				}
				if(ImGui::Checkbox("Backface Culling (B)", &backFace)) {
					// resetHandler(0);
				}if(ImGui::Checkbox("Frontface Culling(F)" , &frontFace)) {
					// resetHandler(0);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		int radioButton = 1;
		if (positionMode) radioButton = 0;
		if( ImGui::RadioButton( "Position Mode (P)", &radioButton ,0 ) ){
			positionMode = true;
			setShaderMode(positionMode);
			unSelectEveryNode(*m_rootNode);
		}
		if( ImGui::RadioButton( "Joint Mode (J)", &radioButton, 1 )) {
			positionMode = false;
			setShaderMode(positionMode);
		}
		// // Create Button, and check if it was clicked:
		// if( ImGui::Button( "Quit Application" ) ) {
		// 	glfwSetWindowShouldClose(m_window, GL_TRUE);
		// }

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

		ImGui::Text( errorMessage.c_str() );

	count++;
	if (count % 300 == 0) errorMessage = "";
	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {
	applyRecursiveTransforms(*m_rootNode, scaleMatrix*zAxisRotation*translationMatrix*trackBallMatrix);
	if (depthBuffer) glEnable( GL_DEPTH_TEST );
	if(backFace || frontFace) glEnable(GL_CULL_FACE);
	if (backFace) glCullFace(GL_BACK);
	if (frontFace) glCullFace(GL_FRONT);
	if (backFace && frontFace) glCullFace(GL_FRONT_AND_BACK);

	renderSceneGraph(*m_rootNode);

	if (depthBuffer) glDisable( GL_DEPTH_TEST );
	if(backFace || frontFace) glDisable(GL_CULL_FACE);

	renderArcCircle();
}



void A3::renderChild(const SceneNode & root, glm::mat4 transSoFar){
	glm::mat4 parentTrans = root.get_transform();

	for (const SceneNode * node : root.children) {

		renderChild(*node, parentTrans);

		if (node->m_nodeType != NodeType::GeometryNode)
			continue;
		
		const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);
		
		updateShaderUniforms(m_shader, *geometryNode, m_view );

		if (!positionMode) {
			m_shader.enable();
			glUniform1f(m_shader.getUniformLocation("objectId"), (float)node->m_nodeId/ 100);
			glUniform1f(m_shader.getUniformLocation("selected"), node->selected);
			CHECK_GL_ERRORS;
			m_shader.disable();
		} 

		// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		//-- Now render the mesh:
		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}
}
//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	// glm::mat4 parentTrans = root.get_transform();

	// for (const SceneNode * node : root.children) {

		// if (node->m_nodeType != NodeType::GeometryNode)
		// 	continue;
		
		// const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

		renderChild(root, glm::mat4(1.0));
		
	// 	updateShaderUniforms(m_shader, *geometryNode, m_view);
	// 	// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	// 	BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

	// 	//-- Now render the mesh:
	// 	m_shader.enable();
	// 	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	// 	m_shader.disable();
	// }

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	if (!showTrackBallCircle) return;
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


glm::vec3 A3::convertToSphere(double x, double y, float radius){
	float newY = y - (m_framebufferHeight/2);
	float newX = x - (m_framebufferWidth/2);
	float newZ = glm::sqrt(glm::pow(radius, 2) - glm::pow(newX,2)- glm::pow(newY, 2));

	return glm::vec3(newX, newY, newZ);
}

void A3::setShaderMode(bool positionMode){
	m_shader.enable();
	GLint location = m_shader.getUniformLocation("positionMode");
	glUniform1i(location, positionMode);
	CHECK_GL_ERRORS;
	m_shader.disable();
}

SceneNode * A3::findNodeById(SceneNode &node, int id){
	if (node.m_nodeId == id) return &node;
	for (SceneNode * child : node.children) {
		SceneNode * maybe = findNodeById( *child, id);
		if (maybe!= nullptr) return maybe;
	}
	return nullptr;
}

void A3::unSelectEveryNode(SceneNode &node){
	node.selected = false;
	for (SceneNode * child : node.children) {
		unSelectEveryNode( *child);
	}
}

std::vector<int> A3::returnIdOfEverySelectedNode(SceneNode &node, vector<int> soFar){
	if(node.selected) soFar.push_back(node.m_nodeId);

	for (SceneNode * child : node.children) {
		soFar = returnIdOfEverySelectedNode( *child, soFar);
	}
	return soFar;
}

void A3::undo(){
	if ((positionInStack -1) <= -1){
		errorMessage = "Cannot undo any further";
	} else{
		positionInStack--;
		for (int i = 0; i < undoStack[positionInStack].size(); i++){
			SceneNode * node = findNodeById(*m_rootNode, undoStack[positionInStack][i]);
			node->execute(undoAngle[positionInStack].x, 'x');
			node->execute(undoAngle[positionInStack].y, 'y');
		}
	}

}
void A3::redo(){
	if(positionInStack == undoStack.size() - 1 || positionInStack == -1){
		errorMessage = "Cannot redo any further";
	} else {
		positionInStack++;
		for (int i = 0; i < undoStack[positionInStack].size(); i++){
			SceneNode * node = findNodeById(*m_rootNode, undoStack[positionInStack][i]);
			node->execute(undoAngle[positionInStack].x, 'x');
			node->execute(undoAngle[positionInStack].y, 'y');
		}
	}
}

void A3::clearUndoStack(int position){
	if (undoStack.size() != 0){
		undoStack.erase(undoStack.begin() + position, undoStack.begin() + undoStack.size() - 1);
		undoAngle.erase(undoAngle.begin() + position, undoAngle.begin() + undoAngle.size() - 1);
	}
}

void A3::resetPosition(){
	translationMatrix = glm::mat4(1.0);
}

void A3::resetOrientation(){
	trackBallMatrix = glm::mat4(1.0);
	zAxisRotation = glm::mat4(1.0);
}

void A3::resetJoints(SceneNode &node){
	if (node.m_nodeId == 1){
		undoStack = std::vector<std::vector<int>>({}); //only want to do this once but im too lazy to create another method to do this properly	
		undoAngle= std::vector<glm::vec3>({});
		positionInStack = -1;
	} 
	node.execute(0.0, 'x');
	node.execute(0.0, 'y');

	node.selected = false;
	for (SceneNode * child : node.children) {
		resetJoints(*child);
	}
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);
	glm::vec2 diff = prevMouse - glm::vec2(xPos, yPos );
	if (positionMode){
		if (isLeftMousePressed)
			translationMatrix = glm::translate(translationMatrix, glm::vec3(glm::inverse(zAxisRotation) * glm::vec4(-diff.x / 25, diff.y / 25, 0.0,1.0))) ;
		if (isMiddleMousePressed)
			translationMatrix = glm::translate(translationMatrix, glm::vec3(0, 0,  -diff.y/5) );
		if (isRightMousePressed){
			float radius = glm::min(m_framebufferHeight, m_framebufferWidth)/4;
			glm::vec3 onSphere = convertToSphere(xPos, yPos, radius);

			if  (!glm::isnan(onSphere.z) ) { // This means that the point is in our circle. Thus we must apply track ball logic.
				glm::vec3 prevOnSphere = convertToSphere(prevMouse.x, prevMouse.y, radius);

				if (glm::isnan(prevOnSphere.z)) return eventHandled;

				glm::vec3 axisOfRot = glm::cross(prevOnSphere, onSphere);
				float angle =2* glm::asinh(glm::length(axisOfRot)/ (glm::length(onSphere) * glm::length(prevOnSphere)));
				trackBallMatrix  = glm::rotate(trackBallMatrix, angle, axisOfRot);
			} 
			else
				zAxisRotation = glm::rotate(zAxisRotation, diff.x/30, glm::vec3(0,0,1));
		}
		
	} else {

		if (isMiddleMousePressed){
			if (positionInStack == -1 || positionInStack == undoStack.size())  return true;
			glm::vec3 old = undoAngle[positionInStack];
			float newAngle = old.x + diff.y;
			undoAngle[positionInStack] = glm::vec3(newAngle, old.y, old.z);

			for (int i = 0; i < undoStack[positionInStack ].size();i++){
				findNodeById(*m_rootNode,undoStack[positionInStack][i])->execute(newAngle, 'x');
			}
		}
		if (isRightMousePressed){
			if (positionInStack == -1 || positionInStack == undoStack.size())  return true;
			glm::vec3 old = undoAngle[positionInStack];
			float newAngle = old.y + diff.y;
			undoAngle[positionInStack] = glm::vec3(old.x, newAngle, old.z);
			for (int i = 0; i < undoStack[positionInStack ].size();i++){
				SceneNode *node = findNodeById(*m_rootNode,undoStack[positionInStack][i]);
				if (node->m_name == "head") node->execute(newAngle, 'y');
			}
		}

	}

	prevMouse = glm::vec2(xPos, yPos);
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		bool action = !(actions == GLFW_RELEASE) ;
		
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			isLeftMousePressed = action;
			if (!positionMode){
				glm::vec3 pixel;
				glReadPixels(prevMouse.x, m_framebufferHeight - prevMouse.y, 1, 1, GL_RGB, GL_FLOAT, &pixel);
				int id = std::roundf(pixel.x * 100);

				if (id == 0 || id > m_rootNode->totalSceneNodes()) return true; // Selected the background

				SceneNode* clicked = findNodeById(*m_rootNode, id);
			
				if (clicked != nullptr && action && clicked->parent->m_nodeType == NodeType::JointNode) {
					clicked->selected = !clicked->selected;
					undoStack.push_back(std::vector<int>({id}));
					undoAngle.push_back(clicked->currentJointAngle);
					positionInStack++;
				}
				if (!action) clearUndoStack(positionInStack);
			}
		} 
		if (button == GLFW_MOUSE_BUTTON_RIGHT){
			isRightMousePressed = action;
			if (! positionMode && action){
				std::vector<int> selected = returnIdOfEverySelectedNode(*m_rootNode, std::vector<int> ({}));
				undoStack.push_back(selected);
				undoAngle.push_back(glm::vec3(0.0));
				positionInStack++;
			}
		}  
		if (button == GLFW_MOUSE_BUTTON_MIDDLE){
			isMiddleMousePressed = action;
			if (! positionMode && action){
				std::vector<int> selected = returnIdOfEverySelectedNode(*m_rootNode, std::vector<int> ({}));
				undoStack.push_back(selected);
				undoAngle.push_back(glm::vec3(0.0));
				positionInStack++;
			} 
			if(!action) clearUndoStack(positionInStack);
		}  
	}
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(true);

	if( action == GLFW_PRESS ) {
		switch(key){
			case (GLFW_KEY_M):
				show_gui = !show_gui;
				eventHandled = true;
			break;
			case (GLFW_KEY_I):
				resetPosition();
			break;
			case (GLFW_KEY_O):
				resetOrientation();
			break;
			case (GLFW_KEY_S):
				resetJoints(*m_rootNode);
			break;
			case (GLFW_KEY_A):
				resetPosition();
				resetJoints(*m_rootNode);
				resetOrientation();
			break;
			case (GLFW_KEY_U):
				undo();
			break;
			case (GLFW_KEY_R):
				redo();
			break;
			case (GLFW_KEY_P):
				positionMode = true;
				setShaderMode(positionMode);
				unSelectEveryNode(*m_rootNode);
			break;
			case (GLFW_KEY_J):
				positionMode = false;
				setShaderMode(positionMode);
			break;
			case (GLFW_KEY_Q):
				glfwSetWindowShouldClose(m_window, GL_TRUE);
			break;
			case(GLFW_KEY_C):
				showTrackBallCircle= !showTrackBallCircle;
			break;
			case(GLFW_KEY_Z):
				depthBuffer = !depthBuffer;
			break;
			case(GLFW_KEY_B):
				backFace = !backFace;
			break;
			case(GLFW_KEY_F):
				frontFace = !frontFace;
			break;

		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
