#include "arean.h"
#include <qtimer.h>

#include <iostream>
#include <Windows.h>
#include <iomanip>
#include <math.h>

#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

#include "tools.h"
#include "mainRenderer.h"
#include "loadOBJFile.h"
#include <qopenglwidget.h>
#include <QCoreApplication>
#include <qkeyevent.h>
#include <QMouseEvent>
#include <string>
#include <QFloat16>
#include <math.h>
#include <thread>



using std::chrono::time_point;
using std::chrono::high_resolution_clock;

// ���������� ������� ��������� ��������� ���� ��� ��������� ���������.
LRESULT CALLBACK MainWindowProcedure(HWND, UINT, WPARAM, LPARAM);

// �������� ��������
VKStr::Texture LoadTextureVk(MainRenderer * renderer, std::string path);

// ����� ��������� ����
HWND hMainWindow = nullptr;

// ��������� �� ��������
MainRenderer * mainRenderer = nullptr;

// ����� ���������� ����� (������ ��������� ��������)
time_point<high_resolution_clock> lastFrameTime;
//������ �� ����
bool ready = false;

double deltaMs;
//��� ����, ����� ������� ���� 1 �����. ����� ��� ������� ����� ���� �����. 
bool threadEnd = true;


// ��������� ����������� ������ ������ (���������� � ����� ������!)
struct {
	// ��������� �� ������ � ������� ������
	glm::vec3 position = glm::vec3(10.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 90.0f, 0.0f);

	// ��������� �������� (�������� �� -1 �� 1)
	glm::i8vec3 movement = glm::i8vec3(0, 0, 0);

	// �������� ����������� � ���������������� ����
	float movementSpeed = 1.0f;
	float mouseSensitivity = 0.5f;

	// �������� �� ������
	bool IsMoving() const {
		return this->movement.x != 0 || this->movement.y != 0 || this->movement.z != 0;
	}

	// ���������� ���������
	// float deltaMs - ����� ������ ����� (�������� �����) �� ������� ����� ��������� �����������
	void UpdatePositions(float deltaMs) {
		if (this->IsMoving()) {

			// ��������� ������ �������� (����� ������� �� ������ ����������)
			glm::vec3 movementVectorLocal = glm::vec3(
				(float)this->movement.x * ((this->movementSpeed / 1000.0f) * deltaMs),
				(float)this->movement.y * ((this->movementSpeed / 1000.0f) * deltaMs),
				(float)this->movement.z * ((this->movementSpeed / 1000.0f) * deltaMs)
			);

			// ���������� ������ �������� (� ������ �������� ������)
			glm::vec3 movementVectorGlobal = glm::vec3(
				(cos(glm::radians(this->rotation.y)) * movementVectorLocal.x) - (sin(glm::radians(this->rotation.y)) * movementVectorLocal.z),
				(cos(glm::radians(this->rotation.x)) * movementVectorLocal.y) + (sin(glm::radians(this->rotation.x)) * movementVectorLocal.z),
				(sin(glm::radians(this->rotation.y)) * movementVectorLocal.x) + (cos(glm::radians(this->rotation.y)) * movementVectorLocal.z)
			);

			// ���������� �������
			this->position += movementVectorGlobal;
		}
	}
} camera;
//---------------------------------
struct {
	double T0 =245; //��������� ����� (��������� ����)
	double T = 245; //������� �����
	double LoAN = 0.178636388; //������� ����������� ���� (Longitude of ascending node)
	double inclination = 0.1; //����������
	double AoP = 0.977839136; //�������� ���������� (argument of periapsis)
	double BSA = 8.661472043; //������� ������� (big semi axis)
	double e = 0.15655003;//�����������
	double M0 = 6.273425038; //������� ��������
	double GGC = 3.9860044; /* glm::pow(10, 14);*///��������������� �������������� ����������
	double n = (std::sqrt(GGC)) / (BSA*std::sqrt(BSA));
	double radiusMod = 1; // ������������� ������ �������
	glm::vec3 oldVec = {}; //������ ��� �����������
	void infoChange()
	{
		n = (std::sqrt(GGC)) / (BSA*std::sqrt(BSA));
		
	}

	glm::vec3 solvek()
	{
		
			double M = M0 + n * (T - T0);
			double E = iteratE(M, 1.0f);
			double tg2V = glm::sqrt((1 + e) / (1 - e))*glm::tan(E / 2);
			double V = glm::atan(tg2V)*2;
			double U = AoP + V;
			double r = radiusMod *BSA * (1 - pow(e,2)) / (1 + e * glm::cos(V));
			oldVec = glm::vec3(
				r*(glm::cos(LoAN)*glm::cos(U) - glm::sin(LoAN)*glm::sin(U)*glm::cos(inclination)),
				r*(glm::sin(LoAN)*glm::cos(U) - glm::cos(LoAN)*glm::sin(U)*glm::cos(inclination)),
				r*glm::sin(U)*glm::sin(inclination)
				);
		
		return oldVec;

	}

	float iteratE(double M, double eps)
	{

		double E = M + e * glm::sin(M);
		double E2 = M + e * glm::sin(E);
		int i = 0;
		while (/*std::abs(E2 - E) < eps*/i<3)//���� �� �������� �� ������� �������� �������
		{
			i++;
			E = E2;
			E2 = M + e * glm::sin(E);
		}
		return E2;

	}
	
} Kepler;

auto arean::vecforform() {
	threadEnd = false;
	glm::vec3 outv = Kepler.solvek();
	ui.label->setText(QString::number(outv.x));
	ui.label_2->setText(QString::number(outv.y));
	ui.label_3->setText(QString::number(outv.z));
	ui.label_4->setText(QString::number(deltaMs));
	ui.label_17->setText(QString::number(Kepler.T));
	threadEnd = true;
}


void arean::showOrbitParam()
{
	ui.lineEdit->setText(QString::number(Kepler.T0));
	ui.lineEdit_2->setText(QString::number(Kepler.LoAN));
	ui.lineEdit_3->setText(QString::number(Kepler.inclination));
	ui.lineEdit_4->setText(QString::number(Kepler.AoP));
	ui.lineEdit_5->setText(QString::number(Kepler.BSA));
	ui.lineEdit_6->setText(QString::number(Kepler.e));
	ui.lineEdit_7->setText(QString::number(Kepler.M0));
	ui.lineEdit_8->setText(QString::number(Kepler.GGC));
}
void arean::setOrbitParam() {

	Kepler.T0 = ui.lineEdit->text().toFloat();
	Kepler.LoAN = ui.lineEdit_2->text().toFloat();
	Kepler.inclination = ui.lineEdit_3->text().toFloat();
	Kepler.AoP = ui.lineEdit_4->text().toFloat();
	Kepler.BSA = ui.lineEdit_5->text().toFloat();
	Kepler.e = ui.lineEdit_6->text().toFloat();
	Kepler.M0 = ui.lineEdit_7->text().toFloat();
	Kepler.GGC = ui.lineEdit_8->text().toFloat();
	void infoChange();
	disableEdits();
	
}

void arean::disableEdits()
{
	ui.lineEdit->setReadOnly(true);
	ui.lineEdit_2->setReadOnly(true);
	ui.lineEdit_3->setReadOnly(true);
	ui.lineEdit_4->setReadOnly(true);
	ui.lineEdit_5->setReadOnly(true);
	ui.lineEdit_6->setReadOnly(true);
	ui.lineEdit_7->setReadOnly(true);
	ui.lineEdit_8->setReadOnly(true);
}

void arean::enableEdit()
{
	ui.lineEdit->setReadOnly(false);
	ui.lineEdit_2->setReadOnly(false);
	ui.lineEdit_3->setReadOnly(false);
	ui.lineEdit_4->setReadOnly(false);
	ui.lineEdit_5->setReadOnly(false);
	ui.lineEdit_6->setReadOnly(false);
	ui.lineEdit_7->setReadOnly(false);
	ui.lineEdit_8->setReadOnly(false);
}


//----------------------------------

// ��������� ������� ���� � ������� ��������� ����
struct {
	int32_t x = 0;
	int32_t y = 0;
	bool lb = false;
	bool rb = false;
	bool cb = false;
} mousePos;


int arean::thread_core()
{
	tools::LogMessage("Start");

	try {
		// ������������� ������ 
		ready = true;
		timer->stop();
		HINSTANCE hInstance = GetModuleHandleA(nullptr);

	/*	// ��������� ����������� ����� �������������� ����� ����
		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);                                           // ������ ���������
		wcex.style = CS_HREDRAW | CS_VREDRAW;                                       // �������������� ��� ��������� �������������� ��� ������������ ��������
		wcex.lpfnWndProc = MainWindowProcedure;                                     // ������� ���������
		wcex.hInstance = hInstance;                                                 // ����� ������
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);                          // ������
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);                   // ����-������
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);                                 // ������
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);                            // ���� ����
		wcex.lpszMenuName = NULL;                                                   // �������� ����
		wcex.lpszClassName = L"VulkanWindowClass";                                  // ������������ ������ (������ wchar_t ��������)

		// ������� ���������������� �����
		if (!RegisterClassEx(&wcex)) {
			throw std::runtime_error("Can't register new windows class (WinWindow)");
		}

		tools::LogMessage("Window class registered sucessfully");

		// �������� ��������� ����
		hMainWindow = CreateWindow(
			wcex.lpszClassName,                                                    // ������������ ������������������� ������ 
			L"trash",															   // ��������� ����
			WS_OVERLAPPEDWINDOW,                                                   // ����� ����
			CW_USEDEFAULT,                                                         // ��������� �� X 
			CW_USEDEFAULT,                                                         // ��������� �� Y
			640,                                                                   // ������ 
			480,                                                                   // ������
			NULL,                                                                  // ��������� �� ����
			NULL,                                                                  // ����� ������������� ����
			hInstance,                                                             // ����� ���������� (instance) ����������
			NULL);           */                                                      // ������������ � ������� ��������� �������� (�����. WM_CREATE)

		hMainWindow = (HWND)this->ui.widget->winId();

		// ���� ����� ���� ��� ��� ���� - ���������� ����������
		if (!hMainWindow) {
			throw std::runtime_error("Can't create new window");
		}

		tools::LogMessage("Window created sucessfully");

		// ����� ����
		ShowWindow(hMainWindow, SW_SHOWNORMAL);

		// ������������� ���������
		mainRenderer = new MainRenderer(hInstance, hMainWindow, 100);

		// �������� �������
		VKStr::Texture cubeTexture = LoadTextureVk(mainRenderer, tools::DirProg() + "..\\Textures\\" + "15ebdab33f70067567ca9603b495f7ba.png");

	/*	mainRenderer->AddPrimitive({
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

		/*	{ { 0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, -0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },*/

	//		}, { 2,1,0, 0,3,2/*, 6,5,4, 4,7,6, 10,9,8, 8,11,10, 14,13,12, 12,15,14, 18,17,16, 16,19,18, 22,21,20, 20,23,22*/ }, &cubeTexture, AREAN_SHADER_T_BASE, { -5.0f, -5.0f, 0.1f }, { 0.0f,0.0f,0.0f }, { 10.0f, 10.0f, 10.0f });

		unsigned int mainLight = mainRenderer->initLight({ 0.0f, 0.0f, 300.0f }, { 1.0f, 1.0f, 1.0f }, 0.01f, 0.5f, 0.7f, 1.0f, 0.0014f, 0.000007f);

	


		LoadOBJ skyBoxinc(tools::DirProg() + "..\\3dmodels\\skybox\\" + "Moon 2K.obj");

		VKStr::Texture skyBoxText = LoadTextureVk(mainRenderer, skyBoxinc.textures[0].patch);
		unsigned int SkyBox = mainRenderer->AddPrimitive(skyBoxinc.vertices, skyBoxinc.indices, &skyBoxText, AREAN_SHADER_T_SKY_BOX, { 0.0, 0.0, -1.0 }, { 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f });

		//LoadOBJ shape(tools::DirProg() + "..\\3dmodels\\" + "111 obj.obj");
		LoadOBJ shape(tools::DirProg() + "..\\3dmodels\\moon\\" + "Moon 2K.obj");

		VKStr::Texture shapetext = LoadTextureVk(mainRenderer, shape.textures[0].patch);

		unsigned int shapeInc = mainRenderer->AddPrimitive(shape.vertices, shape.indices, &shapetext, AREAN_SHADER_T_BASE, { 0.1f, 0.1f, -7.0f }, { 0.0f, 0.0f, 0.0f }, { 0.25, 0.25, 0.25 });


		unsigned int shapeInc1 = mainRenderer->AddPrimitive(shape.vertices, shape.indices, &shapetext, AREAN_SHADER_T_BASE, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });


		// ������������ �����������
		mainRenderer->SetCameraPerspectiveSettings(60.0f, 0.1f, 256.0f);

		// ����� ���������� ����� - ����� ������ �����
		lastFrameTime = high_resolution_clock::now();

		mainRenderer->VideoSettingsChanged();

		
		
		

		// �������� ���� ����������
		while (true)
		{
			// ��������� �������� (����������) ���������
			MSG msg = {};
			
			// ���� �������� �����-�� ��������� �������
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// ����������� (���������� ��������) � ������������� ��������� � ������� ��������� ������ ����
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (!ready)
			{
				break;
			}
			// ���� ����� ���� �� ���� (�� ����� ����� ������ ��� �������� ����)
			if (hMainWindow) {

				// ����� �������� ����� (������� ��������)
				time_point<high_resolution_clock> currentFrameTime = high_resolution_clock::now();

				// ������� ����������� ������ � ��������� ��������
				// 1 ������������ = 1000 ����������� = 1000000 ����������
				int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTime - lastFrameTime).count();

				// ��������� � ������������
				deltaMs = (double)delta / 1000;
				Kepler.T = Kepler.T + deltaMs / 1000;

				// �������� ����� ��������� ��������
				lastFrameTime = currentFrameTime;
				
				// ���������� ����������� ������ � ������ ������� �����
				camera.UpdatePositions(deltaMs);
				if (threadEnd)
				{
					std::thread thr(&arean::vecforform, this);
					thr.detach();
				}
				//vecforform(outv, deltaMs, Kepler.T);
				
				mainRenderer->UpdatePrimitive(SkyBox, { -camera.position.x, -camera.position.y, -camera.position.z }, { 0.0, 0.0, 0.0 }, { 110.0f, 110.0f, 110.0f });
				mainRenderer->UpdatePrimitive(shapeInc, Kepler.solvek(), { 0.0f, 0.0f, 0.0f }, { 0.25, 0.25, 0.25 });

				// �������� �������� � ���������� ����
				mainRenderer->SetCameraPosition(camera.position.x, camera.position.y, camera.position.z);
				mainRenderer->SetCameraRotation(camera.rotation.x, camera.rotation.y, camera.rotation.z);
				mainRenderer->Update();

				mainRenderer->Draw();
			}
		}

		// ���������� ��������
		delete mainRenderer;
	}
	catch (const std::exception &ex) {
		tools::LogError(ex.what());
	}

	// ����� � ����� 0
	while (!threadEnd)
	{

	}
	tools::LogMessage("Application closed successfully\n");
	return 0;
}

// ����������� ������� ��������� ��������� ����
/*LRESULT CALLBACK MainWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_DESTROY:
		// ��� ��������� ��������� �� ����������� ���� (��� ����������� ������ ��� ��������, ���� WM_CLOSE �� ��������������)
		// ������� ����� (��� ��������� ������� � ���������) � ���������� ��������� WM_QUIT (��� ������ PostQuitMessage)
		hMainWindow = nullptr;
		PostQuitMessage(0);
		break;
	case WM_EXITSIZEMOVE:
		// ��� ��������� �������� ���� (����� ����� �������� � ������� ������ ����) //�� �������� ��� ������ ������������� ����� ������ �� ����.
		if (mainRenderer != nullptr) {
			mainRenderer->VideoSettingsChanged();
		}
		break;
	case WM_KEYDOWN:
		// ��� ������� ������ (WASD)
		switch (wParam)
		{
		case 0x57:
			camera.movement.z = 1;
			break;
		case 0x41:
			camera.movement.x = 1;
			break;
		case 0x53:
			camera.movement.z = -1;
			break;
		case 0x44:
			camera.movement.x = -1;
			break;
		}
		break;
	case WM_KEYUP:
		// ��� ������� ������ (WASD)
		switch (wParam)
		{
		case 0x57:
			camera.movement.z = 0;
			break;
		case 0x41:
			camera.movement.x = 0;
			break;
		case 0x53:
			camera.movement.z = 0;
			break;
		case 0x44:
			camera.movement.x = 0;
			break;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		// ��� ������� ����� ������ ����
		mousePos.x = (int32_t)LOWORD(lParam);
		mousePos.y = (int32_t)HIWORD(lParam);
		break;
	case WM_MOUSEMOVE:
		// ��� �������� ����
		// ���� ������ ����� ������ ����
		if (wParam & MK_LBUTTON) {
			int32_t posx = (int32_t)LOWORD(lParam);
			int32_t posy = (int32_t)HIWORD(lParam);
			camera.rotation.x -= (mousePos.y - (float)posy) * camera.mouseSensitivity;
			camera.rotation.y -= (mousePos.x - (float)posx) * camera.mouseSensitivity;
			mousePos.x = posx;
			mousePos.y = posy;
		}
		break;
		//������������� �������� (����� �������� ������!!)
	/*case WM_MOUSEWHEEL:
		int32_t mousewheel = GET_WHEEL_DELTA_WPARAM(wParam);
		camera.movement.y = mousewheel;
		break;*/
/*	default:
		//� ������ ��������� ��������� - �������������� �� � ������� DefWindowProc (������� ��������� �� ���������)

		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}*/
//-----------------------
void arean::resizeEvent(QResizeEvent *e)
{
	ready ? mainRenderer->VideoSettingsChanged():0;
}

void arean::keyPressEvent(QKeyEvent * e)
{
	switch (e->key())
	{
	case 0x57:
		camera.movement.y = -1;
		break;
	case 0x41:
		camera.movement.x = 1;
		break;
	case 0x53:
		camera.movement.y = 1;
		break;
	case 0x44:
		camera.movement.x = -1;
		break;
	}
}

void arean::keyReleaseEvent(QKeyEvent * e)
{
	switch (e->key())
	{
	case 0x57:
		camera.movement.y = 0;
		break;
	case 0x41:
		camera.movement.x = 0;
		break;
	case 0x53:
		camera.movement.y = 0;
		break;
	case 0x44:
		camera.movement.x = 0;
		break;
	}
}

void arean::mousePressEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		mousePos.lb = true;
		mousePos.x = e->x();
		mousePos.y = e->y();
		break;
	case Qt::RightButton:
		mousePos.rb = true;
		break;
	case Qt::MiddleButton:
		mousePos.cb = true;
		break;
	} 
}

void arean::mouseReleaseEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		mousePos.lb = false;
		break;
	case Qt::RightButton:
		mousePos.rb = false;
		break;
	case Qt::MiddleButton:
		mousePos.cb = false;
		break;
	}
}

void arean::mouseMoveEvent(QMouseEvent *e)
{
	if (mousePos.lb)
	{
		int32_t posx = (int32_t)e->x();
		int32_t posy = (int32_t)e->y();
		camera.rotation.x -= (mousePos.y - (float)posy) * camera.mouseSensitivity;
		camera.rotation.y -= (mousePos.x - (float)posx) * camera.mouseSensitivity;
		mousePos.x = posx;
		mousePos.y = posy;
	}
}

void arean::stopWheel()
{
	camera.movement.z = 0;
	wheelTimer->stop();
}

void arean::wheelEvent(QWheelEvent *e)
{
	camera.movement.z = e->delta() / 12;
	wheelTimer->start(10);
		
}

//-----------------------



// �������� ��������
// ����� ������ ��������� � �������� �������� � �����������
VKStr::Texture LoadTextureVk(MainRenderer * renderer, std::string path)
{
	int width;        // ������ ������������ �����������
	int height;       // ������ ������������ �����������
	int channels;     // ���-�� �������
	int bpp = 4;      // ���� �� �������



	// �������� ������� (������ ����)
	unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	// ������� �������� (��������� ������� � ������ ����������)
	VKStr::Texture result = renderer->CreateTexture(
		pixels, (uint32_t)width,
		(uint32_t)height,
		(uint32_t)channels,
		(uint32_t)bpp);

	// �������� ������ ����
	stbi_image_free(pixels);

	return result;
}



arean::arean(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	timer = new QTimer();
	wheelTimer = new QTimer();
    connect(timer, &QTimer::timeout, this, &arean::thread_core);
	connect(wheelTimer, &QTimer::timeout, this, &arean::stopWheel);
	ui.lineEdit->setText(QString::number(Kepler.T0));
	ui.lineEdit_2->setText(QString::number(Kepler.LoAN));
	ui.lineEdit_3->setText(QString::number(Kepler.inclination));
	ui.lineEdit_4->setText(QString::number(Kepler.AoP));
	ui.lineEdit_5->setText(QString::number(Kepler.BSA));
	ui.lineEdit_6->setText(QString::number(Kepler.e));
	ui.lineEdit_7->setText(QString::number(Kepler.M0));
	ui.lineEdit_8->setText(QString::number(Kepler.GGC));
	connect(ui.pushButton, &QAbstractButton::clicked, this, &arean::showOrbitParam);
	connect(ui.pushButton_2, &QAbstractButton::clicked, this, &arean::setOrbitParam);
	connect(ui.pushButton_3, &QAbstractButton::clicked, this, &arean::enableEdit);
	disableEdits();
	timer->start(1); 
	
}

void arean::closeEvent(QCloseEvent * e)
{
	ready = false;
}