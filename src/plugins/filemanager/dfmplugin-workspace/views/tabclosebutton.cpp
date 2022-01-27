/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "tabclosebutton.h"

#include <QIcon>
#include <QGraphicsSceneHoverEvent>

DPWORKSPACE_USE_NAMESPACE

TabCloseButton::TabCloseButton(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

int TabCloseButton::getClosingIndex()
{
    return closingIndex;
}

void TabCloseButton::setClosingIndex(int index)
{
    closingIndex = index;
}

void TabCloseButton::setActiveWidthTab(bool active)
{
    activeWidthTab = active;
    update();
}

QRectF TabCloseButton::boundingRect() const
{
    return QRectF(0, 0, 24, 24);
}

void TabCloseButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QIcon closeIcon = QIcon::fromTheme("window-close_round");

    QIcon::Mode md = QIcon::Mode::Disabled;
    if (mousePressed) {
        md = QIcon::Mode::Selected;
    }
    if (mouseHovered) {
        md = QIcon::Mode::Active;
    }

    QRect rc = boundingRect().toRect();
    closeIcon.paint(painter, rc, Qt::AlignCenter, md);
}

void TabCloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    mousePressed = true;
    if (mouseHovered)
        mouseHovered = false;
    update();
}

void TabCloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    mousePressed = false;
    emit clicked();
    update();
}

void TabCloseButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    mouseHovered = true;
    emit hovered(closingIndex);
    update();
}

void TabCloseButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    event->ignore();
    if (!mouseHovered)
        mouseHovered = true;
    update();
}

void TabCloseButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit unHovered(closingIndex);
    event->ignore();
    mouseHovered = false;
    update();
}