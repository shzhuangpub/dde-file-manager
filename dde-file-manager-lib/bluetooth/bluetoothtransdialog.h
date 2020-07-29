/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: xushitong<xushitong@uniontech.com>
 *
 *
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

#ifndef BluetoothTransDialog_H
#define BluetoothTransDialog_H

#include <dtkwidget_global.h>
#include <DDialog>
#include <durl.h>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
class DCommandLinkButton;
class DListView;
class DProgressBar;
class DStandardItem;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStackedWidget;
QT_END_NAMESPACE

class BluetoothDevice;
class BluetoothAdapter;
class BluetoothTransDialog : public DDialog
{
    Q_OBJECT
public:
    enum TransferMode {
        Default = 0, // 先选择设备再发送
        DirectlySend // 直接发送到指定设备
    };

    BluetoothTransDialog(const QStringList &urls, TransferMode mode = Default, QString targetDevId = QString(), QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initConn();
    QWidget *initDeviceSelectorPage();
    QWidget *initNonDevicePage();
    QWidget *initWaitForRecvPage();
    QWidget *initTranferingPage();
    QWidget *initFailedPage();
    QWidget *initSuccessPage();

    /**
     * @brief getStyledItem 根据蓝牙设备对象获取一个处理后的列表 item
     * @param dev 蓝牙设备对象
     * @return 返回一个列表 item 用于添加到列表中
     */
    DStandardItem *getStyledItem(const BluetoothDevice *dev);

    /**
     * @brief findItemByIdRole 从列表中获取对应蓝牙设备的 item
     * @param dev
     * @return
     */
    DStandardItem *findItemByIdRole(const BluetoothDevice *dev);
    DStandardItem *findItemByIdRole(const QString &devId);

    /**
     * @brief updateDeviceList 初始化加载设备列表
     */
    void updateDeviceList();

    /**
     * @brief addDevice 添加设备到设备列表
     * @param dev 蓝牙设备对象
     */
    void addDevice(const BluetoothDevice *dev);

    /**
     * @brief removeDevice 从列表中移除相应设备
     * @param dev 蓝牙设备对象
     */
    void removeDevice(const BluetoothDevice *dev);
    void removeDevice(const QString &id);

    void sendFiles();

    /**
     * @brief sendFilesToDevice 直接发送文件到指定设备
     * @param devId 接收方id
     */
    void sendFilesToDevice(const QString &devId);

private Q_SLOTS:
    void showBluetoothSetting();

    /**
     * @brief onBtnClicked 对话框按钮槽函数，根据页面及触发按钮的索引执行页面间的跳转
     * @param nIdx 按钮的索引
     */
    void onBtnClicked(const int &nIdx);

    /**
     * @brief onPageChagned 处理页面跳转后界面按钮的更新等操作
     * @param nIdx 页面的索引，索引值与枚举 Page 一致
     */
    void onPageChagned(const int &nIdx);

    /**
     * @brief connectAdapter 连接每个 adapter 的信号
     */
    void connectAdapter(const BluetoothAdapter *);

    /**
     * @brief connectDevice 连接每个设备的信号
     */
    void connectDevice(const BluetoothDevice *);

    /**
     * @brief onAcceptFiles 对方设备同意接收文件时触发
     */
    void onAcceptFiles();

    /**
     * @brief onProgressUpdated
     * @param sessionPath        用来标识当前传输进程的会话id
     * @param total              当前传输列表的总bytes
     * @param transferred        当前已传输的 bytes
     * @param currFileIndex      当前传输文件的序号
     */
    void onProgressUpdated(const QString &sessionPath, const qulonglong &total, const qulonglong &transferred, const int &currFileIndex);

Q_SIGNALS:
    void startSpinner();
    void stopSpinner();
    void resetProgress();

private:
    enum Page {
        SelectDevicePage,
        NoneDevicePage,
        WaitForRecvPage,
        TransferPage,
        FailedPage,
        SuccessPage,
    };

    enum DeviceRoles {
        DevNameRole = Qt::UserRole + 100,
        DevIdRole,
    };

    DLabel *m_titleOfDialog = nullptr;
    QStackedWidget *m_stack = nullptr; // 多页面容器
    DListView *m_devicesList = nullptr; // 设备列表
    QStandardItemModel *m_devModel = nullptr; // 列表的 model
    DLabel *m_subTitleForWaitPage = nullptr;
    DLabel *m_subTitleOfTransPage = nullptr;
    DLabel *m_subTitleOfFailedPage = nullptr;
    DLabel *m_subTitleOfSuccessPage = nullptr;
    DLabel *m_sendingStatus = nullptr;
    DProgressBar *m_progressBar = nullptr;

private:
    QStringList m_urls; // 待发送文件
    QString m_selectedDevice;
    QString m_selectedDeviceId;
    QString m_currSessionPath; // 当前发送进程的 obex 会话路径，用于取消当前传输会话
    int m_progressSignalEmitCount = 0; // 信号发送次数

    QStringList m_connectedAdapter;
};

#endif // BluetoothTransDialog_H
