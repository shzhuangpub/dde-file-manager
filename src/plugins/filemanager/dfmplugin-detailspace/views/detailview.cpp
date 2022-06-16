/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "detailview.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "services/filemanager/detailspace/detailspace_defines.h"

#include <dfm-framework/dpf.h>

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFileSystemModel>
#include <QTreeView>

Q_DECLARE_METATYPE(QString *)

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPDETAILSPACE_USE_NAMESPACE

static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPDETAILSPACE_NAMESPACE) };

DetailView::DetailView(QWidget *parent)
    : QFrame(parent)
{
    initInfoUI();
}

DetailView::~DetailView()
{
}

/*!
 * \brief               在最右文件信息窗口中追加新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::addCustomControl(QWidget *widget)
{
    if (widget) {
        QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
        insertCustomControl(vlayout->count() - 1, widget);
        return true;
    }
    return false;
}

/*!
 * \brief               在最右文件信息窗口中指定位置新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::insertCustomControl(int index, QWidget *widget)
{
    // final one is stretch
    index = index == -1 ? vLayout->count() - 1 : qMin(vLayout->count() - 1, index);

    if (widget) {
        widget->setParent(this);
        QFrame *frame = new QFrame(this);
        QPushButton *btn = new QPushButton(frame);
        btn->setEnabled(false);
        btn->setFixedHeight(1);
        QVBoxLayout *vlayout = new QVBoxLayout;
        vlayout->setMargin(0);
        vlayout->setSpacing(10);
        vlayout->addWidget(btn);
        vlayout->addWidget(widget);
        frame->setLayout(vlayout);

        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->insertWidget(index, frame, 0, Qt::AlignTop);

        QMargins cm = vlayout->contentsMargins();
        QRect rc = contentsRect();
        frame->setMaximumWidth(rc.width() - cm.left() - cm.right());

        expandList.append(frame);
        return true;
    }
    return false;
}

void DetailView::removeControl()
{
    for (QWidget *w : expandList) {
        expandList.removeOne(w);
        QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(expandFrame->layout());
        layout->removeWidget(w);
        delete w;
    }
}

void DetailView::setUrl(const QUrl &url, int widgetFilter)
{
    createHeadUI(url, widgetFilter);
    createBasicWidget(url, widgetFilter);
}

void DetailView::initInfoUI()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setAlignment(Qt::AlignTop);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    expandFrame = new QFrame(this);
    expandFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(expandFrame);

    vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(5, 0, 5, 0);
    vLayout->setSpacing(8);
    vLayout->addStretch();
    expandFrame->setLayout(vLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 30, 0, 0);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(scrollArea, Qt::AlignVCenter);
    this->setLayout(mainLayout);
}

void DetailView::createHeadUI(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DSB_FM_NAMESPACE::DetailFilterType::kIconView) {
        return;
    } else {
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
        if (info.isNull())
            return;

        if (iconLabel) {
            vLayout->removeWidget(iconLabel);
            delete iconLabel;
            iconLabel = nullptr;
        }

        iconLabel = new QLabel(this);
        iconLabel->setFixedSize(160, 160);
        QSize targetSize = iconLabel->size().scaled(iconLabel->width(), iconLabel->height(), Qt::KeepAspectRatio);

        auto findPluginIcon = [](const QUrl &url) -> QString {
            QString iconName;
            bool ok = dpfHookSequence->run(kCurrentEventSpace, "hook_DetailViewIcon", url, &iconName);
            if (ok && !iconName.isEmpty())
                return iconName;

            ok = dpfHookSequence->run(kCurrentEventSpace, "hook_DetailViewIcon", url, &iconName);
            if (ok && !iconName.isEmpty())
                return iconName;

            return QString();
        };

        // get icon from plugin
        const QString &iconName = findPluginIcon(info->url());
        if (!iconName.isEmpty())
            iconLabel->setPixmap(QIcon::fromTheme(iconName).pixmap(targetSize));
        else
            iconLabel->setPixmap(info->fileIcon().pixmap(targetSize));
        iconLabel->setAlignment(Qt::AlignCenter);

        vLayout->insertWidget(0, iconLabel, 0, Qt::AlignCenter);
    }
}

void DetailView::createBasicWidget(const QUrl &url, int widgetFilter)
{
    if (widgetFilter == DSB_FM_NAMESPACE::DetailFilterType::kBasicView) {
        return;
    } else {
        fileBaseInfoView = new FileBaseInfoView(this);
        fileBaseInfoView->setFileUrl(url);
        addCustomControl(fileBaseInfoView);
    }
}

void DetailView::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
}
