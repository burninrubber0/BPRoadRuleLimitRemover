#include "Ui/RRLimitRemover.h"

#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	RRLimitRemover* w = new RRLimitRemover();
	w->show();
	return a.exec();
}