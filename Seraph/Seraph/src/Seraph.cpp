#include "CommonIncludes.h"
#include "Seraph.h"

#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>

#include "Engine/Entity/World.h"

int initEngine();
int runEngine();

void clipVertices(std::vector<Vertex>& verts, std::vector<uint16_t>& indices, float scaleLeft, bool compareLessThan, float scaleRight, bool xUsed, bool flip);

int main() {

	Timer(program, "Seraph Game engine");
	logRecord("Seraph Engine has started");

	int state = initEngine();

	if (state) {
		throwError("Seraph Engine Initialisization Failed\n", logLevelCritical);
	}

	state = runEngine();

	if (state) {
		throwError("Failed to run Seraph Engine.\n", logLevelCritical);
	}
}

int initEngine() {
	Timer(engineInit, "Seraph Engine Initialisation");

	const char* name = "Seraph";
	int height = 540*2;
	int width = 960*2;

	seraph.rendererContext = Renderer::Context("Seraph", width, height);
	seraph.rendererContext.vertices.clear();
	seraph.rendererContext.indices.clear();
	seraph.rendererContext.vertices.push_back({ {  0.5f,  0.5f, 2.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ {  0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ {  0.5f, -0.5f, 2.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ {  0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ { -0.5f,  0.5f, 2.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ { -0.5f,  0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ { -0.5f, -0.5f, 2.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	seraph.rendererContext.vertices.push_back({ { -0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} });
	//seraph.rendererContext.vertices = { { {0,  1.0f},   {0.5f, 0.0f, 0.0f, 0.5f} },
	//								    { {0.5f, -0.5f},  {0.0f, 0.5f, 0.0f, 0.5f} },
	// 							        { {-0.5f, -0.5f}, {0.0f, 0.0f, 0.5f, 0.5f} } };
	seraph.rendererContext.indices = { 0, 1, 3, 0, 3, 2, 0, 2, 6, 0, 6, 4, 2, 3, 7, 6, 2, 7, 6, 7, 5, 4, 6, 5, 3, 1, 5, 3, 5, 7, 1, 0, 4, 1, 4, 5 };

	Renderer::initRenderer(seraph.rendererContext);

	int sizeOfPlayer = sizeof(Player);
	int sizeOfPPball = sizeof(Ball);
	int spaceRequiredInBytes = sizeOfPlayer * 2 + sizeOfPPball;
	seraph.memoryArena = MemoryArena(spaceRequiredInBytes);
	return 0;
}

int runEngine() {

	Timer(engineRun, "Seraph Engine Running");

	DataBlockCodeType p1_Code = seraph.memoryArena.allocateData(sizeof(Player));
	DataBlockCodeType p2_Code = seraph.memoryArena.allocateData(sizeof(Player));
	DataBlockCodeType ppBall_Code = seraph.memoryArena.allocateData(sizeof(Ball));

	Player* p1 = (Player*)seraph.memoryArena.getDataAddress(p1_Code);
	Player* p2 = (Player*)seraph.memoryArena.getDataAddress(p2_Code);
	Ball* ppBall = (Ball*)seraph.memoryArena.getDataAddress(ppBall_Code);

	// get initial data
	short xBuffer = screenWidth - wallBufferWidth;
	short yBuffer = screenHeight - wallBufferHeight;
	PlayerState state = PlayerState();
	Location p1Pos = Location(wallBufferWidth, wallBufferHeight);
	Location p2Pos = Location(xBuffer, wallBufferHeight);
	PlayerSize p1Size = PlayerSize(p1_puckWidth, p1_puckLength);
	PlayerSize p2Size = PlayerSize(p2_puckWidth, p2_puckLength);
	*p1 = Player(p1_Health, state, p1Pos, p1Size);
	*p2 = Player(p2_Health, state, p2Pos, p2Size);
	Location ppballPos = Location(0, 0);
	*ppBall = Ball(PingPongBall_Radius, ppballPos);

	// set world data
	seraph.rendererContext.vertices.clear();
	seraph.rendererContext.indices.clear();
	
	Mesh teapotMesh;
	GameObject teapot;
	teapot.mesh = &teapotMesh;
	teapotMesh.loadMesh("src/teapot.txt");
	teapot.addToBufferNoIndex(seraph.rendererContext.vertices, seraph.rendererContext.indices);
	//Renderer::updateBuffer(seraph.rendererContext);

	//seraph.rendererContext.vertices.clear();
	//seraph.rendererContext.indices.clear();
	Mesh floorMesh;
	GameObject floor;
	floor.mesh = &floorMesh;
	floorMesh.m_vertexCount = 121; // 10 by 10 grid needs 11 by 11 points
	floorMesh.m_vertexData = new Vertex[floorMesh.m_vertexCount];
	floorMesh.m_indicesCount = 100*2*3 * 2;
	floorMesh.m_indices = new uint16_t[floorMesh.m_indicesCount];

	for (int i = 0; i < 11; i++) {
		for (int k = 0; k < 11; k++) {
			floorMesh.m_vertexData[i*11+k] = Vertex({ {  (i - 5.5f)*10,  10, (k - 5.5f) *10}, {1.0f, 1.0f, 1.0f, 1.0f} });
		}
	}
	//seraph.rendererContext.vertices = { { {0,  1.0f},   {0.5f, 0.0f, 0.0f, 0.5f} },
	//								    { {0.5f, -0.5f},  {0.0f, 0.5f, 0.0f, 0.5f} },
	// 							        { {-0.5f, -0.5f}, {0.0f, 0.0f, 0.5f, 0.5f} } };

	std::vector<uint16_t> floorIndexData;

	for (int i = 0; i < 11; i++) {
		for (int k = 0; k < 11; k++) {
			if (k != 10 && i != 10) {
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k)] = i * 11 + k;
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 1] = i * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 2] = (i + 1) * 11 + k;
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 3] = i * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 4] = (i + 1) * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 5] = (i + 1) * 11 + k;

				// same points just anit clockise for reverse viewing.
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 6] = i * 11 + k;
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 2 + 6] = i * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 1 + 6] = (i + 1) * 11 + k;
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 3 + 6] = i * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 5 + 6] = (i + 1) * 11 + (k + 1);
				floorMesh.m_indices[2 * 2 * 3 * (i * 10 + k) + 4 + 6] = (i + 1) * 11 + k;
			}
		}
	}

	floor.addToBuffer(seraph.rendererContext.vertices, seraph.rendererContext.indices);
	Renderer::updateBuffer(seraph.rendererContext);

	// = { 0, 1, 3, 0, 3, 2, 0, 2, 6, 0, 6, 4, 2, 3, 7, 6, 2, 7, 6, 7, 5, 4, 6, 5, 3, 1, 5, 3, 5, 7, 1, 0, 4, 1, 4, 5 };

	// draw initial graphics

	float tempVarForVertex = 0;
	int shift = 1;
	float sum = 0;
	double speed = 2;

	float elapsedAngleX = 0;
	float elapsedAngleY = 0;

	int cursorMode = 0; // 0 for normal (disabled, 1 for hidden)

	double lastFrameTime = glfwGetTime();

	double lastX = 0, lastY = 0;
	glfwGetCursorPos(seraph.rendererContext.window, &lastX, &lastY);

	if (glfwRawMouseMotionSupported()) glfwSetInputMode(seraph.rendererContext.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetInputMode(seraph.rendererContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	double elapsedTime = 0;

	int frameCount = 0;

	while (!Renderer::frameClosed(seraph.rendererContext)) {
		//Timer(a, "FIRST");
		glfwPollEvents();
		{
			//Timer(b, "SECOND");
			frameCount++;

			double timeDelta = glfwGetTime() - lastFrameTime;
			lastFrameTime = glfwGetTime();

			elapsedTime += timeDelta;

			if (elapsedTime > 1) {
				//std::cout << frameCount << "\r";
				//frameCount = 0;
				//elapsedTime = 0;
			}

			float aspectRatio = (seraph.rendererContext.windowHeight + 0.0f) / seraph.rendererContext.windowWidth;
			float fovRadians = 1.0f / tanf(seraph.rendererContext.theta * 0.5f / 180.0f * pi);

			clipVertices(seraph.rendererContext.vertices, seraph.rendererContext.indices, fovRadians * aspectRatio, true, -1.0, true, false);
			clipVertices(seraph.rendererContext.vertices, seraph.rendererContext.indices, fovRadians, true, -1.0, false, false);
			clipVertices(seraph.rendererContext.vertices, seraph.rendererContext.indices, fovRadians * aspectRatio, false, 1.0, true, true);
			clipVertices(seraph.rendererContext.vertices, seraph.rendererContext.indices, fovRadians, false, 1.0, false, true);

			if (seraph.rendererContext.vertices.size() > 0 && seraph.rendererContext.indices.size() > 0) {
				Renderer::updateBuffer(seraph.rendererContext);
			}

			Renderer::runFrame(seraph.rendererContext);

			double xDynamicShift = 0, yDynamicShift = 0, zDynamicShift = 0;
			double xWorldShift = 0, yWorldShift = 0, zWorldShift = 0;

			int state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_W);
			if (state == GLFW_PRESS) {
				zWorldShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_A);
			if (state == GLFW_PRESS) {
				xWorldShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_S);
			if (state == GLFW_PRESS) {
				zWorldShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_D);
			if (state == GLFW_PRESS) {
				xWorldShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_SPACE);
			if (state == GLFW_PRESS) {
				yWorldShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_LEFT_CONTROL);
			if (state == GLFW_PRESS) {
				yWorldShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_LEFT_SHIFT);
			if (state == GLFW_PRESS) {
				xWorldShift *= 10;
				yWorldShift *= 10;
				zWorldShift *= 10;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_LEFT_ALT);
			if (state == GLFW_PRESS) {
				if (0 == cursorMode) {
					glfwSetInputMode(seraph.rendererContext.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
					cursorMode = 1;
				}
				else {
					glfwSetInputMode(seraph.rendererContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					cursorMode = 0;
				}
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_UP);
			if (state == GLFW_PRESS) {
				zDynamicShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_LEFT);
			if (state == GLFW_PRESS) {
				xDynamicShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_DOWN);
			if (state == GLFW_PRESS) {
				zDynamicShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_RIGHT);
			if (state == GLFW_PRESS) {
				xDynamicShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_KP_8);
			if (state == GLFW_PRESS) {
				yDynamicShift += speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_KP_2);
			if (state == GLFW_PRESS) {
				yDynamicShift -= speed * timeDelta;
			}
			state = glfwGetKey(seraph.rendererContext.window, GLFW_KEY_RIGHT_SHIFT);
			if (state == GLFW_PRESS) {
				xDynamicShift *= 10;
				yDynamicShift *= 10;
				zDynamicShift *= 10;
			}

			double newX;
			double newY;
			glfwGetCursorPos(seraph.rendererContext.window, &newX, &newY);

			double diffX = newX - lastX;
			double diffY = (newY - lastY);

			double spinAlongY = -diffX / seraph.rendererContext.windowWidth * 2 * 2 * pi;
			double spinAlongX = diffY / seraph.rendererContext.windowHeight * 2 * 2 * pi;

			if (spinAlongX + elapsedAngleX >= pi / 4) {
				//spinAlongX = pi / 4 - elapsedAngleX;
			}
			if (spinAlongX + elapsedAngleX <= -pi / 4) {
				//spinAlongX = -pi / 4 - elapsedAngleX;
			}

			elapsedAngleX += spinAlongX;
			elapsedAngleY += spinAlongY;

			/////////////////// percentage of screen travelled              x2 for speed, x2pi for radian.

			// rotation around the y-axis (looking around)

			//teapot.shiftNRot(xShift, yShift, zShift, spinAlongY, spinAlongX);

			bool found = false;
			int indexFound = 0;

			int gravity = 10;

			Vec3 p0 { 0, -1, 0 };
			Vec3 u = Vec3{0,0,0} - p0;

			while (!found && indexFound < seraph.rendererContext.indices.size()) {

				Vertex p1 = seraph.rendererContext.vertices[seraph.rendererContext.indices[indexFound]];
				Vertex p2 = seraph.rendererContext.vertices[seraph.rendererContext.indices[indexFound+1]];
				Vertex p3 = seraph.rendererContext.vertices[seraph.rendererContext.indices[indexFound+2]];

				Vec3 v1to2 = p2.pos - p1.pos;
				Vec3 v1to3 = p3.pos - p1.pos;

				Vec3 normal = Vec3::cross(v1to2, v1to3);

				float dot = Vec3::dot(normal, u);

				if (abs(dot) > 1e-6) {

					Vec3 w = p0 - p1.pos;
					float fac = -Vec3::dot(normal, w) / dot;
					u = fac * u;
					Vec3 out = p0 + u;
					if (out.abs() < 2) {
						found = true;
					}
				} 

				indexFound += 3;
			}

			teapot.shiftNRot(xWorldShift+xDynamicShift, yWorldShift + yDynamicShift, zWorldShift + zDynamicShift, spinAlongY, spinAlongX);
			floor.shiftNRot(xWorldShift, yWorldShift, zWorldShift, spinAlongY, spinAlongX);

			seraph.rendererContext.indices.clear();
			seraph.rendererContext.vertices.clear();
			teapot.addToBufferNoIndex(seraph.rendererContext.vertices, seraph.rendererContext.indices);
			floor.addToBufferNoIndex(seraph.rendererContext.vertices, seraph.rendererContext.indices);

			for (int index = 0; index < seraph.rendererContext.indices.size(); index+=3) {

				int v1Pos = seraph.rendererContext.indices[index];
				int v2Pos = seraph.rendererContext.indices[index+1];
				int v3Pos = seraph.rendererContext.indices[index+2];

				Vec3 v1 = seraph.rendererContext.vertices[v1Pos].pos;
				Vec3 v2 = seraph.rendererContext.vertices[v2Pos].pos;
				Vec3 v3 = seraph.rendererContext.vertices[v3Pos].pos;

				Vec3 sum = v1 + v2 + v3;

				Vec3 v1to2 = v2 - v1;
				Vec3 v1to3 = v3 - v1;

				Vec3 normal = Vec3::cross(v1to2, v1to3);
				normal = normal / normal.abs();
				Vec3 other = (1.0 / 3.0) * sum;

				float brightness = Vec3::dot(normal, other)/ (normal.abs() * other.abs());

				brightness = asin(brightness) / pi * 180;

				if (brightness > 90) {
					brightness = 0;
				}

				//brightness = 1;
				brightness /= 90;

				//brightness = -log(1 / brightness - 1);

				//brightness *= abs(brightness);

				if (abs(normal.z) > abs(normal.x) && abs(normal.z) > abs(normal.y)) {
					seraph.rendererContext.vertices[v1Pos].color[0] = 0;
					seraph.rendererContext.vertices[v1Pos].color[1] = 0;
					seraph.rendererContext.vertices[v1Pos].color[2] = brightness;

					seraph.rendererContext.vertices[v2Pos].color[0] = 0;
					seraph.rendererContext.vertices[v2Pos].color[1] = 0;
					seraph.rendererContext.vertices[v2Pos].color[2] = brightness;

					seraph.rendererContext.vertices[v3Pos].color[0] = 0;
					seraph.rendererContext.vertices[v3Pos].color[1] = 0;
					seraph.rendererContext.vertices[v3Pos].color[2] = brightness;

				}  if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z)) {
					seraph.rendererContext.vertices[v1Pos].color[0] = 0;
					seraph.rendererContext.vertices[v1Pos].color[1] = brightness;
					seraph.rendererContext.vertices[v1Pos].color[2] = 0;

					seraph.rendererContext.vertices[v2Pos].color[0] = 0;
					seraph.rendererContext.vertices[v2Pos].color[1] = brightness;
					seraph.rendererContext.vertices[v2Pos].color[2] = 0;

					seraph.rendererContext.vertices[v3Pos].color[0] = 0;
					seraph.rendererContext.vertices[v3Pos].color[1] = brightness;
					seraph.rendererContext.vertices[v3Pos].color[2] = 0;

				} if (abs(normal.x) > abs(normal.y) && abs(normal.x) > abs(normal.z)) {
					seraph.rendererContext.vertices[v1Pos].color[0] = brightness;
					seraph.rendererContext.vertices[v1Pos].color[1] = 0;
					seraph.rendererContext.vertices[v1Pos].color[2] = 0;

					seraph.rendererContext.vertices[v2Pos].color[0] = brightness;
					seraph.rendererContext.vertices[v2Pos].color[1] = 0;
					seraph.rendererContext.vertices[v2Pos].color[2] = 0;

					seraph.rendererContext.vertices[v3Pos].color[0] = brightness;
					seraph.rendererContext.vertices[v3Pos].color[1] = 0;
					seraph.rendererContext.vertices[v3Pos].color[2] = 0;
				}
				seraph.rendererContext.vertices[v1Pos].color[0] = brightness;
				seraph.rendererContext.vertices[v1Pos].color[1] = brightness;
				seraph.rendererContext.vertices[v1Pos].color[2] = brightness;

				seraph.rendererContext.vertices[v2Pos].color[0] = brightness;
				seraph.rendererContext.vertices[v2Pos].color[1] = brightness;
				seraph.rendererContext.vertices[v2Pos].color[2] = brightness;

				seraph.rendererContext.vertices[v3Pos].color[0] = brightness;
				seraph.rendererContext.vertices[v3Pos].color[1] = brightness;
				seraph.rendererContext.vertices[v3Pos].color[2] = brightness;
			}

			lastX = newX;
			lastY = newY;
		}
	}

	std::cout << elapsedTime / frameCount << "time per frame";

	//_ renContext.m_vertices[0].pos[0] = tempVarForVertex;
	tempVarForVertex += shift * 0.001;
	if (tempVarForVertex > 0.75) { shift *= -1; }
	if (tempVarForVertex < -0.75) { shift *= -1; }

	Renderer::waitForDeviceIdle(seraph.rendererContext);

	return 0;
}

void clipVertices(std::vector<Vertex>& verts, std::vector<uint16_t>& indices, float scaleLeft, bool compareLessThan, float scaleRight, bool xUsed, bool flip) {

	std::vector<Vertex> vecData;
	std::vector<uint16_t> indexData;

	int oobCount = 0; // out of bounds count
	// compare left x
	for (int index = 0; index < indices.size(); index += 3) {
		bool first = false, second = false, third = false;
		
		if (xUsed) {
			if (compareLessThan) {
				first = verts[indices[index]].pos.x * scaleLeft < scaleRight * verts[indices[index]].pos.z;
				second = verts[indices[index + 1]].pos.x * scaleLeft < scaleRight * verts[indices[index + 1]].pos.z;
				third = verts[indices[index + 2]].pos.x * scaleLeft < scaleRight * verts[indices[index + 2]].pos.z;
			}
			else {
				first = verts[indices[index]].pos.x * scaleLeft > scaleRight * verts[indices[index]].pos.z;
				second = verts[indices[index + 1]].pos.x * scaleLeft > scaleRight * verts[indices[index + 1]].pos.z;
				third = verts[indices[index + 2]].pos.x * scaleLeft > scaleRight * verts[indices[index + 2]].pos.z;
			}
		} else {
			if (compareLessThan) {
				first = verts[indices[index]].pos.y * scaleLeft < scaleRight * verts[indices[index]].pos.z;
				second = verts[indices[index + 1]].pos.y * scaleLeft < scaleRight * verts[indices[index + 1]].pos.z;
				third = verts[indices[index + 2]].pos.y * scaleLeft < scaleRight * verts[indices[index + 2]].pos.z;
			}
			else {
				first = verts[indices[index]].pos.y * scaleLeft > scaleRight * verts[indices[index]].pos.z;
				second = verts[indices[index + 1]].pos.y * scaleLeft > scaleRight * verts[indices[index + 1]].pos.z;
				third = verts[indices[index + 2]].pos.y * scaleLeft > scaleRight * verts[indices[index + 2]].pos.z;
			}
		}

		oobCount = 0;

		if (first) {
			oobCount++;
		}
		if (second) {
			oobCount++;
		}
		if (third) {
			oobCount++;
		}

		int size = vecData.size();

		//std::cout << oobCount << " " << first << second << third <<  "\r";

		Vertex p1 = verts[indices[index]];
		Vec3 v1 = verts[indices[index]].pos;
		Vertex p2 = verts[indices[index + 1]];
		Vec3 v2 = verts[indices[index + 1]].pos;
		Vertex p3 = verts[indices[index + 2]];
		Vec3 v3 = verts[indices[index + 2]].pos;

		Vec3 normal;
		if (!flip) {
			if (xUsed) {
				normal = { scaleLeft, 0, 1 };
			}
			else {
				normal = { 0, scaleLeft, 1 };
			}
		} else {
			if (xUsed) {
				normal = { -scaleLeft, 0, 1 };
			}
			else {
				normal = { 0, -scaleLeft, 1 };
			}
		}

		switch (oobCount) {
		case 0:
			vecData.push_back(p1);
			vecData.push_back(p2);
			vecData.push_back(p3);
			indexData.push_back(size);
			indexData.push_back(size + 1);
			indexData.push_back(size + 2);
			break;
		case 1:
			if (first) {
				Vec3 V1to2 = v2 - v1;
				Vec3 V1to3 = v3 - v1;

				float dot1to2 = Vec3::dot(V1to2, normal);
				float dot1to3 = Vec3::dot(V1to3, normal);

				Vec3 w = v1 - Vec3{ 0, 0, 0 };
				float fac1to2 = -Vec3::dot(normal, w) / dot1to2;
				float fac1to3 = -Vec3::dot(normal, w) / dot1to3;

				V1to2 = fac1to2 * V1to2;
				V1to3 = fac1to3 * V1to3;

				Vec3 poi1n2 = v1 + V1to2;
				Vec3 poi1n3 = v1 + V1to3;

				Vertex V4 = p1;
				Vertex V5 = p1;

				V4.pos = poi1n2;
				V5.pos = poi1n3;

				vecData.push_back(V4);
				vecData.push_back(V5);
				vecData.push_back(verts[ indices[index + 1]]);
				vecData.push_back(verts[ indices[index + 2]]);
				indexData.push_back(size);
				indexData.push_back(size + 2);
				indexData.push_back(size + 3);
				indexData.push_back(size);
				indexData.push_back(size + 3);
				indexData.push_back(size + 1);
			}
			else if (second) {
				Vec3 V2to1 = v1 - v2;
				Vec3 V2to3 = v3 - v2;

				float dot2to1 = Vec3::dot(V2to1, normal);
				float dot2to3 = Vec3::dot(V2to3, normal);

				Vec3 w = v2 - Vec3{ 0, 0, 0 };
				float fac2to1 = -Vec3::dot(normal, w) / dot2to1;
				float fac2to3 = -Vec3::dot(normal, w) / dot2to3;

				V2to1 = fac2to1 * V2to1;
				V2to3 = fac2to3 * V2to3;

				Vec3 poi2n1 = v2 + V2to1;
				Vec3 poi2n3 = v2 + V2to3;

				Vertex V4 = p2;
				Vertex V5 = p2;

				V4.pos = poi2n1;
				V5.pos = poi2n3;

				vecData.push_back(verts[indices[index]]);
				vecData.push_back(V4);
				vecData.push_back(V5);
				vecData.push_back(verts[indices[index + 2]]);
				indexData.push_back(size);
				indexData.push_back(size + 1);
				indexData.push_back(size + 2);
				indexData.push_back(size);
				indexData.push_back(size + 2);
				indexData.push_back(size + 3);
			}
			else if (third) {

				Vec3 V3to1 = v1 - v3;
				Vec3 V3to2 = v2 - v3;

				float dot3to1 = Vec3::dot(V3to1, normal);
				float dot3to2 = Vec3::dot(V3to2, normal);

				Vec3 w = v3 - Vec3{ 0, 0, 0 };
				float fac3to1 = -Vec3::dot(normal, w) / dot3to1;
				float fac3to2 = -Vec3::dot(normal, w) / dot3to2;

				V3to1 = fac3to1 * V3to1;
				V3to2 = fac3to2 * V3to2;

				Vec3 poi3n1 = v3 + V3to1;
				Vec3 poi3n2 = v3 + V3to2;

				Vertex V4 = p3;
				Vertex V5 = p3;

				V4.pos = poi3n1;
				V5.pos = poi3n2;

				vecData.push_back(verts[indices[index]]);
				vecData.push_back(verts[indices[index + 1]]);
				vecData.push_back(V4);
				vecData.push_back(V5);
				indexData.push_back(size);
				indexData.push_back(size + 1);
				indexData.push_back(size + 3);
				indexData.push_back(size);
				indexData.push_back(size + 3);
				indexData.push_back(size + 2);
			}
			break;
		case 2:
			if (first && second) {
				Vec3 V3to1 = v1 - v3;
				Vec3 V3to2 = v2 - v3;

				float dot3to1 = Vec3::dot(V3to1, normal);
				float dot3to2 = Vec3::dot(V3to2, normal);

				Vec3 w = v3 - Vec3{ 0, 0, 0 };
				float fac3to1 = -Vec3::dot(normal, w) / dot3to1;
				float fac3to2 = -Vec3::dot(normal, w) / dot3to2;

				V3to1 = fac3to1 * V3to1;
				V3to2 = fac3to2 * V3to2;

				Vec3 poi3n1 = v3 + V3to1;
				Vec3 poi3n2 = v3 + V3to2;

				Vertex V4=p3;
				Vertex V5=p3;

				V4.pos = poi3n1;
				V5.pos = poi3n2;

				vecData.push_back(V4);
				vecData.push_back(V5);
				vecData.push_back(verts[indices[index + 2]]);
				indexData.push_back(size);
				indexData.push_back(size + 1);
				indexData.push_back(size + 2);

			}
			else if (first && third) {
				Vec3 V2to1 = v1 - v2;
				Vec3 V2to3 = v3 - v2;

				float dot2to1 = Vec3::dot(V2to1, normal);
				float dot2to3 = Vec3::dot(V2to3, normal);

				Vec3 w = v2 - Vec3{ 0, 0, 0 };
				float fac2to1 = -Vec3::dot(normal, w) / dot2to1;
				float fac2to3 = -Vec3::dot(normal, w) / dot2to3;

				V2to1 = fac2to1 * V2to1;
				V2to3 = fac2to3 * V2to3;

				Vec3 poi2n1 = v2 + V2to1;
				Vec3 poi2n3 = v2 + V2to3;

				Vertex V4 = p2;
				Vertex V5 = p2;

				V4.pos = poi2n1;
				V5.pos = poi2n3;

				vecData.push_back(V4);
				vecData.push_back(verts[indices[index + 1]]);
				vecData.push_back(V5);
				indexData.push_back(size);
				indexData.push_back(size + 1);
				indexData.push_back(size + 2);
			}
			else if (second && third) {
				Vec3 V1to2 = v2 - v1;
				Vec3 V1to3 = v3 - v1;

				float dot1to2 = Vec3::dot(V1to2, normal);
				float dot1to3 = Vec3::dot(V1to3, normal);

				Vec3 w = v1 - Vec3{ 0, 0, 0 };
				float fac1to2 = -Vec3::dot(normal, w) / dot1to2;
				float fac1to3 = -Vec3::dot(normal, w) / dot1to3;

				V1to2 = fac1to2 * V1to2;
				V1to3 = fac1to3 * V1to3;

				Vec3 poi1n2 = v1 + V1to2;
				Vec3 poi1n3 = v1 + V1to3;

				Vertex V4 = p1;
				Vertex V5 = p1;

				V4.pos = poi1n2;
				V5.pos = poi1n3;

				vecData.push_back(verts[indices[index]]);
				vecData.push_back(V4);
				vecData.push_back(V5);
				indexData.push_back(size);
				indexData.push_back(size + 1);
				indexData.push_back(size + 2);
			}
		case 3:
			break;
		default:
			break;
		}
	}

	 verts.clear();
	 indices.clear();

	 verts = std::move(vecData);
	 indices = std::move(indexData);
}
