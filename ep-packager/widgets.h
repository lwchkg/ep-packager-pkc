/*
 * widgets.h
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

#include <QLineEdit>

#ifndef WIDGETS_H_
#define WIDGETS_H_

class QDragMoveEvent;
class QDragEnterEvent;
class QDropEvent;

class dirEdit : public QLineEdit {

public:
	dirEdit(QWidget *parent);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

};

class fileEdit : public QLineEdit {

public:
	fileEdit(QWidget *parent);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

};

#endif /*WIDGETS_H_*/

