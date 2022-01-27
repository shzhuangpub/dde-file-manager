/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "deviceservice.h"
#include "private/defendercontroller.h"
#include "private/devicemonitorhandler.h"

#include "dfm-base/utils/universalutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"

#include <dfm-mount/base/dfmmountutils.h>
#include <dfm-mount/dfmblockmonitor.h>

#include <QtConcurrent>
#include <DDesktopServices>

#include <algorithm>

DWIDGET_USE_NAMESPACE
DSS_USE_NAMESPACE

using namespace GlobalServerDefines;

DeviceMonitorHandler::DeviceMonitorHandler(DeviceService *serv)
    : QObject(nullptr), service(serv)
{
}

/*!
 * \brief maintaining devices data
 */
void DeviceMonitorHandler::startMaintaining()
{
    initBlockDevicesData();
    initProtocolDevicesData();
}

/*!
 * \brief device monitor for block devices and protocol devices
 */
void DeviceMonitorHandler::startConnect()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    // connect block devices signal
    auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).objectCast<DFMMOUNT::DFMBlockMonitor>();
    if (blkMonitor) {
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveAdded, this, &DeviceMonitorHandler::onBlockDriveAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveRemoved, this, &DeviceMonitorHandler::onBlockDriveRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemAdded, this, &DeviceMonitorHandler::onFilesystemAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemRemoved, this, &DeviceMonitorHandler::onFilesystemRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceAdded, this, &DeviceMonitorHandler::onBlockDeviceAdded);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceRemoved, this, &DeviceMonitorHandler::onBlockDeviceRemoved);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountAdded, this, &DeviceMonitorHandler::onBlockDeviceMounted);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountRemoved, this, &DeviceMonitorHandler::onBlockDeviceUnmounted);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::propertyChanged, this, &DeviceMonitorHandler::onBlockDevicePropertyChanged);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockUnlocked, this, &DeviceMonitorHandler::onBlockDeviceUnlocked);
        connect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockLocked, this, &DeviceMonitorHandler::onBlockDeviceLocked);
    }

    auto protoMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::ProtocolDevice).objectCast<DFMMOUNT::DFMProtocolMonitor>();
    if (protoMonitor) {
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceAdded, this, &DeviceMonitorHandler::onProtocolDeviceAdded);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceRemoved, this, &DeviceMonitorHandler::onProtocolDeviceRemoved);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountAdded, this, &DeviceMonitorHandler::onProtocolDeviceMounted);
        connect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountRemoved, this, &DeviceMonitorHandler::onProtocolDeviceUnmounted);
    }

    // connect device size update worker
    connect(&sizeUpdateTimer, &QTimer::timeout, this, &DeviceMonitorHandler::onDeviceSizeUsedTimeout);
    sizeUpdateTimer.setInterval(kSizeUpdateInterval);
    sizeUpdateTimer.start();
}

/*!
 * \brief disconnect device monitor for block devices and protocol devices
 */
void DeviceMonitorHandler::stopConnect()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    // disconnect block devices signal
    auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice).objectCast<DFMMOUNT::DFMBlockMonitor>();
    if (blkMonitor) {
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveAdded, this, &DeviceMonitorHandler::onBlockDriveAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::driveRemoved, this, &DeviceMonitorHandler::onBlockDriveRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemAdded, this, &DeviceMonitorHandler::onFilesystemAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::fileSystemRemoved, this, &DeviceMonitorHandler::onFilesystemRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceAdded, this, &DeviceMonitorHandler::onBlockDeviceAdded);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::deviceRemoved, this, &DeviceMonitorHandler::onBlockDeviceRemoved);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountAdded, this, &DeviceMonitorHandler::onBlockDeviceMounted);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::mountRemoved, this, &DeviceMonitorHandler::onBlockDeviceUnmounted);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::propertyChanged, this, &DeviceMonitorHandler::onBlockDevicePropertyChanged);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockUnlocked, this, &DeviceMonitorHandler::onBlockDeviceUnlocked);
        disconnect(blkMonitor.data(), &DFMMOUNT::DFMBlockMonitor::blockLocked, this, &DeviceMonitorHandler::onBlockDeviceLocked);
    }

    auto protoMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::ProtocolDevice).objectCast<DFMMOUNT::DFMProtocolMonitor>();
    if (protoMonitor) {
        disconnect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceAdded, this, &DeviceMonitorHandler::onProtocolDeviceAdded);
        disconnect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::deviceRemoved, this, &DeviceMonitorHandler::onProtocolDeviceRemoved);
        disconnect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountAdded, this, &DeviceMonitorHandler::onProtocolDeviceMounted);
        disconnect(protoMonitor.data(), &DFMMOUNT::DFMProtocolMonitor::mountRemoved, this, &DeviceMonitorHandler::onProtocolDeviceUnmounted);
    }
}

void DeviceMonitorHandler::initBlockDevicesData()
{
    auto &&blkPtrList = DeviceServiceHelper::createAllBlockDevices();
    for (auto &&blk : blkPtrList)
        insertNewBlockDeviceData(blk);
}

void DeviceMonitorHandler::initProtocolDevicesData()
{
    auto &&protoPtrList = DeviceServiceHelper::createAllProtocolDevices();
    for (auto &&dev : protoPtrList)
        insertNewProtocolDeviceData(dev);
}

bool DeviceMonitorHandler::insertNewBlockDeviceData(const DeviceServiceHelper::BlockDevPtr &ptr)
{
    QString &&id = ptr->path();

    if (id.isEmpty())
        return false;

    BlockDeviceData data;
    DeviceServiceHelper::makeBlockDeviceData(ptr, &data);
    QMutexLocker guard(&mutexForBlock);
    allBlockDevData.insert(id, data);
    return true;
}

bool DeviceMonitorHandler::insertNewProtocolDeviceData(const DeviceServiceHelper::ProtocolDevPtr &ptr)
{
    auto &&id = ptr->path();
    if (id.isEmpty())
        return false;

    ProtocolDeviceData data;
    DeviceServiceHelper::makeProtocolDeviceData(ptr, &data);
    QMutexLocker guard(&mutexForProtocol);
    allProtocolDevData.insert(id, data);
    return true;
}

void DeviceMonitorHandler::removeBlockDeviceData(const QString &deviceId)
{
    QMutexLocker guard(&mutexForBlock);
    if (allBlockDevData.contains(deviceId))
        allBlockDevData.remove(deviceId);
}

void DeviceMonitorHandler::removeProtocolDeviceData(const QString &deviceId)
{
    QMutexLocker guard(&mutexForProtocol);
    if (allProtocolDevData.contains(deviceId))
        allProtocolDevData.remove(deviceId);
}

void DeviceMonitorHandler::updateDataWithOpticalInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&opticalFlag = DFMMOUNT::Property::DriveOptical;
    if (data->opticalDrive && changes.contains(opticalFlag)) {
        if (changes.value(opticalFlag).toBool()) {   // disc inserted
            data->optical = changes.value(opticalFlag).toBool();

            auto &&driveMediaFlag = DFMMOUNT::Property::DriveMedia;
            if (changes.contains(driveMediaFlag))
                data->media = changes.value(driveMediaFlag).toString();

            auto &&driveMediaAvailable = DFMMOUNT::Property::DriveMediaAvailable;
            if (changes.contains(driveMediaAvailable))
                data->mediaAvailable = changes.value(driveMediaAvailable).toBool();

            auto &&partitionSizeFlag = DFMMOUNT::Property::PartitionSize;
            if (changes.contains(partitionSizeFlag)) {
                data->common.sizeTotal = changes.value(partitionSizeFlag).toLongLong();
                data->common.sizeFree = 0;
                data->common.sizeUsed = data->common.sizeTotal;
            }

            auto &&driveOpticalBlank = DFMMOUNT::Property::DriveOpticalBlank;
            if (changes.contains(driveOpticalBlank))
                data->opticalBlank = changes.value(driveOpticalBlank).toBool();
        } else {   // disc ejected, clear all property associated with optical
            data->common.filesystem.clear();
            data->common.sizeUsed = 0;
            data->common.sizeFree = 0;
            data->common.sizeTotal = 0;
            data->common.mountpoint.clear();

            data->mountpoints.clear();
            data->fsVersion.clear();
            data->hasFileSystem = false;
            data->hasPartition = false;
            data->idLabel.clear();
            data->media.clear();
            data->mediaAvailable = false;
            data->optical = false;
            data->opticalBlank = false;
            data->uuid.clear();
        }
    }
}

void DeviceMonitorHandler::updateDataWithMountedInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&idTypeFlag = DFMMOUNT::Property::BlockIDType;
    if (changes.contains(idTypeFlag)) {
        data->common.filesystem = changes.value(idTypeFlag).toString();
        data->hasFileSystem = !(data->common.filesystem.isEmpty());
    }

    //    auto &&idUsageFlag = DFMMOUNT::Property::BlockIDUsage;
    //    if (changes.contains(idUsageFlag))

    auto &&idUUIDFlag = DFMMOUNT::Property::BlockIDUUID;
    if (changes.contains(idUUIDFlag))
        data->uuid = changes.value(idUUIDFlag).toString();

    auto &&idVersionFlag = DFMMOUNT::Property::BlockIDVersion;
    if (changes.contains(idVersionFlag))
        data->fsVersion = changes.value(idVersionFlag).toString();

    auto &&mptFlag = DFMMOUNT::Property::FileSystemMountPoint;
    if (changes.contains(mptFlag)) {
        data->mountpoints = changes.value(mptFlag).toStringList();
        if (data->mountpoints.isEmpty())
            data->common.mountpoint.clear();
        else {
            data->common.mountpoint = data->mountpoints.first();

            if (!data->opticalDrive) {
                QStorageInfo sizeInfo(data->common.mountpoint);
                data->common.sizeUsed = data->common.sizeTotal - sizeInfo.bytesAvailable();
                data->common.sizeFree = sizeInfo.bytesAvailable();
            }
        }
    }
}

void DeviceMonitorHandler::updateDataWithOtherInfo(BlockDeviceData *data, const QMap<dfmmount::Property, QVariant> &changes)
{
    auto &&idLabelFlag = DFMMOUNT::Property::BlockIDLabel;
    auto &&hintIgnoreFlag = DFMMOUNT::Property::BlockHintIgnore;
    auto &&hintSystemFlag = DFMMOUNT::Property::BlockHintSystem;
    auto &&clearTextFalg = DFMMOUNT::Property::EncryptedCleartextDevice;

    // idlable
    if (changes.contains(idLabelFlag)) {
        QString &&idlabel = changes.value(idLabelFlag).toString();
        data->idLabel = idlabel;
    }

    // hintIgnore
    if (changes.contains(hintIgnoreFlag))
        data->hintIgnore = changes.value(hintIgnoreFlag).toBool();

    // hintSystem
    if (changes.contains(hintSystemFlag))
        data->hintSystem = changes.value(hintSystemFlag).toBool();

    // clearText
    if (changes.contains(clearTextFalg))
        data->cleartextDevice = changes.value(clearTextFalg).toString();

    // TODO(zhangs): handle other Property...
}

void DeviceMonitorHandler::handleBlockDevicesSizeUsedChanged()
{
    qDebug() << "Start check block devices size used changed";
    QList<BlockDeviceData> changedDataGroup;
    QMutexLocker guard(&mutexForBlock);
    auto &&keys = allBlockDevData.keys();
    for (const auto &key : keys) {
        auto &val = allBlockDevData[key];
        if (!DeviceServiceHelper::isIgnorableBlockDevice(val) || val.cryptoBackingDevice.length() > 1) {   // need to report the size change of unlocked device
            if (val.optical)
                continue;
            if (val.mountpoints.isEmpty())
                continue;

            const QString &id = val.common.id;
            const QString &mpt = val.common.mountpoint;
            qint64 sizeUsed = val.common.sizeUsed;

            QStorageInfo info(mpt);
            qint64 curSizeUsed = val.common.sizeTotal - info.bytesAvailable();

            if (curSizeUsed != sizeUsed) {
                qInfo() << "Block:" << id << "old size: " << sizeUsed << "new size: " << curSizeUsed;
                DeviceServiceHelper::updateBlockDeviceSizeUsed(&val, val.common.sizeTotal, info.bytesAvailable());
                changedDataGroup.push_back(val);
            }
        }
    }
    guard.unlock();

    for (auto iter = changedDataGroup.cbegin(); iter != changedDataGroup.cend(); ++iter) {
        if (Q_LIKELY(!iter->common.id.isEmpty()))
            emit service->deviceSizeUsedChanged(iter->common.id, iter->common.sizeTotal, iter->common.sizeFree);
    }
}

/*!
 * \brief DeviceMonitorHandler::handleProtolDevicesSizeUsedChanged run in thread
 */
void DeviceMonitorHandler::handleProtolDevicesSizeUsedChanged()
{
    // TODO(xust): this function is run in thread, not sure the gvfs returns correct datas in thread...
    qDebug() << "Start check protocol devices size used changed";
    QList<ProtocolDeviceData> changedDataGroup;
    QMutexLocker guard(&mutexForProtocol);
    auto &&keys = allProtocolDevData.keys();
    for (const auto &key : keys) {
        auto dev = DeviceServiceHelper::createProtocolDevice(key);
        if (dev) {
            qint64 total = dev->sizeTotal();
            qint64 free = dev->sizeFree();
            qint64 usage = dev->sizeUsage();

            auto &devData = allProtocolDevData[key];
            const qint64 &oldTotal = devData.common.sizeTotal;
            const qint64 &oldFree = devData.common.sizeFree;
            const qint64 &oldUsage = devData.common.sizeUsed;

            if (total != oldTotal
                || free != oldFree
                || usage != oldUsage) {
                qInfo() << "Protocol[new/old]: " << key
                        << QString("total: %1/%2, ").arg(total).arg(oldTotal)
                        << QString("usage: %1/%2, ").arg(usage).arg(oldUsage)
                        << QString("free: %1/%2, ").arg(free).arg(oldFree);
                DeviceServiceHelper::updateProtocolDeviceSizeUsed(&devData, total, free, usage);
                changedDataGroup.push_back(devData);
            }
        }
    }
    guard.unlock();

    for (auto iter = changedDataGroup.cbegin(); iter != changedDataGroup.cend(); ++iter) {
        if (Q_LIKELY(!iter->common.id.isEmpty()))
            emit service->deviceSizeUsedChanged(iter->common.id, iter->common.sizeTotal, iter->common.sizeFree);
    }
}

void DeviceMonitorHandler::onBlockDriveAdded(const QString &drvObjPath)
{
    qInfo() << "A block dirve added: " << drvObjPath;
    emit service->blockDriveAdded();
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceAdded);
}

void DeviceMonitorHandler::onBlockDriveRemoved(const QString &drvObjPath)
{
    qInfo() << "A block dirve removed: " << drvObjPath;
    emit service->blockDriveRemoved();
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_DeviceRemoved);
}

/*!
 * \brief mount block device and open url if isAutoMountAndOpenSetting is true
 * \param dev
 */
void DeviceMonitorHandler::onBlockDeviceAdded(const QString &deviceId)
{
    qInfo() << "A block device added: " << deviceId;
    auto blkDev = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!blkDev) {
        qWarning() << "Dev NULL!";
        return;
    }

    if (!insertNewBlockDeviceData(blkDev))
        return;

    emit service->blockDevAdded(deviceId);
    // maybe reload setting ?
    if (service->isInLiveSystem() || !service->isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << service->isInLiveSystem()
                   << "auto mount setting: " << service->isAutoMountSetting();
        return;
    }

    QString &&loginState = dfmbase::UniversalUtils::userLoginState();
    if (loginState != "active") {
        qWarning() << "Cancel mount, user login state is" << loginState;
        return;
    }

    bool isUnlockedDevice = blkDev->getProperty(DFMMOUNT::Property::BlockCryptoBackingDevice).toString().length() > 1;
    if (isUnlockedDevice) {
        qDebug() << "No auto mount for unlocked device: " << blkDev->path();
        return;
    }

    if (service->mountBlockDevice(deviceId, {}).isEmpty()) {
        qWarning() << "Mount device failed: " << blkDev->path();
        return;
    }

    if (service->isAutoMountAndOpenSetting())
        DeviceServiceHelper::openFileManagerToDevice(blkDev);
}

void DeviceMonitorHandler::onBlockDeviceRemoved(const QString &deviceId)
{
    qInfo() << "A block device removed: " << deviceId;
    removeBlockDeviceData(deviceId);
    emit service->blockDevRemoved(deviceId);
}

void DeviceMonitorHandler::onFilesystemAdded(const QString &deviceId)
{
    qInfo() << "A block device fs added: " << deviceId;
    emit service->blockDevFilesystemAdded(deviceId);
    emit service->blockDevicePropertyChanged(deviceId, DeviceProperty::kHasFileSystem, true);
}

void DeviceMonitorHandler::onFilesystemRemoved(const QString &deviceId)
{
    qInfo() << "A block device fs removed: " << deviceId;
    emit service->blockDevFilesystemRemoved(deviceId);
    emit service->blockDevicePropertyChanged(deviceId, DeviceProperty::kHasFileSystem, false);
}

void DeviceMonitorHandler::onBlockDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A block device mounted: " << deviceId;
    emit service->blockDevMounted(deviceId, mountPoint);
}

void DeviceMonitorHandler::onBlockDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A block device unmounted: " << deviceId;
    emit service->blockDevUnmounted(deviceId);
}

void DeviceMonitorHandler::onBlockDevicePropertyChanged(const QString &deviceId,
                                                        const QMap<dfmmount::Property, QVariant> &changes)
{
    QMutexLocker guard(&mutexForBlock);
    if (allBlockDevData.contains(deviceId)) {
        updateDataWithOpticalInfo(&allBlockDevData[deviceId], changes);
        updateDataWithMountedInfo(&allBlockDevData[deviceId], changes);
        updateDataWithOtherInfo(&allBlockDevData[deviceId], changes);
    }
    guard.unlock();
    QList<dfmmount::Property> &&keys = changes.keys();
    for (dfmmount::Property k : keys) {
        QString propertyName { dfmmount::Utils::getNameByProperty(k) };
        if (Q_LIKELY(!propertyName.isEmpty())) {
            emit service->blockDevicePropertyChanged(deviceId, propertyName, changes.value(k));
            qInfo() << "Block Device: " << deviceId << "property: " << propertyName
                    << "changed!"
                    << "New value is:" << changes.value(k);
        }
    }
}

void DeviceMonitorHandler::onBlockDeviceUnlocked(const QString &deviceId, const QString &clearDeviceId)
{
    emit service->blockDevUnlocked(deviceId, clearDeviceId);
}

void DeviceMonitorHandler::onBlockDeviceLocked(const QString &deviceId)
{
    emit service->blockDevLocked(deviceId);
}

void DeviceMonitorHandler::onDeviceSizeUsedTimeout()
{
    QtConcurrent::run([this]() {
        handleBlockDevicesSizeUsedChanged();
        handleProtolDevicesSizeUsedChanged();
    });
}

void DeviceMonitorHandler::onProtocolDeviceAdded(const QString &deviceId)
{
    auto protoDev = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        qWarning() << "protocol device is null!!";
        return;
    }
    qInfo() << "A new protocol device is added: " << protoDev->displayName() << deviceId;

    if (!insertNewProtocolDeviceData(protoDev))
        return;

    emit service->protocolDevAdded(deviceId);

    if (service->isInLiveSystem() || !service->isAutoMountSetting()) {
        qWarning() << "Cancel mount, live system: " << service->isInLiveSystem()
                   << "auto mount setting: " << service->isAutoMountSetting();
        return;
    }

    QString &&loginState = dfmbase::UniversalUtils::userLoginState();
    if (loginState != "active") {
        qWarning() << "Cancel mount, user login state is" << loginState;
        return;
    }

    // TODO(xust) do mount volume here
    if (service->mountProtocolDevice(deviceId, {}).isEmpty()) {
        qWarning() << "Mount device failed: " << protoDev->displayName();
        return;
    }
}

void DeviceMonitorHandler::onProtocolDeviceRemoved(const QString &deviceId)
{
    qInfo() << "A new protocol device is removed: " << deviceId;
    removeProtocolDeviceData(deviceId);
}

void DeviceMonitorHandler::onProtocolDeviceMounted(const QString &deviceId, const QString &mountPoint)
{
    qInfo() << "A protocol device is mounted at: " << mountPoint;
    auto protoDev = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        qWarning() << deviceId << "constructed a null device!!!";
        return;
    }

    // if it already existed in cache, then update it.
    insertNewProtocolDeviceData(protoDev);
    emit service->protocolDevMounted(deviceId, mountPoint);
}

void DeviceMonitorHandler::onProtocolDeviceUnmounted(const QString &deviceId)
{
    qInfo() << "A protocol device in unmounted: " << deviceId;
    auto protoDev = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!protoDev) {
        removeProtocolDeviceData(deviceId);
    } else {
        insertNewProtocolDeviceData(protoDev);
    }

    emit service->protocolDevUnmounted(deviceId);
}

/*!
 * \class DeviceService
 *
 * \brief DeviceService provides a series of interfaces for
 * external device operations and signals for device monitoring,
 * such as mounting, unmounting, ejecting, etc.
 */

DeviceService::DeviceService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<DeviceService>()
{
    monitorHandler.reset(new DeviceMonitorHandler(this));
}

DeviceService::~DeviceService()
{
    stopMonitor();
}

/*!
 * \brief auto mount block devices and protocol devices
 * !!! Note: call once
 */
void DeviceService::startAutoMount()
{
    std::call_once(DeviceServiceHelper::autoMountOnceFlag(), [this]() {
        qInfo() << "Start auto mount";
        if (isInLiveSystem()) {
            qWarning() << "Cannot auto mount, in Live System";
            return;
        }

        if (!isAutoMountSetting()) {
            qWarning() << "Cannot auto mount, AutoMount setting is false";
            return;
        }

        QStringList &&blkList = blockDevicesIdList({ { ListOpt::kMountable, true } });
        for (const QString &id : blkList)
            mountBlockDeviceAsync(id, { { "auth.no_user_interaction", true } });

        QStringList &&protocolList = protocolDevicesIdList();
        for (const QString &id : protocolList)
            mountProtocolDeviceAsync(id, {});

        qInfo() << "End auto mount";
    });
}

bool DeviceService::startMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    monitorHandler->startConnect();
    monitorHandler->startMaintaining();
    bool ret = manager->startMonitorWatch();
    return ret;
}

bool DeviceService::stopMonitor()
{
    auto manager = DFMMOUNT::DFMDeviceManager::instance();
    monitorHandler->stopConnect();
    bool ret = manager->stopMonitorWatch();
    return ret;
}

/*!
 * \brief async eject a block device
 * \param deviceId
 */
void DeviceService::ejectBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isEjectableBlockDevice(ptr)) {
        ptr->ejectAsync(opts, [this, deviceId](bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Eject failed: " << int(err);
                emit blockDevAsyncEjected(deviceId, false);
            } else {
                emit blockDevAsyncEjected(deviceId, true);
            }
        });
    }
}

bool DeviceService::ejectBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isEjectableBlockDevice(ptr))
        return ptr->eject(opts);

    return false;
}

void DeviceService::poweroffBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isCanPoweroffBlockDevice(ptr)) {
        ptr->powerOffAsync(opts, [this, deviceId](bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Poweroff failed: " << int(err);
                emit blockDevAsyncPoweroffed(deviceId, false);
            } else {
                emit blockDevAsyncPoweroffed(deviceId, true);
            }
        });
    }
}

bool DeviceService::poweroffBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (DeviceServiceHelper::isCanPoweroffBlockDevice(ptr))
        return ptr->powerOff(opts);

    return false;
}

bool DeviceService::renameBlockDevice(const QString &deviceId, const QString &newName, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr->mountPoints().isEmpty()) {
        qWarning() << "cannot rename a mounted device!";
        return false;
    }
    return ptr->rename(newName, opts);
}

void DeviceService::renameBlockDeviceAsync(const QString &deviceId, const QString &newName, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr->mountPoints().isEmpty()) {
        qWarning() << "cannot rename a mounted device!";
        return;
    }
    ptr->renameAsync(newName, opts, [=](bool ret, DFMMOUNT::DeviceError err) {
        if (err != DFMMOUNT::DeviceError::NoError)
            qWarning() << "an error occured while renaming a device: " << deviceId << DFMMOUNT::Utils::errorMessage(err);
        emit blockDevAsyncRenamed(deviceId, ret);
    });
}

QString DeviceService::unlockBlockDevice(const QString &passwd, const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "block: cannot create device handler by " << deviceId;
        return "";
    }

    if (ptr->isEncrypted()) {
        QString unlockdedObjPath;
        auto ok = ptr->unlock(passwd, unlockdedObjPath, opts);
        if (!ok) {
            qDebug() << "block: unlock device failed: " << deviceId
                     << dfmmount::Utils::errorMessage(ptr->lastError());
            return "";
        } else {
            return unlockdedObjPath;
        }
    }
    qWarning() << "block: device is not encrypted: " << deviceId;
    return "";
}

void DeviceService::unlockBlockDeviceAsync(const QString &passwd, const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "block: cannot create device handler by " << deviceId;
        return;
    }

    if (ptr->isEncrypted()) {
        ptr->unlockAsync(passwd, opts, [this, deviceId](bool result, dfmmount::DeviceError err, QString ret) {
            if (!result)
                qDebug() << "unlock" << deviceId << "failed: " << dfmmount::Utils::errorMessage(err);
            emit blockDevAsyncUnlocked(deviceId, ret, result);
        });
    } else {
        qDebug() << "device " << deviceId << "is not encrypted device";
    }
}

/*!
 * \brief DeviceService::preLockBlock do something before lock, such as check if encrypted, unmount..
 * \param deviceId
 * \return true if device can be locked otherwise false
 */
bool DeviceService::preLockBlock(const QString &deviceId)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "block: cannot create device handler by " << deviceId;
        return false;
    }

    // 1. get clear text block
    auto cleartextPath = ptr->getProperty(dfmmount::Property::EncryptedCleartextDevice).toString();
    if (cleartextPath == "/") {
        qDebug() << "block: device is not encrypted: " << deviceId;
        return false;
    }

    // 2. check if clear block is mounted
    auto clearBlk = DeviceServiceHelper::createBlockDevice(cleartextPath);
    if (!clearBlk) {
        qDebug() << "block: cannot create clear text block device" << cleartextPath;
        return false;
    }
    if (!clearBlk->mountPoints().isEmpty()) {
        auto ret = clearBlk->unmount({});
        if (!ret) {
            qDebug() << "block: unmount clear text block failed: " << dfmmount::Utils::errorMessage(clearBlk->lastError());
            return false;
        }
    }
    return true;
}

bool DeviceService::lockBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    if (!preLockBlock(deviceId)) {
        qDebug() << "block: pre lock device failed: " << deviceId;
        return false;
    }

    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "block: cannot create device handler by " << deviceId;
        return false;
    }

    auto ok = ptr->lock(opts);
    if (!ok) {
        qDebug() << "block: lock device failed: " << deviceId
                 << dfmmount::Utils::errorMessage(ptr->lastError());
    }
    return ok;
}

void DeviceService::lockBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    if (!preLockBlock(deviceId)) {
        qDebug() << "block: pre lock device failed: " << deviceId;
        return;
    }

    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "block: cannot create device handler by " << deviceId;
        return;
    }

    if (!ptr->isEncrypted()) {
        qDebug() << "device" << deviceId << "is not encrypted";
        return;
    }

    ptr->lockAsync(opts, [this, deviceId](bool result, dfmmount::DeviceError err) {
        if (!result)
            qDebug() << "lock" << deviceId << "failed: " << dfmmount::Utils::errorMessage(err);
        emit blockDevAsyncLocked(deviceId, result);
    });
}

QString DeviceService::mountProtocolDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "Null protocol pointer" << deviceId;
        return "";
    }

    return ptr->mount(opts);
}

void DeviceService::mountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "Null protocol pointer" << deviceId;
        return;
    }

    ptr->mountAsync(opts, [this, deviceId](bool ret, dfmmount::DeviceError err) {
        emit protocolDevAsyncMounted(deviceId, ret);
        if (!ret) {
            qDebug() << "mount protocol device: " << deviceId << "failed. " << dfmmount::Utils::errorMessage(err);
        }
    });
}

bool DeviceService::unmountProtocolDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "Null protocol pointer" << deviceId;
        return false;
    }

    return ptr->unmount(opts);
}

void DeviceService::unmountProtocolDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createProtocolDevice(deviceId);
    if (!ptr) {
        qWarning() << "Null protocol pointer" << deviceId;
        return;
    }

    ptr->unmountAsync(opts, [this, deviceId](bool ret, dfmmount::DeviceError err) {
        emit protocolDevAsyncUnmounted(deviceId, ret);
        if (!ret) {
            qDebug() << "unmount protocol device: " << deviceId << "failed. " << dfmmount::Utils::errorMessage(err);
        }
    });
}

/*!
 * \brief DeviceService::mountNetworkDevice
 * \param address       urls like "smb://1.2.3.4/share-folder/"
 * \param anonymous     if anonymous mount is expected
 * \param opts      the mount params: user(str), domain(str), passwd(str), savePasswd(0 for never, 1 for save before logout, 2 for save permanently).
 *  "user": "Test", "domain": WORKGROUP", "passwd": "123", "savePasswd": 0
 */
void DeviceService::mountNetworkDevice(const QString &address, bool anonymous, const QVariantMap &opts)
{
    Q_ASSERT_X(!address.isEmpty(), "DeviceService", "address is emtpy");
    if (address.isEmpty())
        return;

    dfmmount::MountPassInfo info;
    using namespace NetworkMountParamKey;
    if (anonymous) {
        info.anonymous = true;
    } else if (opts.contains(kUser) && opts.contains(kDomain) && opts.contains(kPasswd)) {
        auto user = opts.value(kUser).toString();
        auto domain = opts.value(kDomain).toString();
        auto passwd = opts.value(kPasswd).toString();

        using namespace dfmmount;
        NetworkMountPasswdSaveMode mode { NetworkMountPasswdSaveMode::NeverSavePasswd };
        if (opts.contains(kPasswdSaveMode)) {
            mode = NetworkMountPasswdSaveMode(opts.value(kPasswdSaveMode).toInt());
            if (mode < NetworkMountPasswdSaveMode::NeverSavePasswd
                || mode > NetworkMountPasswdSaveMode::SavePermanently) {
                qDebug() << "password save mode is not valid, reset to zero";
                mode = NetworkMountPasswdSaveMode::NeverSavePasswd;
            }
        }

        info.domain = domain;
        info.userName = user;
        info.passwd = passwd;
        info.savePasswd = mode;
    } else {
        qDebug() << "param is not valid";
        return;
    }

    auto whenAskPasswd = [info](QString, QString, QString) { return info; };
    dfmmount::DFMProtocolDevice::mountNetworkDevice(address, whenAskPasswd, [address](bool ok, dfmmount::DeviceError err) {
        qDebug() << "mount network device: " << address << ok << dfmmount::Utils::errorMessage(err);
    });
}

bool DeviceService::stopDefenderScanDrive(const QString &deviceId)
{
    auto &&ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QList<QUrl> &&urls = DeviceServiceHelper::makeMountpointsForDrive(ptr->drive());

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
}

bool DeviceService::stopDefenderScanAllDrives()
{
    QList<QUrl> &&urls = DeviceServiceHelper::makeMountpointsForAllDrive();

    if (!DefenderInstance.stopScanning(urls)) {
        qWarning() << "stop scanning timeout";
        return false;
    }

    return true;
}

/*!
 * \brief detach a block device (dde-dock plugin is eject, dde-file-manager
 * is safely remove)
 * \param deviceId is block device path
 * \param removeOptical is only effetive for optical disk
 * \return device ejected or poweroffed
 */

void DeviceService::detachBlockDevice(const QString &deviceId)
{
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    if (!ptr) {
        qWarning() << "Cannot create ptr for" << deviceId;
        return;
    }

    if (!ptr->removable()) {
        qWarning() << "Not removable device: " << deviceId;
        return;
    }

    // A block device may have more than one partition,
    // when detach a device, you need to unmount its partitions,
    // and then poweroff
    QStringList &&idList = DeviceServiceHelper::makeAllDevicesIdForDrive(ptr->drive());
    std::for_each(idList.cbegin(), idList.cend(), [this](const QString &id) {
        if (!unmountBlockDevice(id))
            qWarning() << "Detach " << id << " abnormal, it's cannot unmount";
    });

    if (ptr->mediaCompatibility().join(" ").contains("optical")) {
        if (ptr->optical())
            ejectBlockDeviceAsync(deviceId);
    } else
        poweroffBlockDeviceAsync(deviceId);
}

void DeviceService::detachProtocolDevice(const QString &deviceId)
{
    // for protocol devices, there is no eject/poweroff, so just unmount them
    unmountProtocolDeviceAsync(deviceId);
}

void DeviceService::detachAllMountedBlockDevices()
{
    QStringList &&list = blockDevicesIdList({ { ListOpt::kUnmountable, true } });
    for (const QString &id : list)
        detachBlockDevice(id);
}

void DeviceService::detachAllMountedProtocolDevices()
{
}

void DeviceService::mountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QString errMsg;
    if (DeviceServiceHelper::isMountableBlockDevice(ptr, &errMsg)) {
        ptr->mountAsync(opts, [this, deviceId](bool ret, DFMMOUNT::DeviceError err) {
            if (!ret) {
                qWarning() << "Mount failed: " << int(err);
                emit blockDevAsyncMounted(deviceId, false);
            } else {
                emit blockDevAsyncMounted(deviceId, true);
            }
        });
    } else {
        qWarning() << "Not mountable device: " << errMsg;
    }
}

QString DeviceService::mountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QString errMsg;
    if (DeviceServiceHelper::isMountableBlockDevice(ptr, &errMsg))
        return ptr->mount(opts);
    else
        qWarning() << "Not mountable device: " << errMsg;

    return "";
}

void DeviceService::unmountBlockDeviceAsync(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QString errMsg;
    if (DeviceServiceHelper::isUnmountableBlockDevice(ptr, &errMsg)) {
        if (Q_UNLIKELY(ptr->isEncrypted())) {
            auto clearBlkId = ptr->getProperty(dfmmount::Property::EncryptedCleartextDevice).toString();
            if (clearBlkId.length() > 1) {
                auto clearBlkPtr = DeviceServiceHelper::createBlockDevice(clearBlkId);
                clearBlkPtr->unmountAsync(opts, [deviceId, this, ptr](bool ret, DFMMOUNT::DeviceError err) {
                    if (err != DFMMOUNT::DeviceError::NoError)
                        qWarning() << "unmount device failed: " << deviceId << ": " << DFMMOUNT::Utils::errorMessage(err);
                    emit this->blockDevAsyncUnmounted(deviceId, ret);
                    if (ret) {
                        ptr->lockAsync({}, [this, deviceId](bool ret, DFMMOUNT::DeviceError lockErr) {
                            if (lockErr != DFMMOUNT::DeviceError::NoError)
                                qWarning() << "lock device failed: " << deviceId << ": " << DFMMOUNT::Utils::errorMessage(lockErr);
                            emit this->blockDevAsyncLocked(deviceId, ret);
                        });
                    }
                });
            }
        } else {
            ptr->unmountAsync(opts, [this, deviceId](bool ret, DFMMOUNT::DeviceError err) {
                if (!ret) {
                    qWarning() << "Unmount failed: " << int(err);
                    emit blockDevAsyncUnmounted(deviceId, false);
                } else {
                    emit blockDevAsyncUnmounted(deviceId, true);
                }
            });
        }
    } else {
        qWarning() << "Not unmountable device: " << errMsg;
    }
}

bool DeviceService::unmountBlockDevice(const QString &deviceId, const QVariantMap &opts)
{
    Q_ASSERT_X(!deviceId.isEmpty(), "DeviceService", "id is empty");
    auto ptr = DeviceServiceHelper::createBlockDevice(deviceId);
    QString errMsg;
    if (DeviceServiceHelper::isUnmountableBlockDevice(ptr, &errMsg)) {
        if (Q_UNLIKELY(ptr->isEncrypted())) {
            auto clearBlkId = ptr->getProperty(dfmmount::Property::EncryptedCleartextDevice).toString();
            if (clearBlkId.length() > 1) {
                auto clearBlkPtr = DeviceServiceHelper::createBlockDevice(clearBlkId);
                return clearBlkPtr->unmount(opts) && ptr->lock();
            }
        } else {
            return ptr->unmount(opts);
        }
    } else
        qWarning() << "Not unmountable device: " << errMsg;

    return false;
}

bool DeviceService::isBlockDeviceMonitorWorking() const
{
    bool ret = false;
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    if (manager) {
        auto blkMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::BlockDevice);
        if (blkMonitor && blkMonitor->status() == DFMMOUNT::MonitorStatus::Monitoring)
            ret = true;
    }

    return ret;
}

bool DeviceService::isProtolDeviceMonitorWorking() const
{
    bool ret = false;
    auto manager = DFMMOUNT::DFMDeviceManager::instance();

    if (manager) {
        auto protocolMonitor = manager->getRegisteredMonitor(DFMMOUNT::DeviceType::ProtocolDevice);
        if (protocolMonitor && protocolMonitor->status() == DFMMOUNT::MonitorStatus::Monitoring)
            ret = true;
    }

    return ret;
}

/*!
 * \brief check if we are in live system, don't do auto mount if we are in live system
 * \return true if live system
 */
bool DeviceService::isInLiveSystem() const
{
    bool ret = false;
    static const QMap<QString, QString> &cmdline = dfmbase::FileUtils::getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live"))
        ret = true;
    return ret;
}

/*!
 * \brief check property "AutoMount" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMount" property value
 */
bool DeviceService::isAutoMountSetting() const
{
    return DeviceServiceHelper::getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool();
}

/*!
 * \brief check property "AutoMountAndOpen" of ~/.config/deepin/dde-file-manager.json
 * \return "AutoMountAndOpen" property value
 */
bool DeviceService::isAutoMountAndOpenSetting() const
{
    return DeviceServiceHelper::getGsGlobal()->value("GenericAttribute", "AutoMountAndOpen", false).toBool();
}

bool DeviceService::isDefenderScanningDrive(const QString &deviceId) const
{
    QList<QUrl> urls;
    QString driveName;

    if (!deviceId.isEmpty()) {
        auto &&ptr = DeviceServiceHelper::createBlockDevice(deviceId);
        if (ptr)
            driveName = ptr->drive();
    }

    if (driveName.isNull() || driveName.isEmpty())
        urls = DeviceServiceHelper::makeMountpointsForAllDrive();
    else
        urls = DeviceServiceHelper::makeMountpointsForDrive(driveName);
    return DefenderInstance.isScanning(urls);
}

/*!
 * \brief user input a opts, then return block devices list
 * \param opts: bool unmountable     -> has mounted devices(dde-dock plugin use it)
 *              bool mountable       -> has unmounted devices
 *              bool not_ignorable   -> computer and sidebar devices
 * \return devices id list
 */
QStringList DeviceService::blockDevicesIdList(const QVariantMap &opts) const
{
    QStringList idList;

    // {"unmountable" : GLib.Variant("b", True)}
    bool needUnmountable = opts.value(ListOpt::kUnmountable).toBool();
    bool needMountable = opts.value(ListOpt::kMountable).toBool();
    bool needNotIgnorable = opts.value(ListOpt::kNotIgnorable).toBool();

    const auto allBlkData = monitorHandler->allBlockDevData;   // must use value!!!
    for (const auto &data : allBlkData) {
        if (needUnmountable && DeviceServiceHelper::isUnmountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        if (needMountable && DeviceServiceHelper::isMountableBlockDevice(data)) {
            idList.append(data.common.id);
            continue;
        }

        QString errMsg;
        if (needNotIgnorable && !DeviceServiceHelper::isIgnorableBlockDevice(data, &errMsg)) {
            idList.append(data.common.id);
            continue;
        } else if (!errMsg.isEmpty()) {
            qWarning() << "Device has beeen ignore: " << errMsg;
        }

        if (!needUnmountable && !needMountable && !needNotIgnorable) {
            idList.append(data.common.id);
            continue;
        }
    }

    return idList;
}

/*!
 * \brief make a map that contains all info for the block device
 * \param deviceId
 * \param detail: return all if true
 * if input:  '/org/freedesktop/UDisks2/block_devices/sr0', GLib.Variant("b", True)
 * \return like this:
 * {'can_power_off': True,
 *  'crypto_backingDevice': '/',
 *  'device': '/dev/sr0',
 *  'drive': '/org/freedesktop/UDisks2/drives/HL_DT_ST_DVDRAM_GP75N_KXDJAVB0521',
 *  'ejectable': True,
 *  'filesystem': 'iso9660',
 *  'has_filesystem': True,
 *  'has_partition_table': False,
 *  'hint_ignore': False,
 *  'hint_system': False,
 *  'id': '/org/freedesktop/UDisks2/block_devices/sr0',
 *  'id_label': '',
 *  'is_encrypted': False,
 *  'is_loop_device': False,
 *  'media': 'optical_dvd_r',
 *  'media_available': True,
 *  'media_compatibility': ['optical_cd',
 *                          'optical_cd_r',
 *                          'optical_cd_rw',
 *                          'optical_dvd',
 *                          'optical_dvd_plus_r',
 *                          'optical_dvd_plus_r_dl',
 *                          'optical_dvd_plus_rw',
 *                          'optical_dvd_r',
 *                          'optical_dvd_ram',
 *                          'optical_dvd_rw',
 *                          'optical_mrw',
 *                          'optical_mrw_w'],
 *  'mountpoint': '/media/zhangs/2021-10-15-09-50-02-00',
 *  'mountpoints': ['/media/zhangs/2021-10-15-09-50-02-00'],
 *  'optical': True,
 *  'optical_blank': False,
 *  'removable': True,
 *  'size_free': 0,
 *  'size_total': 393216,
 *  'size_usage': 393216}
 */
QVariantMap DeviceService::blockDeviceInfo(const QString &deviceId, bool detail) const
{
    QVariantMap info;
    const auto allBlkData = monitorHandler->allBlockDevData;   // must use value!!!
    if (!allBlkData.contains(deviceId))
        return info;

    const auto &&blkData = allBlkData.value(deviceId);
    DeviceServiceHelper::makeBlockDeviceMap(blkData, &info, detail);
    return info;
}

QStringList DeviceService::protocolDevicesIdList() const
{
    QStringList idList;

    const auto allProtoData = monitorHandler->allProtocolDevData;
    auto iter = allProtoData.cbegin();
    while (iter != allProtoData.cend()) {
        idList.append(iter.value().common.id);
        iter += 1;
    }
    return idList;
}

QVariantMap DeviceService::protocolDeviceInfo(const QString &deviceId, bool detail) const
{
    QVariantMap info;
    const auto allProtoData = monitorHandler->allProtocolDevData;
    if (!allProtoData.contains(deviceId))
        return info;

    const auto &protoData = allProtoData.value(deviceId);
    DeviceServiceHelper::makeProtocolDeviceMap(protoData, &info, detail);
    return info;
}