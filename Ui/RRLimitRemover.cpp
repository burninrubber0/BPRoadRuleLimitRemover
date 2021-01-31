#include "RRLimitRemover.h"

#include "ui_RRLimitRemover.h"

#include <cstdint>
#include <filesystem>
#include <fstream>

#include <binaryio/binaryreader.hpp>
#include <binaryio/binarywriter.hpp>

#include <QFileDialog>
#include <QMessageBox>

// ***** Slots *****
// Load executable for editing
void RRLimitRemover::openFile()
{
	QString tmpPath;
	if (!filePath.isEmpty())
		tmpPath = filePath;
	filePath = QFileDialog::getOpenFileName(this, tr("Select executable"), defaultPath,
		tr("Executables (*.exe *.elf *.xex)"));
	if (std::filesystem::exists(filePath.toStdString().c_str()))
	{
		ui.lnExecutable->setText(filePath);
		ui.chkTime->setEnabled(true);
		ui.chkShowtime->setEnabled(true);
	}
	else if (std::filesystem::exists(ui.lnExecutable->text().toStdString().c_str()))
	{
		filePath = ui.lnExecutable->text();
		ui.chkTime->setEnabled(true);
		ui.chkShowtime->setEnabled(true);
	}
	else
	{
		ui.chkTime->setEnabled(false);
		ui.chkShowtime->setEnabled(false);
	}
}

// Save modified executable
void RRLimitRemover::saveFile()
{
	std::ifstream in;
	std::ofstream out;

	// Open executable, copy to backup, output backup, mod, output mod
	// Write backup
	QString backupPath = filePath + ".bck";
	if (!std::filesystem::exists(backupPath.toStdString().c_str()))
		std::filesystem::copy(filePath.toStdString().c_str(), backupPath.toStdString().c_str());

	in.open(filePath.toStdString(), std::ios::in | std::ios::binary);
	const uint64_t fileSize = std::filesystem::file_size(filePath.toStdString());

	// Load file into reader and writer
	const auto& buffer = std::make_shared<std::vector<uint8_t>>(fileSize);
	in.read(reinterpret_cast<char*>(buffer->data()), fileSize);
	auto reader = binaryio::BinaryReader(buffer);
	auto writer = binaryio::BinaryWriter();
	writer.Seek(0);
	writer.Write(buffer->data(), buffer->size());

	// Detect big endian
	uint64_t head;
	in.seekg(0);
	in.read(reinterpret_cast<char*>(&head), sizeof(head));
	if ((head & 0xFFFFFFFF) == 0x32584558 || head == 0x66010202464C457F)
	{
		reader.SetBigEndian(true);
		writer.SetBigEndian(true);
	}

	in.close();

	// Mod executable and write
	modifyFile(fileSize, reader, writer);
	out.open(filePath.toStdString(), std::ios::out | std::ios::binary);
	out << writer.GetStream().rdbuf();
	out.close();

	QMessageBox::information(this, "Modification Complete", "Road Rule limits removed successfully!",
		QMessageBox::Ok, QMessageBox::NoButton);
}

void RRLimitRemover::determineIfChecked()
{
	// Make sure mods are selected
	if (!ui.chkTime->isChecked() && !ui.chkShowtime->isChecked())
		ui.btnApply->setEnabled(false);
	else
		ui.btnApply->setEnabled(true);
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
	connect(ui.chkTime, &QCheckBox::stateChanged, this, &RRLimitRemover::determineIfChecked);
	connect(ui.chkShowtime, &QCheckBox::stateChanged, this, &RRLimitRemover::determineIfChecked);
}

// Determine platform, version, 
void RRLimitRemover::modifyFile(uint64_t fsize, binaryio::BinaryReader &reader, binaryio::BinaryWriter &writer)
{
	reader.Seek(0);
	uint64_t head = reader.Read<uint64_t>();
	off_t minLimitsOffset = 0;
	off_t maxLimitsOffset = 0;

	if ((head & 0xFFFF) == exeHdr
		|| head == elfHdrPs3
		|| head == elfHdrPs4)
	{
		switch (fsize)
		{
		case 0x11FB510: // Securom 1.0.0.0
		case 0x11FE510: // Securom 1.0.0.1
			minLimitsOffset = 0x6599F4;
			maxLimitsOffset = 0x6599FC;
			break;
		case 0x12D4510: // Securom 1.1.0.0
		case 0xC87510:  // Steam 1.1.0.0
		case 0xC3DB48:  // Origin 1.1.0.0
			minLimitsOffset = 0x666DC8;
			maxLimitsOffset = 0x666DD0;
			break;
		case 0x7C8E9B8: // PC Remastered
		case 0xE048110: // PC Remastered trial
			minLimitsOffset = 0xADDF08;
			maxLimitsOffset = 0xADDF00;
			break;
		}
	}

	// Write changes
	if (ui.chkTime->isChecked())
	{
		reader.Seek(minLimitsOffset);
		if (reader.Read<uint32_t>() == MinTime
			|| reader.Read<uint32_t>() == MaxTime
			|| reader.Read<uint32_t>() == MaxTimeRM)
			writer.VisitAndWrite<uint64_t>(minLimitsOffset, 0); // Time/Showtime minimum = 0
	}
	if (ui.chkShowtime->isChecked())
	{
		if (reader.Read<uint32_t>() == MinShowtime
			|| reader.Read<uint32_t>() == MaxShowtime)
			// Not sure whether it's signed, this will have to do
			writer.VisitAndWrite<uint64_t>(maxLimitsOffset, 0x7FFFFFFF7FFFFFFF); // Time/Showtime maximum = 2147483647
	}
}

// ***** Constructors *****
RRLimitRemover::RRLimitRemover(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	connectActions();
}