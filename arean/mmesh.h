#pragma once
#ifndef MMESH_H
#define MMESH_H
#include "pch.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
//#include "sheider1.h"
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>


//структура данных загружаемых вершин
struct Vertex {
	glm::vec3 Position; //координаты
	glm::vec3 Normal; //нормаль
	glm::vec2 TexCoords; //текстурные координаты

						 // = [0.2f, 0.4f, 0.6f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f];
};
//структура текстурных данныз
struct Texture {
	unsigned int id; //айдишник
	std::string type; //тип диффузная или бликовая
	std::string path;  // храним путь к текстуре для нужд сравнения объектов текстур
};

class Mesh {
public:
	/*  Mesh Data  */
	
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	/*  Functions  */
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	};
/*	void DrawG(ShaderPlusGeom shader)
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		//std::cout << textures.size() << std::endl;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // активируем текстурный блок, до привязки
											  // получаем номер текстуры
			std::stringstream ss; 


			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
				(ss << diffuseNr++); // передаем unsigned int в stream
			else if (name == "texture_specular")
				(ss << specularNr++); // передаем unsigned int в stream
			number = ss.str();

			shader.setFloat(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);

		// отрисовывем полигональную сетку
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	};*/
	void Draw(Shader shader)
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		//std::cout << textures.size() << std::endl;
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i); // активируем текстурный блок, до привязки
											  // получаем номер текстуры
			std::stringstream ss;


			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
				(ss << diffuseNr++); // передаем unsigned int в stream
			else if (name == "texture_specular")
				(ss << specularNr++); // передаем unsigned int в stream
			number = ss.str();

			shader.setFloat(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);

		// отрисовывем полигональную сетку
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	};
private:
	/*  Render data  */
	unsigned int VAO, VBO, EBO;
	/*  Functions    */
	void setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
			&indices[0], GL_STATIC_DRAW);

		// vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		//мы можем применять sizeof к целой структуре, будет возвращена суммарная длина всех значений
		//offsetof
		//1 аргумент структра
		//2 аргумент имя переменной структуры
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glBindVertexArray(0);
	};
	
};


#endif

