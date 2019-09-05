#include "tools.h"


void tools::LogMessage(std::string message, bool printTime)
{
	std::string result = "";

	if (printTime) {
		time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		result += tools::TimeToStr(&time, "[%Y-%m-%d %H:%M:%S] ");
	}

	result += message + '\n';
	std::cout << result;

	try {
		std::string filePath = tools::DirProg() + "..\\" + LOG_FILE;

		std::fstream fs;
		fs.exceptions(std::ios::failbit | std::ios::badbit);
		fs.open(filePath.c_str(), std::ios_base::app);

		fs << result;
		fs.close();
	}
	catch (const std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}
}

std::string tools::TimeToStr(std::time_t * time, const char * format)
{
	tm timeInfo;
	localtime_s(&timeInfo, time);
	char strTime[32];
	strftime(strTime, 32, format, &timeInfo);
	return strTime;
}

std::string tools::DirProg()
{
	char path[MAX_PATH] = {};
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpecA(path);
	PathAddBackslashA(path);
	return std::string(path);
}

bool tools::LoadBytesFromFile(const std::string &path, char** pData, size_t * size)
{
	// Открытие файла в режиме бинарного чтения
	std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

	// Если файл открыт
	if (is.is_open())
	{
		*size = (size_t)is.tellg(); // Получить размер
		is.seekg(0, std::ios::beg); // Перейти в начало файла
		*pData = new char[*size];   // Аллокировать необходимое кол-во памяти
		is.read(*pData, *size);     // Прочесть файл и поместить данные в буфер
		is.close();                 // Закрыть файл

		return true;
	}

	return false;
}

void tools::LogError(std::string message, bool printTime)
{
	std::string newMessage = "ERROR! ";
	newMessage += message;
	LogMessage(message, printTime);
}

