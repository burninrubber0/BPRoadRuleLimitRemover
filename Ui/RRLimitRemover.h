#pragma once

#include "ui_RRLimitRemover.h"

#include <cstdint>
#include <fstream>

#include <binaryio/binaryreader.hpp>
#include <binaryio/binarywriter.hpp>

#include <QStandardPaths>

class RRLimitRemover : public QDialog
{
	Q_OBJECT

public:
	RRLimitRemover(QWidget* parent = Q_NULLPTR);

private:
	Ui::BPRRLimitRemover ui;

	QString defaultPath = "C:/Program Files (x86)/Origin Games";
	QString filePath;

	// Headers for platform detection
	uint64_t elfHdrPs3 = 0x66010202464C457F; // ELF (PS3)
	uint64_t elfHdrPs4 = 0x9010102464C457F; // ELF (PS4)
	uint32_t xexHdr = 0x32584558; // XEX (X360)
	uint16_t exeHdr = 0x5A4D; // EXE (PC)

	enum Limits : uint32_t
	{
		MinTime = 2000, // Introduced in V1.3
		MinTimeV10 = 0,
		MinShowtime = 0,
		MaxTime = 600000,
		MaxTimeRM = 240000, // Introduced in PS4 1.03
		MaxShowtime = 1000000000
	};

	void connectActions();
	bool modifyFile(uint64_t head, uint64_t fsize, binaryio::BinaryReader &reader, binaryio::BinaryWriter &writer);

private slots:
	void openFile();
	void determineIfChecked();
	bool saveFile();
	void quitRRLR();
};