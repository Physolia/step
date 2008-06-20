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

#include "particlegraphics.h"

#include <stepcore/particle.h>

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>
#include <KSvgRenderer>
#include <KDebug>

ParticleGraphicsItem::ParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Particle*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    _boundingRect = QRectF(-RADIUS, -RADIUS, RADIUS*2, RADIUS*2);
    //_lastArrowRadius = -1;
    //_velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, worldScene, this,
    //               _item->metaObject()->property("velocity"));
    //_velocityHandler->setVisible(false);
    //scene()->addItem(_velocityHandler);
}

QPainterPath ParticleGraphicsItem::shape() const
{
    QPainterPath path;
    double radius = (RADIUS+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

#if 0
void ParticleGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //painter->setPen(QPen(Qt::green, 0));
    //painter->drawRect(boundingRect());

    _worldScene->worldRenderer()->svgRenderer()->
            render(painter, "Particle", QRectF(-RADIUS,-RADIUS,RADIUS*2,RADIUS*2));

    /*
    int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QColor color = QColor::fromRgba(particle()->color());
    if(isItemHighlighted()) color = highlightColor(color);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));

    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setPen(QPen(QColor::fromRgba(particle()->color()), 2*radius, Qt::SolidLine, Qt::RoundCap));
    painter->drawPoint(0,0);

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }

    if(_isMouseOverItem || _isSelected) {
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, particle()->velocity());
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, particle()->acceleration());
    }
    */
}
#endif

QString ParticleGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("Particle-%1x%2").arg(c.x()).arg(c.y());
}

QPixmap* ParticleGraphicsItem::paintPixmap()
{
    QPixmap* pixmap = new QPixmap(2*RADIUS, 2*RADIUS);
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    _worldScene->worldRenderer()->svgRenderer()->render(&painter, "Particle",
                        QRectF(pos() - pos().toPoint(), QSize(RADIUS*2,RADIUS*2)));
    painter.end();
    return pixmap;
}

void ParticleGraphicsItem::viewScaleChanged()
{
    /*
    prepareGeometryChange();

    double s = currentViewScale();
    _boundingRect = QRectF((-RADIUS-SELECTION_MARGIN)/s,  (-RADIUS-SELECTION_MARGIN)/s,
                            (RADIUS+SELECTION_MARGIN)*2/s,(RADIUS+SELECTION_MARGIN)*2/s);

    if(_isMouseOverItem || _isSelected) {
        if(_lastArrowRadius < 0) {
            double vnorm = particle()->velocity().norm();
            double anorm = particle()->acceleration().norm();
            _lastArrowRadius = qMax(vnorm, anorm) + ARROW_STROKE/s;
        }
        _boundingRect |= QRectF(-_lastArrowRadius, -_lastArrowRadius,
                                    2*_lastArrowRadius, 2*_lastArrowRadius);
    }
    */
    worldDataChanged(true);
}

void ParticleGraphicsItem::worldDataChanged(bool)
{
    /*
    if(_isMouseOverItem || _isSelected) {
        double vnorm = particle()->velocity().norm();
        double anorm = particle()->acceleration().norm();
        double arrowRadius = qMax(vnorm, anorm) + ARROW_STROKE/currentViewScale();
        if(arrowRadius > _lastArrowRadius || arrowRadius < _lastArrowRadius/2) {
            _lastArrowRadius = arrowRadius;
            viewScaleChanged();
        }
        update();
    }
    */
    setPos(_worldScene->vectorToPoint(particle()->position()));
}

void ParticleGraphicsItem::stateChanged()
{
    /*
    if(_isSelected) _velocityHandler->setVisible(true);
    else _velocityHandler->setVisible(false);
    if(!_isMouseOverItem && !_isSelected) _lastArrowRadius = -1;
    viewScaleChanged();
    update();
    */
}

