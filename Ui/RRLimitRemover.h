#pragma once

#include "ui_RRLimitRemover.h"

#include <QStandardPaths>

class RRLimitRemover : public QDialog
{
	Q_OBJECT

public:
	RRLimitRemover(QWidget* parent = Q_NULLPTR);

private:
	Ui::BPRRLimitRemover ui;

	QString defaultPath = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).at(0);

	void connectActions();
	void loadFile(QString path);

private slots:
	void openFile();
	void saveFile();
	void quitRRLR();
};