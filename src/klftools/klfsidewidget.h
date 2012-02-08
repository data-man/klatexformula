/***************************************************************************
 *   file klfsidewidget.h
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2011 by Philippe Faist
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

#ifndef KLFSIDEWIDGET_H
#define KLFSIDEWIDGET_H


#include <QObject>
#include <QWidget>
#include <QPointer>

#include <klfdefs.h>
#include <klffactory.h>


class KLFSideWidgetManagerBasePrivate;
class KLF_EXPORT KLFSideWidgetManagerBase : public QObject
{
  Q_OBJECT
public:
  /**
   * \warning This function does NOT initialize its internal pParentWidget and pSideWidget members---they
   *   are always initialized to NULL. Reason: subclasses should set these in their constructors via
   *   setSideWidget() and setOurParentWidget(), which then call newSideWidgetSet() etc., which passes
   *   through the virtual call (it does not in the base class constructor). */
  KLFSideWidgetManagerBase(QWidget *parentWidget = NULL, QWidget *sideWidget = NULL,
			   bool requireSideWidgetParentConsistency = false, QObject *managerParent = NULL);
  virtual ~KLFSideWidgetManagerBase();

  inline QWidget * sideWidget() const { return pSideWidget; }
  inline QWidget * ourParentWidget() const { return pParentWidget; }

  void setSideWidget(QWidget *widget);
  void setOurParentWidget(QWidget *widget);

  virtual bool sideWidgetVisible() const = 0;

signals:
  /** \brief Emitted whenever the shown status of the side widget has changed.
   *
   * Subclasses must emit this signal in their showSideWidget() implementation. It is also up to the
   * subclass to emit it in their newSideWidgetSet() implementation with a consistent initial value.  */
  void sideWidgetShown(bool shown);

public slots:
  virtual void showSideWidget(bool show) = 0;
  void showSideWidget() { showSideWidget(true); }
  void hideSideWidget(bool hide = true) { showSideWidget(!hide); }
  void toggleSideWidget() { showSideWidget(!sideWidgetVisible()); }

  virtual void waitForShowHideActionFinished(int timeout_ms = 2000);

protected:
  virtual void newSideWidgetSet(QWidget *oldSideWidget, QWidget *newSideWidget)
  {  Q_UNUSED(oldSideWidget); Q_UNUSED(newSideWidget);  }
  virtual void newParentWidgetSet(QWidget *oldParentWidget, QWidget *newParentWidget)
  {  Q_UNUSED(oldParentWidget); Q_UNUSED(newParentWidget); }

  virtual bool showHideIsAnimating() { return false; }

private:

  QPointer<QWidget> pSideWidget;
  QPointer<QWidget> pParentWidget;

  KLF_DECLARE_PRIVATE(KLFSideWidgetManagerBase);
};


// -----


class KLFShowHideSideWidgetManagerPrivate;
class KLF_EXPORT KLFShowHideSideWidgetManager : public KLFSideWidgetManagerBase
{
  Q_OBJECT

  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation) ;
  Q_PROPERTY(int calcSpacing READ calcSpacing WRITE setCalcSpacing) ;
  KLF_PROPERTY_GET(Qt::Orientation orientation) ;
  KLF_PROPERTY_GET(int calcSpacing) ;
public:
  KLFShowHideSideWidgetManager(QWidget *parentWidget = NULL, QWidget *sideWidget = NULL,
			       QObject *managerParent = NULL);
  virtual ~KLFShowHideSideWidgetManager();

  virtual bool eventFilter(QObject *obj, QEvent *event);

  virtual bool sideWidgetVisible() const;

public slots:
  virtual void showSideWidget(bool show);
  void setOrientation(Qt::Orientation o);
  void setCalcSpacing(int cs);

protected:
  virtual bool event(QEvent *event);

  virtual void newSideWidgetSet(QWidget *oldSideWidget, QWidget *newSideWidget);
  virtual void newParentWidgetSet(QWidget *oldParentWidget, QWidget *newParentWidget);

private slots:
  void resizeParentWidget(const QSize& size);

private:
  KLF_DECLARE_PRIVATE(KLFShowHideSideWidgetManager);
};


// ------

class KLFContainerSideWidgetManagerPrivate;

class KLFContainerSideWidgetManager : public KLFSideWidgetManagerBase
{
  Q_OBJECT

public:
  KLFContainerSideWidgetManager(QWidget *parentWidget = NULL, QWidget *sideWidget = NULL,
				QObject *managerParent = NULL);
  virtual ~KLFContainerSideWidgetManager();

  virtual bool sideWidgetVisible() const;

  bool eventFilter(QObject *obj, QEvent *event);

public slots:
  virtual void showSideWidget(bool show);

protected:
  /** Must be called in subclasses' constructor */
  void init();

  /** Must be reimplemented to create the container widget */
  virtual QWidget * createContainerWidget(QWidget * pw) = 0;

  virtual QWidget * containerWidget() const;

  virtual void newSideWidgetSet(QWidget *oldSideWidget, QWidget *newSideWidget);
  virtual void newParentWidgetSet(QWidget *oldWidget, QWidget *newWidget);

private slots:
  void aWidgetDestroyed(QObject *);

private:
  KLF_DECLARE_PRIVATE(KLFContainerSideWidgetManager) ;
};


// ------

class KLFDrawerSideWidgetManagerPrivate;
class KLF_EXPORT KLFDrawerSideWidgetManager : public KLFContainerSideWidgetManager
{
  Q_OBJECT

  Q_PROPERTY(Qt::DockWidgetArea openEdge READ openEdge WRITE setOpenEdge) ;
  KLF_PROPERTY_GET(Qt::DockWidgetArea openEdge) ;
public:
  KLFDrawerSideWidgetManager(QWidget *parentWidget = NULL, QWidget *sideWidget = NULL,
			     QObject *managerParent = NULL);
  virtual ~KLFDrawerSideWidgetManager();

  virtual bool sideWidgetVisible() const;

public slots:
  virtual void showSideWidget(bool show);
  void setOpenEdge(Qt::DockWidgetArea edge);

protected:
  virtual QWidget * createContainerWidget(QWidget * pw);

  virtual void newSideWidgetSet(QWidget *oldSideWidget, QWidget *newSideWidget);
  virtual void newParentWidgetSet(QWidget *oldWidget, QWidget *newWidget);

  virtual bool showHideIsAnimating();

private:
  KLF_DECLARE_PRIVATE(KLFDrawerSideWidgetManager) ;
};


// -------

class KLFFloatSideWidgetManagerPrivate;
class KLF_EXPORT KLFFloatSideWidgetManager : public KLFContainerSideWidgetManager
{
  Q_OBJECT

  Q_PROPERTY(Qt::WindowFlags wflags READ wflags WRITE setWFlags) ;
  KLF_PROPERTY_GET(Qt::WindowFlags wflags) ;
public:
  KLFFloatSideWidgetManager(QWidget *parentWidget = NULL, QWidget *sideWidget = NULL,
			    QObject *managerParent = NULL);
  virtual ~KLFFloatSideWidgetManager();

  virtual bool sideWidgetVisible() const;

public slots:
  virtual void showSideWidget(bool show);
  void setWFlags(Qt::WindowFlags wflags);

protected:
  void newSideWidgetSet(QWidget *oldw, QWidget *w);

  virtual QWidget * createContainerWidget(QWidget * pw);

private:
  KLF_DECLARE_PRIVATE(KLFFloatSideWidgetManager) ;
};



// ------


/**
 * \note This class acts as base class for any factory of side-widget-managers, as well as
 *   a functional instantiable factory for the built-in types.
 *
 * \note When reimplementing this class, it is automatically registered upon instanciation by
 *   the KLFFactoryBase base class.
 */
class KLF_EXPORT KLFSideWidgetManagerFactory : public KLFFactoryBase
{
public:
  KLFSideWidgetManagerFactory();
  virtual ~KLFSideWidgetManagerFactory();

  virtual QStringList supportedTypes() const;
  /** \brief A human-readable title to display as label of given type, e.g. in combo box */
  virtual QString getTitleFor(const QString& type) const;
  virtual KLFSideWidgetManagerBase * createSideWidgetManager(const QString& type, QWidget *parentWidget,
							     QWidget *sideWidget, QObject *parent);

  static QStringList allSupportedTypes();
  static KLFSideWidgetManagerBase * findCreateSideWidgetManager(const QString& type, QWidget *parentWidget,
								QWidget *sideWidget, QObject *parent);
  static KLFSideWidgetManagerFactory * findFactoryFor(const QString& managertype);

private:
  static KLFFactoryManager pFactoryManager;
};




// -----------------------------



class KLFSideWidgetPrivate;
class KLF_EXPORT KLFSideWidget : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString sideWidgetManagerType READ sideWidgetManagerType WRITE setSideWidgetManager) ;
public:
  enum SideWidgetManager { ShowHide = 1, Float, Drawer } ;

  //  KLFSideWidget(SideWidgetManager mtype = Float, QWidget *parent = NULL);
  //  KLFSideWidget(const QString& mtype = QLatin1String("Float"), QWidget *parent = NULL);
  KLFSideWidget(QWidget *parent = NULL);
  virtual ~KLFSideWidget();

  bool sideWidgetVisible() const;

  QString sideWidgetManagerType() const;

  KLFSideWidgetManagerBase * sideWidgetManager();

signals:
  void sideWidgetShown(bool shown);
  void sideWidgetManagerTypeChanged(const QString& managerType);

public slots:
  void showSideWidget(bool show = true);
  void hideSideWidget(bool hide = true) { showSideWidget(!hide); }
  void toggleSideWidget() { showSideWidget(!sideWidgetVisible()); }

  void setSideWidgetManager(SideWidgetManager mtype);
  void setSideWidgetManager(const QString& mtype);

  void debug_unlock_qtdesigner();

private:

  KLF_DECLARE_PRIVATE(KLFSideWidget) ;

  friend class KLFSideWidgetDesPlugin;
  bool _inqtdesigner;
};




#endif