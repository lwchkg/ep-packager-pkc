/* -*- mode: c; c-file-style: "k&r" -*-

 qstrnatcmp.cpp -- Modified 'natural order' comparision for strings in Qt.
 Copyright (C) 2007 by WC Leung

 Note: this is essentially a C program except for the use of C++ datatype
       'QString'

 modified from strnatcmp.c (see below)

 strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
 Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 */

/* partial change history:
 *
 * 2007-12-19 wcleung: Removed fractional expressions. Leading zeros are
 *                     discarded in the comparison.
 * 2004-10-10 mbp: Lift out character type dependencies into macros.
 *
 * Eric Sosman pointed out that ctype functions take a parameter whose
 * value must be that of an unsigned int, even on platforms that have
 * negative chars in their default char type.
 */

#include <QChar>
#include <QString>
#include "qstrnatcmp.h"

static int Qcompare_right(const QChar *a, const QChar *b) {
	int bias = 0;

	/* The longest run of digits wins.  That aside, the greatest
	 value wins, but we can't know that it will until we've scanned
	 both numbers to know that they have the same magnitude, so we
	 remember it in BIAS. */
	for (;; a++, b++) {
		if (!a->isDigit() && !b->isDigit())
			return bias;
		else if (!a->isDigit())
			return -1;
		else if (!b->isDigit())
			return +1;
		else if (*a < *b && !bias)
			bias = -1;
		else if (*a > *b && !bias)
			bias = +1;
		else if (a->isNull() && b->isNull())
			return bias;
	}

	return 0;
}

/* Compare two left-aligned numbers: the first to have a
 different value wins. */
/*static int Qcompare_left(const QChar *a, const QChar *b) {
	for (;; a++, b++) {
		if (!a->isDigit() && !b->isDigit())
			return 0;
		else if (!a->isDigit())
			return -1;
		else if (!b->isDigit())
			return +1;
		else if (*a < *b)
			return -1;
		else if (*a > *b)
			return +1;
		else if (a->isNull() && b->isNull())
			return 0;
	}

	return 0;
}*/

static int QStrNatCmp0(const QString &a, const QString &b, int fold_case)
{
	const QChar *ca = a.constData(), *cb = b.constData();
	//int fractional;
	int result;

	while (1) {

		/* skip over leading spaces or zeros */
		while (ca->isSpace())
			ca++;

		while (cb->isSpace())
			cb++;

		/* process run of digits */
		if (ca->isDigit() && cb->isDigit()) {
			//fractional = (*ca == '0' || *cb == '0');
			
			/* leading zeros does not count, so just skip them */
			while (*ca == '0') ca++;
			while (*cb == '0') cb++; 

			//if (fractional) {
			//	if ((result = Qcompare_left(ca, cb)) != 0)
			//		return result;
			//} else {
				if ((result = Qcompare_right(ca, cb)) != 0)
					return result;
			//}
		}

		if (ca->isNull() && cb->isNull()) {
			/* The strings compare the same.  Perhaps the caller
			 will want to call strcmp to break the tie. */
			return 0;
		}

		if (fold_case ? ca->toUpper() < cb->toUpper() : *ca < *cb)
			return -1;
		else if (fold_case ? ca->toUpper() > cb->toUpper() : *ca > *cb)
			return +1;

		ca++; cb++;
	}
}

int QStrNatCmp(const QString &a, const QString &b) {
	return QStrNatCmp0(a, b, 0);
}

/* Compare, recognizing numeric string and ignoring case. */
int QStrNatCaseCmp(const QString &a, const QString &b) {
	return QStrNatCmp0(a, b, 1);
}

bool QStrNatLessThan(const QString &a, const QString &b) {
	return QStrNatCmp0(a, b, 0) < 0;
}

bool QStrNatCaseLessThan(const QString &a, const QString &b) {
	return QStrNatCmp0(a, b, 1) < 0;
}
