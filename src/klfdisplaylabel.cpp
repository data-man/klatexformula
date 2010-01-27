/***************************************************************************
 *   file klfdisplaylabel.cpp
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2009 by Philippe Faist
 *   philippe.faist at bluewin.ch
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/* $Id$ */

#include <QLabel>
#include <QDir>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QVariant>

#include "klfconfig.h"
#include "klfdisplaylabel.h"


KLFDisplayLabel::KLFDisplayLabel(QWidget *parent)
  : QLabel(parent), mTooltipFile(NULL)
{
  setText(QString());
  setLabelFixedSize(QSize(120,80));

  pDefaultPalette = palette();
  pErrorPalette = pDefaultPalette;
  pErrorPalette.setColor(QPalette::Window, QColor(255, 200, 200));
}

KLFDisplayLabel::~KLFDisplayLabel()
{
  if (mTooltipFile)
    delete mTooltipFile;
}

void KLFDisplayLabel::setLabelFixedSize(const QSize& size)
{
  pLabelFixedSize = size;
  setFixedSize(size);
}


void KLFDisplayLabel::displayClear()
{
  setPixmap(QPixmap());
  setText(QString());
  setEnabled(false);
  set_error(false);
}

void KLFDisplayLabel::display(QImage displayimg, QImage tooltipimage, bool labelenabled)
{
  setPixmap(QPixmap::fromImage(displayimg));

  // un-set any error
  set_error(false);

  if (mTooltipFile) {
    delete mTooltipFile;
    mTooltipFile = 0;
  }
  // no big preview by default
  _bigPreviewText = "";
  // but if one is given then prepare it (prepare it even if "enableToolTipPreview" is false,
  // because we will need it for the "showBigPreview" button)
  if ( ! tooltipimage.isNull() ) {
    mTooltipFile = new QTemporaryFile(this);
    if ( ! mTooltipFile->open() ) {
      qWarning("WARNING: Failed open for Tooltip Temp Image!\n%s\n",
	       qPrintable(mTooltipFile->fileTemplate()));
      delete mTooltipFile;
      mTooltipFile = 0;
    } else {
      mTooltipFile->setAutoRemove(true);
      bool res = tooltipimage.save(mTooltipFile, "PNG");
      if ( ! res ) {
	QMessageBox::critical(this, tr("Error"), tr("Failed write to ToolTip Temp Image file %1!")
			      .arg(mTooltipFile->fileName()));
	qWarning("WARNING: Failed write to Tooltip temp image to temporary file `%s' !\n",
		 qPrintable(mTooltipFile->fileTemplate()));
	delete mTooltipFile;
	mTooltipFile = 0;
      } else {
	_bigPreviewText = QString("<img src=\"%1\">").arg(mTooltipFile->fileName());
      }
    }
  }
  if (klfconfig.UI.enableToolTipPreview) {
    setToolTip(QString("<p>%1</p>").arg(_bigPreviewText));
  } else {
    setToolTip(QString(""));
  }

  setEnabled(labelenabled);
}

void KLFDisplayLabel::displayError(bool labelenabled)
{
  set_error(true);
  setEnabled(labelenabled);
}


void KLFDisplayLabel::set_error(bool error_on)
{
  setProperty("realTimeLatexError", QVariant(error_on));
  QPalette *p;
  if (error_on) {
    p = &pErrorPalette;
  } else {
    p = &pDefaultPalette;
  }
  setStyleSheet(styleSheet()); // force style sheet refresh
  setPalette(*p);
}

void KLFDisplayLabel::mouseMoveEvent(QMouseEvent */*e*/)
{
  emit labelDrag();
}
