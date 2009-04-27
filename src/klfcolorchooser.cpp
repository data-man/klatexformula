/***************************************************************************
 *   file klfcolorchooser.cpp
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2008 by Philippe Faist
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

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QColorDialog>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOptionButton>

#include "klfconfig.h"
#include "klfcolorchooser.h"


#define cstr(x) (x).toLocal8Bit().constData()


// -------------------------------------------------------------------

KLFColorChooseWidgetPane::KLFColorChooseWidgetPane(QWidget *parent)
  : QWidget(parent), _img()
{
}

void KLFColorChooseWidgetPane::setColor(const QColor& newcolor)
{
  _color = newcolor;
  update();
  emit colorChanged(_color);
}
void KLFColorChooseWidgetPane::setPaneType(const QString& panetype)
{
  QStringList strlist = panetype.split("+");
  _colorcomponent = strlist[0].toLower();
  _colorcomponent_b = strlist[1].toLower();
}
void KLFColorChooseWidgetPane::paintEvent(QPaintEvent */*e*/)
{
  QPainter p(this);
  int x;
  int y;
  _img = QImage(width(), height(), QImage::Format_ARGB32);
  double xfac = (double)valueAMax() / (_img.width()-1);
  double yfac = (double)valueBMax() / (_img.height()-1);
  for (x = 0; x < _img.width(); ++x) {
    for (y = 0; y < _img.height(); ++y) {
      _img.setPixel(x, y, colorFromValues(_color, xfac*x, yfac*y).rgba());
    }
  }
  p.drawImage(0, 0, _img);
  // draw crosshairs
  QColor hairscol = _color.value() > 50 ? Qt::black : Qt::white;
  if ( ! _colorcomponent.isEmpty() && _colorcomponent != "fix" ) {
    p.setPen(QPen(hairscol, 1.f, Qt::DotLine));
    x = valueA()/xfac;
    if (x < 0) x = 0; if (x >= width()) x = width()-1;
    p.drawLine(x, 0, x, height());
  }
  if ( ! _colorcomponent_b.isEmpty() && _colorcomponent_b != "fix" ) {
    p.setPen(QPen(hairscol, 1.f, Qt::DotLine));
    y = valueB()/yfac;
    if (y < 0) y = 0; if (y >= height()) y = height()-1;
    p.drawLine(0, y, width(), y);
  }
}
void KLFColorChooseWidgetPane::mousePressEvent(QMouseEvent *e)
{
  double xfac = (double)valueAMax() / (_img.width()-1);
  double yfac = (double)valueBMax() / (_img.height()-1);
  int x = e->pos().x();
  int y = e->pos().y();

  setColor(colorFromValues(_color, x*xfac, y*yfac));
}
void KLFColorChooseWidgetPane::mouseMoveEvent(QMouseEvent *e)
{
  double xfac = (double)valueAMax() / (_img.width()-1);
  double yfac = (double)valueBMax() / (_img.height()-1);
  int x = e->pos().x();
  int y = e->pos().y();
  if (x < 0) x = 0; if (x >= width()) x = width()-1;
  if (y < 0) y = 0; if (y >= height()) y = height()-1;

  setColor(colorFromValues(_color, x*xfac, y*yfac));
}


// -------------------------------------------------------------------


int KLFColorComponentsEditorBase::valueAFromNewColor(const QColor& color) const
{
  return valueFromNewColor(color, _colorcomponent);
}
int KLFColorComponentsEditorBase::valueBFromNewColor(const QColor& color) const
{
  return valueFromNewColor(color, _colorcomponent_b);
}
int KLFColorComponentsEditorBase::valueFromNewColor(const QColor& color, const QString& component)
{
  int value = -1;
  if (component == "hue") {
    value = color.hue();
  } else if (component == "sat") {
    value = color.saturation();
  } else if (component == "val") {
    value = color.value();
  } else if (component == "red") {
    value = color.red();
  } else if (component == "green") {
    value = color.green();
  } else if (component == "blue") {
    value = color.blue();
  } else if (component == "alpha") {
    value = color.alpha();
  } else if (component == "fix" || component.isEmpty()) {
    value = -1;
  } else {
    qWarning("Unknown color component property : %s", component.toLocal8Bit().constData());
  }
  return value;
}

int KLFColorComponentsEditorBase::valueMax(const QString& component)
{
  if (component == "hue")
    return 359;
  else if (component == "sat" || component == "val" ||
	   component == "red" || component == "green" ||
	   component == "blue" || component == "alpha")
    return 255;
  else if (component == "fix" || component.isEmpty())
    return -1;

  qWarning("Unknown color component property : %s", component.toLocal8Bit().constData());
  return -1;
}

QColor KLFColorComponentsEditorBase::colorFromValues(QColor base, int a, int b)
{
  QColor col = base;
  /*  printf("colorFromValues(%s/alpha=%d, %d, %d): My components:(%s+%s);\n", cstr(col.name()),
      col.alpha(), a, b, cstr(_colorcomponent), cstr(_colorcomponent_b)); */
  if (_colorcomponent == "hue") {
    col.setHsv(a, col.saturation(), col.value());
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "sat") {
    col.setHsv(col.hue(), a, col.value());
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "val") {
    col.setHsv(col.hue(), col.saturation(), a);
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "red") {
    col.setRgb(a, col.green(), col.blue());
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "green") {
    col.setRgb(col.red(), a, col.blue());
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "blue") {
    col.setRgb(col.red(), col.green(), a);
    col.setAlpha(base.alpha());
  } else if (_colorcomponent == "alpha") {
    col.setAlpha(a);
  } else if (_colorcomponent == "fix") {
    // no change to col
  } else {
    qWarning("Unknown color component property : %s", _colorcomponent.toLocal8Bit().constData());
  }
  QColor base2 = col;
  //  printf("\tnew color is (%s/alpha=%d);\n", cstr(col.name()), col.alpha());
  if ( ! _colorcomponent_b.isEmpty() && _colorcomponent_b != "fix" ) {
    //    printf("\twe have a second component\n");
    if (_colorcomponent_b == "hue") {
      col.setHsv(b, col.saturation(), col.value());
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "sat") {
      col.setHsv(col.hue(), b, col.value());
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "val") {
      col.setHsv(col.hue(), col.saturation(), b);
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "red") {
      col.setRgb(b, col.green(), col.blue());
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "green") {
      col.setRgb(col.red(), b, col.blue());
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "blue") {
      col.setRgb(col.red(), col.blue(), b);
      col.setAlpha(base2.alpha());
    } else if (_colorcomponent_b == "alpha") {
      col.setAlpha(b);
    } else {
      qWarning("Unknown color component property : %s", _colorcomponent_b.toLocal8Bit().constData());
    }
  }
  //  printf("\tand color is finally %s/alpha=%d\n", cstr(col.name()), col.alpha());
  return col;
}
bool KLFColorComponentsEditorBase::refreshColorFromInternalValues(int a, int b)
{
  QColor oldcolor = _color;
  _color = colorFromValues(_color, a, b);
  /*  printf("My components:(%s+%s); New color is %s/alpha=%d\n", _colorcomponent.toLocal8Bit().constData(),
      _colorcomponent_b.toLocal8Bit().constData(),  _color.name().toLocal8Bit().constData(), _color.alpha()); */
  if ( oldcolor != _color )
    return true;
  return false;
}


// -------------------------------------------------------------------


KLFColorComponentSpinBox::KLFColorComponentSpinBox(QWidget *parent)
  : QSpinBox(parent)
{
  _color = Qt::black;

  setColorComponent("hue");
  setColor(_color);

  connect(this, SIGNAL(valueChanged(int)), this, SLOT(internalChanged(int)));

  setValue(valueAFromNewColor(_color));
}

void KLFColorComponentSpinBox::setColorComponent(const QString& comp)
{
  _colorcomponent = comp.toLower();
  setMinimum(0);
  setMaximum(valueAMax());
}

void KLFColorComponentSpinBox::internalChanged(int newvalue)
{
  if ( refreshColorFromInternalValues(newvalue) )
    emit colorChanged(_color);
}

void KLFColorComponentSpinBox::setColor(const QColor& color)
{
  int value = valueAFromNewColor(color);
  /*  printf("My components:(%s+%s); setColor(%s/alpha=%d); new value = %d\n",
      _colorcomponent.toLocal8Bit().constData(), _colorcomponent_b.toLocal8Bit().constData(),
      color.name().toLocal8Bit().constData(), color.alpha(), value); */
  _color = color;
  setValue(value); // will emit QSpinBox::valueChanged() --> internalChanged() --> colorChanged()
}


// -------------------------------------------------------------------


QList<QColor> KLFColorChooseWidget::_recentcolors;
QList<QColor> KLFColorChooseWidget::_standardcolors;
QList<QColor> KLFColorChooseWidget::_customcolors;


KLFColorChooseWidget::KLFColorChooseWidget(QWidget *parent)
  : QWidget(parent)
{
  setupUi(this);

  if (_standardcolors.size() == 0) {
    // add a few standard colors.
    QList<QRgb> rgbs;
    // inspired from the "Forty Colors" Palette in KDE3 color dialog
    rgbs << 0x000000 << 0x303030 << 0x585858 << 0x808080 << 0xa0a0a0 << 0xc3c3c3
	 << 0xdcdcdc << 0xffffff << 0x400000 << 0x800000 << 0xc00000 << 0xff0000
	 << 0xffc0c0 << 0x004000 << 0x008000 << 0x00c000 << 0x00ff00 << 0xc0ffc0
	 << 0x000040 << 0x000080 << 0x0000c0 << 0x0000ff << 0xc0c0ff << 0x404000
	 << 0x808000 << 0xc0c000 << 0xffff00 << 0xffffc0 << 0x004040 << 0x008080
	 << 0x00c0c0 << 0x00ffff << 0xc0ffff << 0x400040 << 0x800080 << 0xc000c0
	 << 0xff00ff << 0xffc0ff << 0xc05800 << 0xff8000 << 0xffa858 << 0xffdca8 ;
    for (int k = 0; k < rgbs.size(); ++k)
      _standardcolors.append(QColor(QRgb(rgbs[k])));
  }

  _connectedColorChoosers.append(mDisplayColor);
  _connectedColorChoosers.append(mHueSatPane);
  _connectedColorChoosers.append(mValPane);
  _connectedColorChoosers.append(mAlphaPane);
  _connectedColorChoosers.append(mColorTriangle);
  _connectedColorChoosers.append(mHueSlider);
  _connectedColorChoosers.append(mSatSlider);
  _connectedColorChoosers.append(mValSlider);
  _connectedColorChoosers.append(mRedSlider);
  _connectedColorChoosers.append(mGreenSlider);
  _connectedColorChoosers.append(mBlueSlider);
  _connectedColorChoosers.append(mAlphaSlider);
  _connectedColorChoosers.append(spnHue);
  _connectedColorChoosers.append(spnSat);
  _connectedColorChoosers.append(spnVal);
  _connectedColorChoosers.append(spnRed);
  _connectedColorChoosers.append(spnGreen);
  _connectedColorChoosers.append(spnBlue);
  _connectedColorChoosers.append(spnAlpha);

  KLFGridFlowLayout *lytRecent = new KLFGridFlowLayout(12, mRecentColorsPalette);
  lytRecent->setSpacing(2);
  KLFGridFlowLayout *lytStandard = new KLFGridFlowLayout(12, mStandardColorsPalette);
  lytStandard->setSpacing(2);
  KLFGridFlowLayout *lytCustom = new KLFGridFlowLayout(12, mCustomColorsPalette);
  lytCustom->setSpacing(2);

  updatePalettes();

  int k;
  for (k = 0; k < _connectedColorChoosers.size(); ++k) {
    connect(_connectedColorChoosers[k], SIGNAL(colorChanged(const QColor&)),
	    this, SLOT(internalColorChanged(const QColor&)));
  }

  connect(lstNames, SIGNAL(itemClicked(QListWidgetItem*)),
	  this, SLOT(internalColorNameSelected(QListWidgetItem*)));
  connect(txtHex, SIGNAL(textChanged(const QString&)),
	  this, SLOT(internalColorNameSet(const QString&)));

  connect(btnAddCustomColor, SIGNAL(clicked()),
	  this, SLOT(setCurrentToCustomColor()));

  QStringList colornames = QColor::colorNames();
  for (k = 0; k < colornames.size(); ++k) {
    QPixmap colsample(16, 16);
    colsample.fill(QColor(colornames[k]));
    new QListWidgetItem(QIcon(colsample), colornames[k], lstNames);
  }

  internalColorChanged(_color);
}

void KLFColorChooseWidget::internalColorChanged(const QColor& newcolor)
{
  int k;
  for (k = 0; k < _connectedColorChoosers.size(); ++k) {
    _connectedColorChoosers[k]->blockSignals(true);
    _connectedColorChoosers[k]->setProperty("color", QVariant(newcolor));
    _connectedColorChoosers[k]->blockSignals(false);
  }  
  txtHex->blockSignals(true);
  txtHex->setText(newcolor.name());
  txtHex->blockSignals(false);

  _color = newcolor;

  emit colorChanged(newcolor);
}

void KLFColorChooseWidget::internalColorNameSelected(QListWidgetItem *item)
{
  if (!item)
    return;
  QColor color(item->text());
  internalColorChanged(color);
}

void KLFColorChooseWidget::internalColorNameSet(const QString& name)
{
  QColor color(name);
  internalColorChanged(color);
}

void KLFColorChooseWidget::setColor(const QColor& color)
{
  if (color == _color)
    return;

  internalColorChanged(color);
}

void KLFColorChooseWidget::fillPalette(QList<QColor> colorlist, QWidget *w)
{
  int k;
  KLFGridFlowLayout *lyt = dynamic_cast<KLFGridFlowLayout*>( w->layout() );
  lyt->clearAll();
  for (k = 0; k < colorlist.size(); ++k) {
    KLFColorClickSquare *sq = new KLFColorClickSquare(colorlist[k], 12, w);
    connect(sq, SIGNAL(colorActivated(const QColor&)),
	    this, SLOT(internalColorChanged(const QColor&)));
    lyt->insertGridFlowWidget(sq);
    sq->show();
  }
}

void KLFColorChooseWidget::setCurrentToCustomColor()
{
  _customcolors.append(_color);
  updatePalettes();
}

void KLFColorChooseWidget::updatePalettes()
{
  fillPalette(_recentcolors, mRecentColorsPalette);
  fillPalette(_standardcolors, mStandardColorsPalette);
  fillPalette(_customcolors, mCustomColorsPalette);
}


// -------------------------------------------------------------------



void KLFColorList::addColor(const QColor& color)
{
  int i;
  if ( (i = list.indexOf(color)) >= 0 )
    list.removeAt(i);

  list.append(color);
  while (list.size() >= klfconfig.UI.maxUserColors)
    list.pop_front();

  emit listChanged();
}

// static
KLFColorList *KLFColorChooser::_colorlist = NULL;

KLFColorChooser::KLFColorChooser(QWidget *parent)
  : QPushButton(parent), _color(0,0,0,255), _pix(), _allowdefaultstate(false), _autoadd(true), _size(120, 20),
    _xalignfactor(0.5f), _yalignfactor(0.5f),
    mMenu(0)
{
  ensureColorListInstance();
  connect(_colorlist, SIGNAL(listChanged()), this, SLOT(_makemenu()));
  _makemenu();
  _setpix();
}


KLFColorChooser::~KLFColorChooser()
{
}


QColor KLFColorChooser::color() const
{
  return _color;
}

QSize KLFColorChooser::sizeHint() const
{
  // inspired by QPushButton::sizeHint() in qpushbutton.cpp

  ensurePolished();

  int w = 0, h = 0;
  QStyleOptionButton opt;
  initStyleOption(&opt);

  // calculate contents size...
  w = _pix.width()+4;
  h = _pix.height()+2;

  if (menu())
    w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);

  return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
	  expandedTo(QApplication::globalStrut()));
}

void KLFColorChooser::setColor(const QColor& col)
{
  if ( ! _allowdefaultstate && ! col.isValid() )
    return;

  _color = col;
  _setpix();

  if (_autoadd && _color.isValid()) {
    _colorlist->addColor(_color);
  }
}

void KLFColorChooser::setDefaultColor()
{
  setColor(QColor());
}

void KLFColorChooser::setAllowDefaultState(bool allow)
{
  _allowdefaultstate = allow;
  _makemenu();
}

void KLFColorChooser::requestColor()
{
  // prefer our own color selection dialog
  QColor col = KLFColorDialog::getColor(_color, this);
  // QColor col = QColorDialog::getColor(_color, this);
  if ( ! col.isValid() )
    return;

  setColor(col);
}

void KLFColorChooser::setSenderPropertyColor()
{
  QColor c = sender()->property("setColor").value<QColor>();
  setColor(c);
}

void KLFColorChooser::_makemenu()
{
  if (mMenu) {
    setMenu(0);
    mMenu->deleteLater();
  }

  QSize menuIconSize = QSize(16,16);

  mMenu = new QMenu(this);

  if (_allowdefaultstate) {
    mMenu->addAction(QIcon(colorPixmap(QColor(), menuIconSize)), tr("[ Default ]"), this, SLOT(setDefaultColor()));
    mMenu->addSeparator();
  }

  int n, k, nk;
  ensureColorListInstance();
  n = _colorlist->list.size();
  for (k = 0; k < n; ++k) {
    nk = n - k - 1;
    QAction *a = mMenu->addAction(QIcon(colorPixmap(_colorlist->list[nk], menuIconSize)), _colorlist->list[nk].name(),
				  this, SLOT(setSenderPropertyColor()));
    a->setProperty("setColor", QVariant(_colorlist->list[nk]));
  }
  if (k > 0)
    mMenu->addSeparator();

  mMenu->addAction(tr("Custom ..."), this, SLOT(requestColor()));

  setMenu(mMenu);
}

void KLFColorChooser::paintEvent(QPaintEvent *e)
{
  QPushButton::paintEvent(e);
  QPainter p(this);
  p.setClipRect(e->rect());
  p.drawPixmap(QPointF(_xalignfactor*(width()-_pix.width()), _yalignfactor*(height()-_pix.height())), _pix);
}

void KLFColorChooser::_setpix()
{
  //  if (_color.isValid()) {
  _pix = colorPixmap(_color, _size);
  // DON'T setIcon() because we draw ourselves ! see paintEvent() !
  //  setIconSize(_pix.size());
  //  setIcon(_pix);
  setText("");
  //  } else {
  //    _pix = QPixmap();
  //    setIcon(QIcon());
  //    setIconSize(QSize(0,0));
  //    setText("");
  //  }
}


QPixmap KLFColorChooser::colorPixmap(const QColor& color, const QSize& size)
{
  QPixmap pix = QPixmap(size);
  if (color.isValid()) {
    pix.fill(color);
  } else {
    // draw "transparent"-representing pixmap
    pix.fill(QColor(127,127,127,80));
    QPainter p(&pix);
    p.setPen(QPen(QColor(255,0,0), 2));
    p.drawLine(0,0,size.width(),size.height());
  }
  return pix;
}



// static
void KLFColorChooser::ensureColorListInstance()
{
  if ( _colorlist == 0 )
    _colorlist = new KLFColorList;
}
// static
void KLFColorChooser::setColorList(const QList<QColor>& colors)
{
  ensureColorListInstance();
  //  *_colorlist = colors;
  _colorlist->list = colors;
  _colorlist->notifyListChanged();
}

// static
QList<QColor> KLFColorChooser::colorList()
{
  ensureColorListInstance();
  QList<QColor> l = _colorlist->list;
  return l;
}

