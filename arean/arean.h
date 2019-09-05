#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_arean.h"
#include <glm\glm.hpp>





class arean : public QMainWindow
{
	Q_OBJECT

public:
	arean(QWidget *parent = Q_NULLPTR);
	auto vecforform();
	
private:
	Ui::areanClass ui;
	int thread_core();
	virtual void resizeEvent(QResizeEvent *e) override;
	virtual void keyPressEvent(QKeyEvent * e) override;
	virtual void keyReleaseEvent(QKeyEvent * e) override;
	virtual void closeEvent(QCloseEvent * e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	virtual void mouseMoveEvent(QMouseEvent *e) override;
	virtual void wheelEvent(QWheelEvent *e) override;
	QTimer *timer;
	QTimer *wheelTimer;
	void stopWheel();
	void showOrbitParam();
	void setOrbitParam();
	void enableEdit();
	void disableEdits();
};

