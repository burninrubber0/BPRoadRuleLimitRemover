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

	QString defaultPath = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation).at(0);
	QString filePath;

	// Headers for platform detection
	uint64_t certFileHdrPs3 = 0x0200000000454353; // Certified file (PS3)
	uint64_t elfHdrPs3 = 0x7F454C4602020166; // ELF (PS3)
	uint64_t elfHdrPs4 = 0x09010102464C457F; // ELF (PS4)
	uint32_t xexHdr = 0x58455832; // XEX (X360)
	uint16_t exeHdr = 0x5A4D; // EXE (PC)
	uint64_t nsoHdr = 0x00000000304F534E; // NSO (Switch)

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
	bool modifyFile(uint64_t fsize, binaryio::BinaryReader &reader, binaryio::BinaryWriter &writer);

private slots:
	void openFile();
	void determineIfChecked();
	void saveFile();
	void quitRRLR();
};