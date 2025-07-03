#pragma once

#include "Vertex.h"
#include <fstream>
#include <vector>

class Entity;

class WorldNodeTree {
	struct WorldNode {
	private:
		WorldNode* m_left = nullptr;
		WorldNode* m_right = nullptr;

		Entity* m_entity;

		int height() {
			if (nullptr == m_left && nullptr == m_right) {
				return 0;
			}else if (nullptr == m_left) {
				return m_right->height() + 1;
			} else if (nullptr == m_right) {
				return m_left ->height() + 1;
			} else {
				int left = m_left->height();
				int right = m_right->height();
				if (right > left) {
					return right  +1;
				} else {
					return left + 1;
				}
			}

		}
	};

	WorldNodeTree() {}

	void add(WorldNode& someNode) {

	}
};

// Class world is a tree that manages the location of all world datablocks
class World {
private:
	
};

struct Mesh {

	Vertex* m_vertexData = nullptr;
	uint32_t m_vertexCount = 0;

	uint16_t* m_indices = nullptr;
	uint32_t m_indicesCount = 0;

	Mesh() {
	}

	~Mesh() {
		if (nullptr != m_vertexData) {
			delete[] m_vertexData;
		}
		if (nullptr != m_indices) {
			delete[] m_indices;
		}

	}

	void loadMesh(std::string filename) {
		std::ifstream inputFile(filename);

		if (!inputFile.is_open()) {
			throwError("Cannot open file: " + filename, logLevelError);
		}

		std::string line;

		std::vector<Vertex> verticesLoaded;
		std::vector<uint16_t> indicesLoaded;

		while (std::getline(inputFile, line)) {
			if ('v' == line[0]) {
				line = line.substr(line.find(' ') + 1);
				float x = std::stof(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				float y = -std::stof(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				float z = std::stof(line);
				float dist = sqrt(x * x + y * y + z * z);
				verticesLoaded.push_back({ { x,  y, z }, {1.0f, 1.0f, 1.0f, 1.0f} });
			}
			else if ('f' == line[0]) {
				line = line.substr(line.find(' ') + 1);
				int x = std::stoi(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				int y = std::stoi(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				int z = std::stoi(line);
				indicesLoaded.push_back(x - 1);
				indicesLoaded.push_back(y - 1);
				indicesLoaded.push_back(z - 1);
			}
		}

		inputFile.close();

		m_vertexCount = verticesLoaded.size();
		m_indicesCount = indicesLoaded.size();

		if (nullptr != m_vertexData) {
			delete[] m_vertexData;
		}
		if (nullptr != m_indices) {
			delete[] m_indices;
		}

		m_vertexData = new Vertex[m_vertexCount];
		m_indices = new uint16_t[m_indicesCount];

		for (int index = 0; index < m_vertexCount; index++) {
			m_vertexData[index] = verticesLoaded[index];
		}

		for (int index = 0; index < m_indicesCount; index++) {
			m_indices[index] = indicesLoaded[index];
		}

	}

};

struct GameObject {
	Mesh* mesh;

	void shiftMesh(float x, float y, float z) {
		shiftNRot(x, y, z, 0, 0);
	}

	void rot(float spinAlongY, float spinAlongX) {
		shiftNRot(0, 0, 0, spinAlongY, spinAlongX);
	}

	void shiftNRot(float xShift, float yShift, float zShift, float spinAlongY, float spinAlongX) {
		for (int index = 0; index < mesh->m_vertexCount; index++) {
			float x = mesh->m_vertexData[index].pos[0] + xShift;
			float y = mesh->m_vertexData[index].pos[1] + yShift;
			float z = mesh->m_vertexData[index].pos[2] + zShift;

			mesh->m_vertexData[index].pos[0] = x * cosf(spinAlongY) + z * sinf(spinAlongY);
			mesh->m_vertexData[index].pos[1] = y;
			mesh->m_vertexData[index].pos[2] = z * cosf(spinAlongY) - x * sinf(spinAlongY);

			y = mesh->m_vertexData[index].pos[1];
			z = mesh->m_vertexData[index].pos[2];

			mesh->m_vertexData[index].pos[1] = y * cosf(spinAlongX) - z * sinf(spinAlongX);
			mesh->m_vertexData[index].pos[2] = z * cosf(spinAlongX) + y * sinf(spinAlongX);
		}
	}

	void addToBuffer(std::vector<Vertex>& vertex, std::vector<uint16_t>& indices) {
		int16_t currentBufferAmount = vertex.size();
		for (int index = 0; index < mesh->m_indicesCount; index++) {
			indices.push_back(mesh->m_indices[index] + currentBufferAmount);
		}

		for (int index = 0; index < mesh->m_vertexCount; index++) {
			vertex.push_back(mesh->m_vertexData[index]);
		}

	}

	void addToBufferNoIndex(std::vector<Vertex>& vertex, std::vector<uint16_t>& indices) {
		int16_t currentBufferAmount = vertex.size();
		for (int index = 0; index < mesh->m_indicesCount; index++) {
			vertex.push_back(mesh->m_vertexData[mesh->m_indices[index]]);
			indices.push_back(currentBufferAmount + index);
		}
	}
};

