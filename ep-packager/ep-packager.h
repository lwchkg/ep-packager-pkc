/*
 * ep-packager.h
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

#ifndef EP_PACKAGER_H
#define EP_PACKAGER_H

#include <QtGui/QWidget>
#include "ui_ep-packager.h"

class ep_packager : public QWidget
{
    Q_OBJECT

public:
    ep_packager(QWidget *parent = 0);
    ~ep_packager();

private slots:
	void on_srcDir_textChanged(); 
	void on_srcDir_editingFinished();
	void on_srcDir_more_clicked();

	void on_srcFile_editingFinished();
	void on_srcFile_textChanged(); 
	void on_srcFile_more_clicked();

	void on_cmdProcess_clicked();

private:
	void setInfo(const QString &field, const QString &s);
	void setInfo(const QString &field, qint64 n);
    Ui::ep_packagerClass ui;
};

#endif // EP_PACKAGER_H
