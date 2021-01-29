#include "RRLimitRemover.h"

#include "ui_RRLimitRemover.h"

#include <QFileDialog>

// ***** Slots *****
// Load executable for editing
void RRLimitRemover::openFile()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Select exectutable"), defaultPath,
		tr("Executables (*.exe *.elf *.xex)"));
	loadFile(filePath);
}
// Save modified executable
void RRLimitRemover::saveFile()
{

}

// Quit program
void RRLimitRemover::quitRRLR()
{
	QApplication::quit();
}

// ***** UI *****
// Connections
void RRLimitRemover::connectActions()
{
	connect(ui.btnBrowse, &QToolButton::clicked, this, &RRLimitRemover::openFile);
	connect(ui.btnApply, &QPushButton::clicked, this, &RRLimitRemover::saveFile);
	connect(ui.btnQuit, &QPushButton::clicked, this, &RRLimitRemover::quitRRLR);
}

void RRLimitRemover::loadFile(QString path)
{
	
}

// ***** Constructors *****
RRLimitRemover::RRLimitRemover(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	connectActions();
}