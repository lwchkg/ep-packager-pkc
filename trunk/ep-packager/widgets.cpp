/*
 * widgets.cpp
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

#include <QtGui>
#include <QDir>
#include <QFileInfo>
#include "widgets.h"

dirEdit::dirEdit(QWidget *parent) :
	QLineEdit(parent) {

}

void dirEdit::dragMoveEvent(QDragMoveEvent *event) {
	if (event->mimeData()->hasUrls() && event->mimeData()->urls().count() == 1)
		event->acceptProposedAction();
}

void dirEdit::dragEnterEvent(QDragEnterEvent *event) {
	this->dragMoveEvent(event);
}

void dirEdit::dropEvent(QDropEvent *event) {
	if (!event->mimeData()->hasUrls())
		return;
	QList<QUrl> urls(event->mimeData()->urls());
	if (urls.count() != 1)
		return;
	QFileInfo finfo(urls[0].toLocalFile());
	this->setText(QDir::toNativeSeparators(finfo.canonicalPath()));
	event->acceptProposedAction();
	emit editingFinished();
}

fileEdit::fileEdit(QWidget *parent) :
	QLineEdit(parent) {

}

void fileEdit::dragMoveEvent(QDragMoveEvent *event) {
	if (!event->mimeData()->hasUrls())
		return;
	QList<QUrl> urls(event->mimeData()->urls());
	if (urls.count() != 1)
		return;
	QFileInfo finfo(urls[0].toLocalFile());
	if (!finfo.isFile())
		return;
	event->acceptProposedAction();
}

void fileEdit::dragEnterEvent(QDragEnterEvent *event) {
	this->dragMoveEvent(event);
}

void fileEdit::dropEvent(QDropEvent *event) {
	if (!event->mimeData()->hasUrls())
		return;
	QList<QUrl> urls(event->mimeData()->urls());
	if (urls.count() != 1)
		return;
	QFileInfo finfo(urls[0].toLocalFile());
	if (!finfo.isFile())
		return;
	this->setText(QDir::toNativeSeparators(finfo.canonicalFilePath()));
	event->acceptProposedAction();
	emit editingFinished();
}
