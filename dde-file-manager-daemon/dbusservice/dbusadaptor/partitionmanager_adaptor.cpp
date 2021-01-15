/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i ../partman/partitionmanager.h -c PartitionManagerAdaptor -l PartitionManager -a dbusadaptor/partitionmanager_adaptor partitionmanager.xml
 *
 * qdbusxml2cpp is Copyright (C) 2016 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "dbusadaptor/partitionmanager_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class PartitionManagerAdaptor
 */

PartitionManagerAdaptor::PartitionManagerAdaptor(PartitionManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

PartitionManagerAdaptor::~PartitionManagerAdaptor()
{
    // destructor
}

bool PartitionManagerAdaptor::mkfs(const QString &path, const QString &fstype, const QString &label)
{
    // handle method call com.deepin.filemanager.daemon.PartitionManager.mkfs
    return parent()->mkfs(path, fstype, label);
}
