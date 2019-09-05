#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <string>
#include "mmesh.h"
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	/*  Методы   */
	Model(const char *path)
	{
		loadModel(path);
	}
private:
	/*  Данные модели  */
	std::vector<Texture> textures_loaded;
	std::vector<Mesh> meshes;
	std::string directory;
	/*  Методы   */
	void loadModel(std::string path)
	{
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	};
	void processNode(aiNode *node, const aiScene *scene) {
		// обработать все полигональные сетки в узле(если есть)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// выполнить ту же обработку и для каждого потомка узла
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	};
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		//std::cout << mesh->mNumVertices << std::endl;
		GLfloat maxlen = 0.0;
		size_t inc = mesh->mNumVertices;

		for (unsigned int i = 0; i < inc; i++)
		{
			GLfloat len = mesh->mVertices[i].x;
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
		}

		for (unsigned int i = 0; i < inc; i++)
		{
			Vertex vertex;
			// обработка координат, нормалей и текстурных координат вершин
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x/maxlen;
			vector.y = mesh->mVertices[i].y/maxlen;
			vector.z = mesh->mVertices[i].z/maxlen;
			//std::cout << vector.x <<' '<< vector.y<< ' ' << vector.z << std::endl;
			vertex.Position = vector;
			
			
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			//std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << std::endl;
			vertex.Normal = vector;
			if (mesh->mTextureCoords[0]) // сетка обладает набором текстурных координат?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				//std::cout << mesh->mTextureCoords[0][i].x << ' ' << mesh->mTextureCoords[0][i].y << std::endl;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
				vertices.push_back(vertex);
		}

		
		std::cout << maxlen << std::endl;
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
				if (mesh->mMaterialIndex >= 0)
				{
					aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
					std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
						aiTextureType_DIFFUSE, "texture_diffuse");
					textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
					std::vector<Texture> specularMaps = loadMaterialTextures(material,
						aiTextureType_SPECULAR, "texture_specular");
					textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
				}
			}

		return Mesh(vertices, indices, textures);
	};
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{   // если текстура не была загружена – сделаем это
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				// занесем текстуру в список уже загруженных
				textures_loaded.push_back(texture);
			}
		}
		return textures;
	};
	GLint TextureFromFile(const char* path, std::string directory) 
	{ //Generate texture ID and load texture data 
		std::string filename = std::string(path); 
		filename = directory + '/' + filename;
		
		std::cout << filename << std::endl;
		GLuint textureID; 
		glGenTextures(1, &textureID); 

		
		int width, height;

		unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB); 
		//std::cout << image << std::endl;
		// Assign texture to ID 
		glBindTexture(GL_TEXTURE_2D, textureID); 
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D); 
		// Parameters 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glBindTexture(GL_TEXTURE_2D, 0); 
		SOIL_free_image_data(image); 
		return textureID;
	};
};
#endif