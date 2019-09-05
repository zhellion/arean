#pragma once
#ifndef LOADOBJFILE
#define LOADOBJFILE


#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "mainRenderer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



struct Texture
{
	std::string patch;
	std::string type;
};

class LoadOBJ {
public:
	/*  Mesh Data  */

	std::vector<VKStr::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	
	/*  Functions  */
	LoadOBJ(std::string pth)
	{
		pth_ = pth;
		loadModel(pth);
	};

private:
	std::string pth_;
	std::string directory_;
	void loadModel(std::string path)
	{
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		directory_ = path.substr(0, path.find_last_of('\\'));
		processNode(scene->mRootNode, scene);
	};
	void processNode(aiNode *node, const aiScene *scene) {
		// обработать все полигональные сетки в узле(если есть)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			processMesh(mesh, scene);
		}
		// выполнить ту же обработку и для каждого потомка узла
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	};
	void processMesh(aiMesh *mesh, const aiScene *scene)
	{
		
	
	float maxlen = 0.0;
		size_t inc = mesh->mNumVertices;

	/*	for (unsigned int i = 0; i < inc; i++)
		{
			float len = mesh->mVertices[i].x;
			if (maxlen < len)
			{
				maxlen = len;
			}
			len = mesh->mVertices[i].y;
			if (maxlen < len)
			{
				maxlen = len;
			}
			len = mesh->mVertices[i].z;
			if (maxlen < len)
			{
				maxlen = len;
			}
		}*/

		for (unsigned int i = 0; i < inc; i++)
		{
			VKStr::Vertex vertex;
			// обработка координат, нормалей и текстурных координат вершин
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x; // maxlen;
			vector.y = mesh->mVertices[i].y; // maxlen;
			vector.z = mesh->mVertices[i].z; // maxlen;
			//std::cout << vector.x <<' '<< vector.y<< ' ' << vector.z << std::endl;
			vertex.position = vector;


			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			//std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << std::endl;
			vertex.normals = vector; //тут должны быть нормали, но поф, потом
			vertex.color = { 0.0f, 0.0f, 0.0f };
			if (mesh->mTextureCoords[0]) // сетка обладает набором текстурных координат?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				//std::cout << mesh->mTextureCoords[0][i].x << ' ' << mesh->mTextureCoords[0][i].y << std::endl;
				vertex.texCoord = vec;
			}
			else
				vertex.texCoord = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}


		//std::cout << maxlen << std::endl;
		// орбаботка индексов
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// обработка материала
		if (mesh->mMaterialIndex >= 0)
		{
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
					aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				/*std::vector<VKStr::Texture> specularMaps = loadMaterialTextures(material,
					aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());*/
			
		}

	};
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures_;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
				Texture texture;
				texture.patch =  directory_+'\\'+ str.C_Str();
				
				texture.type = typeName;
				
				textures_.push_back(texture);
				// занесем текстуру в список уже загруженных
				textures_.push_back(texture);
				
		}
		return textures_;
	};


};

#endif
