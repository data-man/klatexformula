/***************************************************************************
 *   file klflibview.h
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2010 by Philippe Faist
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

#ifndef KLFLIBVIEW_H
#define KLFLIBVIEW_H

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>

#include <klflib.h>


/** \brief Model for Item-Views displaying a library resource's contents
 *
 * The Model can morph into different forms, for simulating various common & useful
 * displays (chronological list (history), category/tags tree (archive), maybe icons
 * in the future, ...).
 *
 */
class KLFLibModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  KLFLibModel(KLFLibResourceEngine *resource, uint flavorFlags = LinearList|GroupSubCategories,
	      QObject *parent = NULL);
  virtual ~KLFLibModel();

  enum ItemKind { EntryKind, CategoryLabelKind };
  enum {
    ItemKindItemRole = Qt::UserRole+800,
    EntryContentsTypeItemRole,
    CategoryLabelItemRole,
    FullCategoryPathItemRole
  };

  /** For example use
   * \code
   *  model->data(index, KLFLibModel::entryItemRole(KLFLibEntry::Latex)).toString()
   * \endcode
   * to get LaTeX string for model index \c index.
   */
  static inline int entryItemRole(int propertyId) { return Qt::UserRole+810+propertyId; }
  
  virtual void setResource(KLFLibResourceEngine *resource);

  enum FlavorFlag {
    LinearList = 0x0001,
    CategoryTree = 0x0002,
    DisplayTypeMask = 0x000f,

    GroupSubCategories = 0x1000
  };
  /** sets the flavor flags given by \c flags. Only flags masked by \c modify_mask
   * are affected. Examples:
   * \code
   *  // Display type set to LinearList. GroupSubCategories is unchanged.
   *  m->setFlavorFlags(KLFLibModel::LinearList, KLFLibModel::DisplayTypeMask);
   *  // Set, and respectively unset the group sub-categories flag (no change to other flags)
   *  m->setFlavorFlags(KLFLibModel::GroupSubCategories, KLFLibModel::GroupSubCategories);
   *  m->setFlavorFlags(0, KLFLibModel::GroupSubCategories);
   * \endcode  */
  virtual void setFlavorFlags(uint flags, uint modify_mask = 0xffffffff);
  virtual uint flavorFlags() const;
  inline uint displayType() const { return flavorFlags() & DisplayTypeMask; }

  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual bool hasChildren(const QModelIndex &parent) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
			      int role = Qt::DisplayRole) const;
  virtual bool hasIndex(int row, int column,
			const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column,
			    const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &index) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

  virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:

  KLFLibResourceEngine *pResource;

  unsigned int pFlavorFlags;

  int entryContentsType(int column) const;

  typedef qint32 IndexType;
  struct NodeId {
    NodeId(ItemKind k = EntryKind, IndexType i = -1) : kind(k), index(i)  { }
    bool valid() const { return index >= 0; }
    ItemKind kind;
    IndexType index;
  };
  struct Node {
    Node() : parent(NodeId()), children(QList<NodeId>()) { }
    NodeId parent;
    QList<NodeId> children;
    virtual int nodeKind() const = 0;
  };
  struct EntryNode : public Node {
    EntryNode() : entryid(-1), entry() { }
    virtual int nodeKind() const { return EntryKind; }
    KLFLibResourceEngine::entryId entryid;
    KLFLibEntry entry;
  };
  struct CategoryLabelNode : public Node {
    CategoryLabelNode() : categoryLabel(), fullCategoryPath()  { }
    virtual int nodeKind() const { return CategoryLabelKind; }
    //! The last element in \ref fullCategoryPath eg. "General Relativity"
    QString categoryLabel;
    //! The full category path of this category eg. "Physics/General Relativity"
    QString fullCategoryPath;
  };
  /** QVector is used since the QModelIndex s uses pointer to individual
   * nodes */
  typedef QVector<EntryNode> EntryCache;
  /** QVector is used since the QModelIndex s uses pointer to individual
   * nodes */
  typedef QVector<CategoryLabelNode> CategoryLabelCache;

  EntryCache pEntryCache;
  CategoryLabelCache pCategoryLabelCache;

  /** If row is negative, it will be looked up automatically. */
  QModelIndex createIndexFromId(NodeId nodeid, int row, int column) const;
  /** If row is negative, it will be looked up automatically. */
  QModelIndex createIndexFromPtr(Node *node, int row, int column) const;
  Node * getNodeForIndex(const QModelIndex& index) const;
  Node * getNode(NodeId nodeid) const;
  NodeId getNodeId(Node *node) const;
  /** get the row of \c nodeid in its parent. if you already have a pointer to the 
   * \ref Node with id \c nodeid, pass it to \c node pointer (slight optimization).
   * Otherwise pass NULL pointer and getNodeRow() will figure out the pointer with
   * a call to getNode().  */
  int getNodeRow(NodeId nodeid, Node *node = NULL) const;

  void updateCacheSetupModel();

  IndexType cacheFindCategoryLabel(QString category, bool createIfNotExists = false);

  void dumpNodeTree(Node *node, int indent = 0) const;

  class KLFLibModelSorter
  {
  public:
    KLFLibModelSorter(KLFLibModel *m, int entry_prop, Qt::SortOrder sort_order, bool groupcategories)
      : model(m), entryProp(entry_prop), sortOrderFactor( (sort_order == Qt::AscendingOrder) ? -1 : 1),
	groupCategories(groupcategories) { }
    bool operator()(const NodeId& a, const NodeId& b);
  private:
    KLFLibModel *model;
    int entryProp;
    int sortOrderFactor;
    bool groupCategories;
  };
  QString nodeValue(Node *ptr, int entryProperty);
  void sortCategory(CategoryLabelNode *categoryPtr, int column, Qt::SortOrder order);

  int lastSortColumn;
  Qt::SortOrder lastSortOrder;

};



class KLF_EXPORT KLFLibViewDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  KLFLibViewDelegate(QObject *parent);
  virtual ~KLFLibViewDelegate();

  virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem& option,
				const QModelIndex& index) const;
  virtual bool editorEvent(QEvent *event,QAbstractItemModel *model, const QStyleOptionViewItem& option,
			   const QModelIndex& index);
  virtual void paint(QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setEditorData(QWidget *editor, const QModelIndex& index) const;
  virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem& option,
				    const QModelIndex& index) const;

protected:
  virtual void paintEntry(QPainter *painter, const QStyleOptionViewItem& option,
			  const QModelIndex& index) const;
  virtual void paintCategoryLabel(QPainter *painter, const QStyleOptionViewItem& option,
				  const QModelIndex& index) const;

private:
  /** Temporary variable paint() sets for subcalls to paintEntry() and paintCategoryLabel() */
  mutable QRect innerRectText, innerRectImage;
  /** Temporary variable paint() sets for subcalls to paintEntry() and paintCategoryLabel() */
  mutable bool isselected;
};



#endif
