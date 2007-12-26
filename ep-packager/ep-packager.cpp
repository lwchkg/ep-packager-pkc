/*
 * ep-packager.cpp
 * Copyright (C) 2007 WC Leung
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 **/

#include "ep-packager.h"
#include "img2pdf.h"
#include "qstrnatcmp.h"
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/quazipnewinfo.h>

const qint64 bufSize = 32768;

bool srcDirChanged = false, srcFileChanged = false;

// public function (to be moved away)
static QStringList QDirFilters(const QString &s) {
	QRegExp re("\\((.*)\\)"), reSplit(" *; *");
	if (re.indexIn(s) != -1) {
		return re.cap(1).split(reSplit);
	} else {
		return s.split(reSplit);
	}
}

// class functions
ep_packager::ep_packager(QWidget *parent) :
	QWidget(parent) {
	ui.setupUi(this);
}

ep_packager::~ep_packager() {
}

void ep_packager::on_srcDir_textChanged() {
	srcDirChanged = true;
	if (!ui.srcDir->property("focus").toBool())
		on_srcDir_editingFinished();
}

void ep_packager::on_srcDir_editingFinished() {

	if (!srcDirChanged) return;
	srcDirChanged = false;

	int c, i, maxV;
	QMap<QString, int> m;
	QString maxK;
	QStringList list;

	QDir dir(ui.srcDir->text());
	if (!dir.exists()) {
		// directory does not exist
		setInfo("datacount", tr("<nil>"));
		return;
	}
	list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	// if no of directories >= 10, then directories should be the correct type
	if (list.count() >= 10) {
		ui.srcType->lineEdit()->setText("<Directories>");
		setInfo("datacount", list.count());
		return;
	}

	list = dir.entryList(QDir::Files);
	list = list.replaceInStrings(QRegExp("^.+\\."), "");
	list.sort();

	// count file extensions (in lower case)
	c = list.count();
	for (i=0; i<c; i++)
		m[list[i].toLower()]++;

	// find most frequent file extension, and accept it as correct type if count >= 10
	QMapIterator<QString, int> iter(m);
	maxV = 0;
	while (iter.hasNext()) {
		iter.next();
		if (iter.value() > maxV) {
			maxK = iter.key();
			maxV = iter.value();
		}
	}

	// accept extension if count >= 10
	if (maxV >= 10) {
		ui.srcType->lineEdit()->setText("*." + maxK);
		setInfo("datacount", maxV);
	} else
		setInfo("datacount", tr("<nil>"));

}

void ep_packager::on_srcDir_more_clicked() {
	QString dir =
			QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,
					tr("Open Directory with Portfolio Items"),
					ui.srcDir->text()));
	if (!dir.isNull())
		ui.srcDir->setText(dir);
}

void ep_packager::on_srcFile_textChanged() {
	srcFileChanged = true;
	if (!ui.srcFile->property("focus").toBool())
		on_srcFile_editingFinished();
}

void ep_packager::on_srcFile_editingFinished() {

	if (!srcFileChanged) return;
	srcFileChanged = false;

	QuaZip z(ui.srcFile->text());
	try {
		if (!z.open(QuaZip::mdUnzip)) {
			//ui.debug_con->append("Can't open file"); //debug
			throw 1;
		}

		if (!z.setCurrentFile(".metainfo/title")) {
			//ui.debug_con->append("Bad Zip file"); //debug
			throw 1;
		}
	} catch (...) {
		setInfo("students", "<nil>");
		setInfo("title", "<nil>");
		return;
	}

	QuaZipFile zf(&z, this);
	zf.open(QIODevice::ReadOnly);
	setInfo("title", QString(zf.readAll()));

	QStringList list;
	for (bool more=z.goToFirstFile(); more; more=z.goToNextFile()) {
		list << z.getCurrentFileName();
	}

	z.close();

	list = list.filter(QRegExp("^[^.].*/$"));
	list.sort();

	setInfo("students", list.count());
}

void ep_packager::on_srcFile_more_clicked() {
	QString fileName = QDir::toNativeSeparators(QFileDialog::getOpenFileName(
			this, tr("Open Zip File"), ui.srcFile->text(),
			tr("Zip Archives (*.zip)")));
	if (!fileName.isNull())
		ui.srcFile->setText(fileName);
}

void ep_packager::on_cmdProcess_clicked() {
	int c, i, j, filesPerStudent, fileNo;
	qint64 noBytes, totalBytes;
	char buf[bufSize];
	bool dirMode, pdfMode;
	QString srcDir = QDir::fromNativeSeparators(ui.srcDir->text());
	QString srcFile = QDir::fromNativeSeparators(ui.srcFile->text());
	QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd-hhmmss");
	QString destFile;
	QDir dir;
	QStringList srcDirList, fileList, missingList;
	QMap<QString, QStringList> toAddList;

	ui.debug_con->append("Start processing.");

	QuaZip zSrc(srcFile);

	// Check validity of "missing items"
	if (ui.itemMissing->text().contains(',')) {
		missingList = ui.itemMissing->text()
		.toLower().split(QRegExp(" *, *")).replaceInStrings(" ", "-");
		if (missingList.contains("")) {
			QMessageBox::critical(this, tr("Process Failed"),
					tr("Invalid \"Missing Items.\""));
			return;
		}
	} else {
		missingList = ui.itemMissing->text().toLower().split(' ');
	}
	missingList.removeAll("");

	// Check validity of ZIP file
	if (!zSrc.open(QuaZip::mdUnzip) || !zSrc.setCurrentFile(".metainfo/title")) {
		QMessageBox::critical(this, tr("Process Failed"),
				tr("Can't open ZIP file."));
		return;
	}

	// Load ZIP File
	for (bool more=zSrc.goToFirstFile(); more; more=zSrc.goToNextFile()) {
		srcDirList << zSrc.getCurrentFileName();
	}

	zSrc.close();

	srcDirList = srcDirList.filter(QRegExp("^[^.].*/$"));
	srcDirList.sort();

	// Remove items specified by "missing items"
	c = missingList.count();
	for (i=0; i<c; i++) {
		QStringList removeList = srcDirList.filter(QRegExp("(^|-0?|_)"
				+ QRegExp::escape(missingList[i]) + "($|-|_)",
				Qt::CaseInsensitive));
		if (removeList.count() == 1) {
			srcDirList.removeAll(removeList[0]);
			ui.debug_con->append("Remove item: " + removeList[0]);
		} else {
			QMessageBox::critical(
					this,
					tr("Process Failed"),
					tr("Missing item \"%1\" matches %2 student.\nExactly ONE match is required.")
					.arg(missingList[i]).arg(removeList.count()));
			return;
		}
	}
	ui.debug_con->append(QString("Folders: %1").arg(srcDirList.count())); // debug

	// Check validity of source directory
	dir.setPath(srcDir);
	if (!dir.exists()) {
		QMessageBox::critical(this, tr("Process Failed"),
				tr("Portfolio directory does not exist!"));
		return;
	}

	// Count the number items in source directory
	dirMode = ui.srcType->lineEdit()->text().startsWith("<Dir", Qt::CaseInsensitive);
	if (dirMode) {
		fileList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	} else {
		ui.debug_con->append(QDirFilters(ui.srcType->lineEdit()->text()).join(","));
		fileList = dir.entryList(QDirFilters(ui.srcType->lineEdit()->text()), QDir::Files);
	}
	qSort(fileList.begin(), fileList.end(), QStrNatCaseLessThan);
	ui.debug_con->append(QString("Items: %1").arg(fileList.count())); // debug

	// Check whether the list counts match
	if (fileList.count() == 0 || dirMode ? fileList.count()
			!= srcDirList.count() : fileList.count() % srcDirList.count()) {
		QMessageBox::critical(this, tr("Process Failed"), tr("Can't put %1 items to %2 students.")
		.arg(fileList.count()).arg(srcDirList.count()));
		return;
	}

	// Propagate set of files to be added
	if (dirMode) {
		c = srcDirList.count();
		for (i=0; i<c; i++) {
			dir.setPath(QDir::cleanPath(srcDir) + "/" + fileList[i]);
			// no recursiveness at the moment
			toAddList[srcDirList[i]] = dir.entryList(QDir::Files);
			// remove Thumbs.db which is generated by explorer
			toAddList[srcDirList[i]].removeAll("Thumbs.db");
			qSort(toAddList[srcDirList[i]].begin(),
					toAddList[srcDirList[i]].end(), QStrNatCaseLessThan);
		}
	} else {
		// in non-dir mode, do not remove "Thumbs.db"
		c = srcDirList.count();
		filesPerStudent = fileList.count() / c;
		for (i=0; i<c; i++)
			for (j=0; j<filesPerStudent; j++)
				toAddList[srcDirList[i]] << fileList[i * filesPerStudent + j];
	}

	// Start making zip files
	const int maxZipFileSize = ui.maxZipFileSize->value();
	fileNo = 0;
	QMapIterator<QString, QStringList> iter(toAddList);
	for (j=0; iter.hasNext(); j++) {
		iter.next();
		ui.debug_con->append("Processing path: " + iter.key());
		QStringList processList = iter.value();

		c = processList.count();
		for (i=0; i<c; i++) {
			try {
				QByteArray pdfbuf;
				QBuffer buffer;
				QuaZip z;
				QuaZipFile zf;
				QFile file;
				QDataStream datastream;

				// Open device for reading (pdf buffer or file)
				QStringList pdfprocessList;
				pdfMode = false;
				if (i == 0 && ui.pdf_single->isChecked()) {
					pdfprocessList = processList.filter(QRegExp("\\.(jpe?g|tiff?)$",
									Qt::CaseInsensitive));
					// enable pdf mode only if there are files to convert
					if (pdfprocessList.count() >= 1)
					pdfMode = true;
				}
				if (pdfMode) {
					// create pdf for all image files
					pdfbuf = createPdf(this, pdfprocessList,
							srcDir + "/" + (dirMode ? fileList[j] : "") + "/");
					buffer.setData(pdfbuf);
					buffer.open(QIODevice::ReadOnly);
					datastream.setDevice(&buffer);

					// fixme: inefficient code
					for (int k = 0; k < pdfprocessList.count(); k++) {
						processList.removeAll(pdfprocessList[k]);
						ui.debug_con->append(QString("Added %1 to PDF file")
								.arg(pdfprocessList[k]));
					}

					processList.prepend(timestamp + ".pdf");

					c = processList.count(); // reset count
				} else {
					// open file for reading
					file.setFileName(srcDir + "/" + (dirMode ? fileList[j] : "") + "/" +
							processList[i]);
					if (!file.open(QIODevice::ReadOnly)) {
						QMessageBox::critical(this, tr("Process Failed"),
								tr("Can't open %1 for reading.")
								.arg(QDir::toNativeSeparators(file.fileName())));
						throw 0;
					}
					datastream.setDevice(&file);
				}

				// Create zip file if max size is going to be exceeded
				if (fileNo == 0 || maxZipFileSize < QFileInfo(destFile).size()
						+ datastream.device()->size() + 90 +
						2 * QString(iter.key() + processList[i]).length() ) {
					fileNo++;
					destFile = srcFile;
					if (destFile.endsWith(".zip", Qt::CaseInsensitive)) destFile.chop(4);
					destFile.append(QString("-%1.zip").arg(fileNo));
					try {
						QFile zipFile(destFile);
						if (zipFile.exists() && !zipFile.remove()) throw 0;
						if (!QFile::copy(srcFile, destFile)) throw 0;

						ui.debug_con->append("Created: " +
								QDir::toNativeSeparators(destFile)); // debug
					} catch (...) {
						QMessageBox::critical(this, tr("Process Failed"),
								tr("Can't create destination zip file."));
						return;
					}
				}

				// Open zip file for writing
				z.setFileNameCodec("UTF-8");
				z.setZipName(destFile);
				if (!z.open(QuaZip::mdAdd)) {
					QMessageBox::critical(this, tr("Process Failed"),
							tr("Can't open zip file for writing."));
					throw 1;
				}
				zf.setZip(&z);

				if (!zf.open(QIODevice::WriteOnly,
								QuaZipNewInfo(iter.key() +
										(ui.original_filename->isChecked() ?
												processList[i] : timestamp + QString("-%1").arg(i+1) +
												(processList[i].contains(".") ? QString(processList[i]).
														replace(QRegExp("^.+\\."), ".").toLower() : "")),
										file.fileName()))) {
					QMessageBox::critical(this, tr("Process Failed"),
							tr("Can't open zip file for writing."));
					throw 1;
				}

				// Copy the opened file to the zip archive
				for (totalBytes = 0;; totalBytes += noBytes) {
					if ((noBytes = datastream.readRawData(&buf[0], sizeof(buf))) == -1) {
						// assume we don't fail reading from byte array,
						// so the failure must be from the file
						QMessageBox::critical(this, tr("Process Failed"),
								tr("Can't read from %1.")
								.arg(QDir::toNativeSeparators(file.fileName())));
						throw 2;
					}

					if (noBytes == 0) break;

					if (zf.write(&buf[0], noBytes) != noBytes) {
						QMessageBox::critical(this, tr("Process Failed"),
								tr("Can't write zip file."));
						throw 3;
					}
				}

				ui.debug_con->append(QString("Written: %1 (%2 bytes)")
						.arg(processList[i]).arg(totalBytes));

			} catch (...) {
				// terminate process
				return;
			}
		}
	}

	ui.debug_con->append(QString("Finish processing. Created %1 files.").arg(fileNo));
	if (fileNo > 1)
		QMessageBox::information(this, tr("e-Portfolio Packager"),
				tr("Finish processing.\nCreated %1 files.").arg(fileNo));
	else
		QMessageBox::information(this, tr("e-Portfolio Packager"),
				tr("Finish processing."));
}

void ep_packager::setInfo(const QString &field, const QString &s) {
	if (field == "datacount")
		ui.infoDatacount->setText(QString("Files/Directories: %1").arg(s));
	else if (field == "students")
		ui.infoStudents->setText(QString("Students: %1").arg(s));
	else if (field == "title")
		ui.infoTitle->setText(QString("Title: %1").arg(s));
}
void ep_packager::setInfo(const QString &field, qint64 n) {
	setInfo(field, QString("%1").arg(n));
}
