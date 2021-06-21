/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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


/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i accesscontrol/accesscontrolmanager.h -c AccessControlAdaptor -l AccessControlManager -a ./dbusadaptor/accesscontrol_adaptor accesscontrol.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "./dbusadaptor/accesscontrol_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class AccessControlAdaptor
 */

AccessControlAdaptor::AccessControlAdaptor(AccessControlManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

AccessControlAdaptor::~AccessControlAdaptor()
{
    // destructor
}

QVariantList AccessControlAdaptor::QueryAccessPolicy()
{
    // handle method call com.deepin.filemanager.daemon.AccessControlManager.QueryAccessPolicy
    return parent()->QueryAccessPolicy();
}

QString AccessControlAdaptor::SetAccessPolicy(const QVariantMap &policy)
{
    // handle method call com.deepin.filemanager.daemon.AccessControlManager.SetAccessPolicy
    return parent()->SetAccessPolicy(policy);
}
