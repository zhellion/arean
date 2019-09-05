#pragma once

#ifndef TOOLS_H
#define TOOLS_H




#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <vulkan\vulkan.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <ctime>
#include <time.h>
#include <chrono>


#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#ifndef LOG_FILE
#define LOG_FILE "log.txt"
#endif
namespace tools{


void LogMessage(std::string message, bool printTime = true);
std::string TimeToStr(std::time_t * time, const char * format);
std::string DirProg();
bool LoadBytesFromFile(const std::string &path, char** pData, size_t * size);
void LogError(std::string message, bool printTime = true);

}



#endif
