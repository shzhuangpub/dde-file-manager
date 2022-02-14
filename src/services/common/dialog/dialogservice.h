/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DIALOGSERVICE_H
#define DIALOGSERVICE_H

#include "dfm_common_service_global.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"
#include "dfm-base/utils/devicemanager.h"

#include <dfm-framework/service/pluginservicecontext.h>
#include <dfm-mount/base/dfmmount_global.h>

#include <DDialog>

using namespace DTK_NAMESPACE::Widget;
DSC_BEGIN_NAMESPACE
class TaskDialog;
class ComputerPropertyDialog;
class TrashPropertyDialog;
class DialogService final : public dpf::PluginService, dpf::AutoServiceRegister<DialogService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DialogService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    enum MessageType {
        kMsgInfo = 1,
        kMsgWarn = 2,
        kMsgErr = 3
    };
    static QString name()
    {
        return "org.deepin.service.DialogService";
    }

    DDialog *showQueryScanningDialog(const QString &title);
    void showErrorDialog(const QString &title, const QString &message);
    int showMessageDialog(MessageType messageLevel, const QString &title, const QString &message = "", QString btnTxt = tr("Confirm", "button"));
    void showErrorDialogWhenMountDeviceFailed(DFMMOUNT::DeviceError err);
    void addTask(const JobHandlePointer &task);
    void showSetingsDialog(DFMBASE_NAMESPACE::FileManagerWindow *window);
    QString askPasswordForLockedDevice();
    bool askForFormat();

private:
    explicit DialogService(QObject *parent = nullptr);
    virtual ~DialogService() override;
    TaskDialog *taskdailog = nullptr;   // 文件任务进度和错误处理弹窗
};

DSC_END_NAMESPACE

#endif   // DIALOGSERVICE_H
