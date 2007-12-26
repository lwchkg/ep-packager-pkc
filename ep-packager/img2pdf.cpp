/*
 * img2pdf.cpp
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

#include <pdflib/pdflib.hpp>
#include <QMessageBox>

using namespace std;

const string BOM("\xEF\xBB\xBF");

const QByteArray createPdf(QWidget *parent, QStringList &src,
		const QString &pathPrepend = "") {
	QByteArray buf;
	QString stringErr;

	try {

		PDFlib p;
		int i, frame, bparent = 0;
		bool successful;
		int image;

		const char *buf1;
		long size = 0;

		//  This means we must check return values of load_font() etc.
		p.set_parameter("errorpolicy", "return");

		p.set_parameter("hypertextencoding", "winansi");

		if (p.begin_document("", "") == -1) {
			stringErr = QString("Error: %1").arg(p.get_errmsg().c_str());
			QMessageBox::critical(parent, QObject::tr("Process Failed"), stringErr);
			return 0;
		}

		p.set_info("Creator", "E-Portfolio Packager");

		for (i=0; i < src.count();) {

			/* process all frames in a multi-page image file */
			successful = true;
			for (frame=1; /* */; frame++)
			{
				image = p.load_image("auto", BOM + string((pathPrepend + src[i]).toUtf8().data()),
						QString("page=%1").arg(frame).toStdString());
				if (image == -1) {
					if (frame == 1) successful = false;
					break;
				}

				/* dummy page size, will be adjusted later */
				p.begin_page_ext(20, 20, "");

				/* define outline with filename */
				if (frame == 1)
				bparent = p.create_bookmark(QString("File %1").arg(i+1).toStdString(),
						"open");
				else
				{
					p.create_bookmark(QString("page %1").arg(frame).toStdString(),
							QString("parent=%1").arg(bparent).toStdString());
				}

				p.fit_image(image, 0.0, 0.0, "adjustpage");
				p.close_image(image);

				p.end_page_ext("");
			}

			//if (frame > 2)
			//fprintf(stderr, "(%d frames)", frame-1);

			//fprintf(stderr, "\n");

			// delete image from list if failure, else advance counter
			if (successful)
			i++;
			else
			src.removeAt(i);
		}

		p.end_document("");

		buf1 = p.get_buffer(&size);
		buf = QByteArray(buf1, size);
	}
	catch (PDFlib::Exception &ex) {
		stringErr = "PDFlib exception occurred:\n";
		stringErr += QString("[%1] %2: %3").arg(ex.get_errnum()).arg(ex.get_apiname().c_str())
		.arg(ex.get_errmsg().c_str());
		QMessageBox::critical(parent, QObject::tr("Process Failed"), stringErr);
		return 0;
	}

	return buf;
}
