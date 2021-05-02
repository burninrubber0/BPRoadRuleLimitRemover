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
		tr("Executables (*.exe *.elf *.xex *.bin);;Xbox DLLs (*.xed);;All Files (*.*)"));
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

// Detect executable type and save if no errors
bool RRLimitRemover::saveFile()
{
	log("I: Started...");

	std::ifstream in;
	std::ofstream out;
	uint64_t head;

	in.open(filePath.toStdString(), std::ios::in | std::ios::binary);

	// Ensure executable is valid
	in.seekg(0);
	in.read(reinterpret_cast<char*>(&head), sizeof(head));
	if (head == 0x200000000454353) // Certified file (PS3) LE
	{
		log("E: Encryption encountered");
		QMessageBox::critical(this, "Error: Encryption encountered",
			"The executable is encrypted.<br>Please decrypt using TrueAncestor's SELF Resigner.",
			QMessageBox::Ok, QMessageBox::NoButton);
		return true;
	}
	else if (head == 0x304F534E) // NSO (Switch) LE
	{
		log("E: Unsupported platform: Switch");
		QMessageBox::critical(this, "Unsupported platform",
			"Nintendo Switch versions are not supported at this time.",
			QMessageBox::Ok, QMessageBox::NoButton);
		return true;
	}
	else if ((head & 0xFFFF) == exeHdr // EXE (PC)
		|| (head & 0xFFFFFFFF) == xexHdr // XEX (X360) LE
		|| head == elfHdrPs3 // ELF (PS3) LE
		|| head == elfHdrPs4) // ELF (PS4)
	{
		const uint64_t fileSize = std::filesystem::file_size(filePath.toStdString());
		log("I: Size is " + QString::number(fileSize));

		// Reader and writer
		const auto& buffer = std::make_shared<std::vector<uint8_t>>(fileSize);
		in.seekg(0);
		log("I: Reading file...");
		in.read(reinterpret_cast<char*>(buffer->data()), fileSize);
		auto reader = binaryio::BinaryReader(buffer);
		auto writer = binaryio::BinaryWriter();
		writer.Seek(0);
		log("I: Creating write buffer...");
		writer.Write(buffer->data(), buffer->size());

		// Detect big endian
		if ((head & 0xFFFFFFFF) == xexHdr || head == elfHdrPs3) // X360 or PS3
		{
			reader.SetBigEndian(true);
			writer.SetBigEndian(true);
		}

		// Detect xex encryption/compression
		if ((head & 0xFFFFFFFF) == xexHdr)
		{
			reader.Seek(0x14);
			int defCount = reader.Read<int>();
			int baseFileFmtOffset = 0;
			for (int i = 0; i < defCount; ++i)
			{
				reader.Seek(0x18 + (i * 4));
				if (reader.Read<int>() == 0x3FF)
				{
					baseFileFmtOffset = reader.Read<int>();
					break;
				}
			}
			reader.Seek(baseFileFmtOffset + 4);
			uint32_t baseFileFmt = reader.Read<uint32_t>();
			if ((baseFileFmt & 0xF0000) != 0 // Unencrypted
				&& (baseFileFmt & 0xF) != 1) // Uncompressed
			{
				if (((baseFileFmt & 0xF0000) >> 16) == 1 // Encrypted
					&& (baseFileFmt & 0xF) == 2) // Compressed
				{
					log("E: Encryption and compression encountered");
					QMessageBox::critical(this, "Error: Encryption/compression encountered",
						"File is encrypted and compressed.<br>Please decrypt and decompress before modding.",
						QMessageBox::Ok, QMessageBox::NoButton);
					return true;
				}
				else if (((baseFileFmt & 0xF0000) >> 16) == 1) // Encrypted
				{
					log("E: Encryption encountered");
					QMessageBox::critical(this, "Error: Encryption encountered",
						"File is encrypted.<br>Please decrypt before modding.",
						QMessageBox::Ok, QMessageBox::NoButton);
					return true;
				}
				else if ((baseFileFmt & 0xF) == 2) // Compressed
				{
					log("E: Compression encountered");
					QMessageBox::critical(this, "Error: Compression encountered",
						"File is compressed.<br>Please decompress before modding.",
						QMessageBox::Ok, QMessageBox::NoButton);
					return true;
				}
				else
				{
					log("E: Unrecognized base file format");
					QMessageBox::critical(this, "Unrecognized base file format",
						"Cannot determine if executable is encrypted/compressed.<br>Operation halted.",
						QMessageBox::Ok, QMessageBox::NoButton);
					return true;
				}
			}
		}

		// Write backup
		log("I: Writing backup...");
		QString backupPath = filePath + ".bck";
		if (!std::filesystem::exists(backupPath.toStdString().c_str()))
			std::filesystem::copy(filePath.toStdString().c_str(), backupPath.toStdString().c_str());
		log("S: Backup written");

		// Mod executable and write
		if (!modifyFile(head, fileSize, reader, writer))
		{
			log("I: Writing modded file...");
			out.open(filePath.toStdString(), std::ios::out | std::ios::binary);
			out << writer.GetStream().rdbuf();
			out.close();
			log("S: Modification successful");

			QMessageBox::information(this, "Modification Complete",
				"Road Rule limits removed successfully!", QMessageBox::Ok);
		}
		else
			log("E: Modification failed");
	}
	else // Unrecognized
	{
		log("E: Unrecognized file format");
		QMessageBox::critical(this, "Unrecognized file",
			"File not recognized.<br>Please select a valid executable.",
			QMessageBox::Ok, QMessageBox::NoButton);
		return true;
	}

	in.close();

	return false;
}

// Enable apply button if checked
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

// Log messages
void RRLimitRemover::log(QString msg)
{
	ui.pteLog->appendPlainText(QTime::currentTime().toString() + " " + msg);
	QApplication::processEvents();
}

// Determine platform and version
bool RRLimitRemover::modifyFile(uint64_t head, uint64_t fsize, binaryio::BinaryReader &reader, binaryio::BinaryWriter &writer)
{
	if ((head & 0xFFFF) == exeHdr)
		log("I: Platform is PC");
	else if (head == elfHdrPs3)
		log("I: Platform is PS3");
	else if (head == elfHdrPs4)
		log("I: Platform is PS4");
	else if ((head & 0xFFFFFFFF) == xexHdr)
		log("I: Platform is Xbox 360");

	reader.Seek(0);
	off_t minLimitsOffset = 0;
	off_t maxLimitsOffset = 0;

	if ((head & 0xFFFF) == exeHdr
		|| head == elfHdrPs3
		|| head == elfHdrPs4)
	{
		switch (fsize)
		{
		// *** PC versions ***
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

		// *** PS3 versions ***
		// V1.0
		case 0xCD6F00: // BLAS50039 1.0, BLUS30061 1.0
		case 0xCD6F18: // BLES00073 1.0
			reader.Seek(0x8F9D8C);
			if (reader.Read<uint32_t>() == 600000) // Is BLAS50039/BLES00073
			{
				minLimitsOffset = 0x8F9D94;
				maxLimitsOffset = 0x8F9D8C;
			}
			else // Is BLUS30061
			{
				minLimitsOffset = 0x8F9DB4;
				maxLimitsOffset = 0x8F9DAC;
			}
			break;
		case 0xD0F098: // BLES00074 1.0
			minLimitsOffset = 0x8F9F74;
			maxLimitsOffset = 0x8F9F6C;
			break;
		case 0xD0F118: // BLJM60053 1.0
			minLimitsOffset = 0x8F9FB4;
			maxLimitsOffset = 0x8F9FAC;
			break;

		// V1.3
		case 0xDE8058: // All 1.3
			minLimitsOffset = 0x963040;
			maxLimitsOffset = 0x963038;
			break;

		// V1.4
		case 0xDC9978: // Disc 1.4
			minLimitsOffset = 0x969B74;
			maxLimitsOffset = 0x969B6C;
			break;
		case 0xDCA208: // PSN 1.4
			minLimitsOffset = 0x969384;
			maxLimitsOffset = 0x96937C;
			break;

		// V1.5
		case 0xDC95C0: // Disc 1.5
			minLimitsOffset = 0x964BBC;
			maxLimitsOffset = 0x964BB4;
			break;
		case 0xDC9F50: // PSN 1.5
			minLimitsOffset = 0x9651D4;
			maxLimitsOffset = 0x9651CC;
			break;

		// V1.6
		case 0xDD7478: // All 1.6
			minLimitsOffset = 0x98BA38;
			maxLimitsOffset = 0x98BA40;
			break;

		// V1.7
		case 0xDF4698: // All 1.7
			minLimitsOffset = 0x9803BC;
			maxLimitsOffset = 0x9803C4;
			break;

		// V1.8
		case 0xE05738: // All 1.8
			minLimitsOffset = 0x9901FC;
			maxLimitsOffset = 0x990204;
			break;

		// V1.9
		case 0xE282B8: // All 1.9
			minLimitsOffset = 0x9B0330;
			maxLimitsOffset = 0x9B0338;
			break;

		// *** PS4 versions ***
		case 0x17EB75D: // All 1.00
			minLimitsOffset = 0xF3A304;
			maxLimitsOffset = 0xF3A30C;
			break;
		case 0x174901D: // All 1.01
			minLimitsOffset = 0xEE47C4;
			maxLimitsOffset = 0xEE47CC;
			break;
		case 0x17490BD: // All 1.02
			minLimitsOffset = 0xEE4AA4;
			maxLimitsOffset = 0xEE4AAC;
			break;
		case 0x174559D: // All 1.03
			minLimitsOffset = 0xEE39E4;
			maxLimitsOffset = 0xEE39EC;
			break;

		default:
			log("E: Unrecognized executable");
			QMessageBox::critical(this, "Unrecognized executable",
				"Executable not recognized.<br>Did you remember to decrypt?",
				QMessageBox::Ok, QMessageBox::NoButton);
			return true;
		}
	}

	// *** Xbox 360 versions ***
	else if ((head & 0xFFFFFFFF) == xexHdr)
	{
		switch (fsize)
		{
		case 0xB6B000: // 20B57926 (V2, NTSC), 1B7D0C5B (V1, PAL)
			minLimitsOffset = 0xC4284;
			maxLimitsOffset = 0xC428C;
			break;
		case 0xB73000: // 1E51ADC2 (V7, NTSC-J)
			minLimitsOffset = 0xC4274;
			maxLimitsOffset = 0xC427C;
			break;
		case 0xBBB000: // 2B99A9CE (V4, NTSC-J)
			minLimitsOffset = 0xC430C;
			maxLimitsOffset = 0xC4314;
			break;
		case 0xBB3000: // 24EF448B (V5, PAL)
			minLimitsOffset = 0xC428C;
			maxLimitsOffset = 0xC4294;
			break;
		case 0xB93000: // 2B99A9CE (V8, All - TUB)
			minLimitsOffset = 0xC511C;
			maxLimitsOffset = 0xC5124;
			break;
		case 0xBD3000: // V1.4 update
			minLimitsOffset = 0xD2AA4;
			maxLimitsOffset = 0xD2AAC;
			break;
		case 0xC1B000: // V1.6 update
			minLimitsOffset = 0xD3FEC;
			maxLimitsOffset = 0xD3FF4;
			break;
		case 0xC23000: // V1.7 update
			minLimitsOffset = 0xD4024;
			maxLimitsOffset = 0xD402C;
			break;
		case 0xC33000: // V1.8 update
			minLimitsOffset = 0xD4254;
			maxLimitsOffset = 0xD425C;
			break;
		case 0xC53000: // V1.9 update
			minLimitsOffset = 0xD421C;
			maxLimitsOffset = 0xD4224;
			break;
		default:
			log("E: Unrecognized executable");
			QMessageBox::critical(this, "Unrecognized executable",
				"Executable not recognized.<br>Did you remember to decrypt/decompress?",
				QMessageBox::Ok, QMessageBox::NoButton);
			return true;
		}
	}
	
	// Unrecognized
	else
	{
		log("E: Unrecognized or invalid executable");
		QMessageBox::critical(this, "Unrecognized executable",
			"Executable not recognized.<br>Please select a valid executable.",
			QMessageBox::Ok, QMessageBox::NoButton);
		return true;
	}

	log("I: Min limits at 0x" + QString::number(minLimitsOffset, 16).toUpper()
		+ ", max limits at 0x" + QString::number(maxLimitsOffset, 16).toUpper());

	// Write new minimums
	// Actually only alters time
	if (ui.chkTime->isChecked())
	{
		reader.Seek(minLimitsOffset);
		uint32_t tmp = reader.Read<uint32_t>();
		if (tmp == MinTime || tmp == MinTimeV10)
		{
			writer.VisitAndWrite<uint64_t>(minLimitsOffset, 0); // Time/Showtime = 0
			log("I: Wrote new Time limits");
		}
		else
		{
			log("E: Min limits do not match known minimums");
			return true;
		}
	}
	// Write new maximums
	// Actually only alters showtime in OG
	// Time is changed in last update of RM, plus PC and NX
	if (ui.chkShowtime->isChecked())
	{
		reader.Seek(maxLimitsOffset);
		uint32_t tmp = reader.Read<uint32_t>();
		if (tmp == MaxTime || tmp == MaxTimeRM)
		{
			// Not sure whether it's signed, this will have to do
			writer.Seek(maxLimitsOffset);
			// High time limit causes issues, using normal one instead
			writer.Write<uint32_t>(MaxTime); // Time = 10 mins
			writer.Write<uint32_t>(0x77359400); // Showtime = 2 billion
			log("I: Wrote new Showtime limits");
		}
		else
		{
			log("E: Max limits do not match known maximums");
			return true;
		}
	}

	return false;
}

// ***** Constructors *****
RRLimitRemover::RRLimitRemover(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	connectActions();
}