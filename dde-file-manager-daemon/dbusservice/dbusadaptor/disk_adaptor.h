/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i ../disk/diskmanager.h -c DiskAdaptor -l DiskManager -a ./dbusadaptor/disk_adaptor disk.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DISK_ADAPTOR_H
#define DISK_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "../disk/diskmanager.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.DiskManager
 */
class DiskAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.DiskManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.DiskManager\">\n"
"    <signal name=\"confirmed\">\n"
"      <arg direction=\"out\" type=\"b\" name=\"state\"/>\n"
"    </signal>\n"
"    <signal name=\"finished\">\n"
"      <arg direction=\"out\" type=\"i\" name=\"code\"/>\n"
"    </signal>\n"
"    <method name=\"changeDiskPassword\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"oldPwd\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"newPwd\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    DiskAdaptor(DiskManager *parent);
    virtual ~DiskAdaptor();

    inline DiskManager *parent() const
    { return static_cast<DiskManager *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void changeDiskPassword(const QString &oldPwd, const QString &newPwd);
Q_SIGNALS: // SIGNALS
    void confirmed(bool state);
    void finished(int code);
};

#endif
