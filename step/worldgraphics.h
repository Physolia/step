/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef STEP_WORLDGRAPHICS_H
#define STEP_WORLDGRAPHICS_H

#include <stepcore/vector.h>
#include <QGraphicsItem>
#include <QRectF>
#include <QColor>

namespace StepCore {
    class Item;
    class Particle;
    class Spring;
    class MetaProperty;
}
class WorldModel;
class WorldScene;
class QEvent;

class WorldGraphicsItem: public QGraphicsItem {
public:
    WorldGraphicsItem(StepCore::Item* item, WorldModel* worldModel, QGraphicsItem* parent = 0);
    StepCore::Item* item() const { return _item; }
    QRectF boundingRect() const;
    //void advance(int phase) const;

    static bool createItem(const QString& className, WorldModel* worldModel,
                            WorldScene* scene, QEvent* e);

protected:
    virtual void mouseSetPos(const QPointF& pos);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

protected:
    StepCore::Item* _item;
    WorldModel* _worldModel;
    QRectF _boundingRect;
    bool   _isMouseOverItem;

    double currentViewScale() const;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void drawHandler(QPainter* painter, const StepCore::Vector2d& v);
    void drawArrow(QPainter* painter, const StepCore::Vector2d& v);

    static const QColor SELECTION_COLOR;
    static const int SELECTION_MARGIN = 4;
    static const int ARROW_STROKE = 6;
    static const int HANDLER_SIZE = 6;
};

class ArrowHandlerGraphicsItem: public WorldGraphicsItem {
public:
    ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    const StepCore::MetaProperty* _property;
};

#endif

