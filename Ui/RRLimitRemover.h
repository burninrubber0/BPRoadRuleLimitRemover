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
		MinTime = 2000,
		MinShowtime = 0,
		MaxTime = 600000,
		MaxTimeRM = 240000,
		MaxShowtime = 1000000000
	};

	// Platforms
	enum class Platform
	{
		PC,
		PS3,
		PS4,
		Switch,
		X360
	};

	enum class Version
	{
		// OG Console
		V10,
		V13,
		V14,
		V15,
		V16,
		V17,
		V18,
		V19,

		// OG PC
		Securom_V1000,
		Securom_V1001,
		Securom_V1100,
		Origin_V1100,
		Steam_V1100,

		// Remastered PS4
		Remastered100,
		Remastered101,
		Remastered102,
		Remastered103
	};

	enum class Variant
	{
		// OG PS3 Title ID
		BLAS50039,
		BLES00073,
		BLES00074,
		BLJM60053,
		BLUS30061,

		// TUB PS3 Title ID
		BLES00455,
		BLES00462,
		BLJM60133,
		BLUS30250,

		// PSN PS3 Title ID
		NPEB00043,
		NPHB00052,
		NPJB00011,
		NPUB30040,

		// TODO: Demo PS3 Title ID

		// PS4 Title ID
		CUSA00851,
		CUSA00866,

		// OG X360 Media ID
		ID_1B7D0C5B, // PAL, V1
		ID_20B57926, // NTSC, V2
		ID_2B99A9CE, // NTSC-J, V4
		ID_24EF448B, // PAL, V5
		ID_1E51ADC2, // NTSC-J, V7

		// TUB X360 Media ID
		ID_7CCD18F1 // All Regions, V8

		// TODO: XBL X360 Media ID
	};

	void connectActions();
	void modifyFile(uint64_t fsize, binaryio::BinaryReader &reader, binaryio::BinaryWriter &writer);

private slots:
	void openFile();
	void determineIfChecked();
	void saveFile();
	void quitRRLR();
};