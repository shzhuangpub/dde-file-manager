/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "app/global.h"
#include "appcontroller.h"
#include "fileoperation.h"
#include "tag/tagmanagerdaemon.h"
#include "usershare/usersharemanager.h"
#include "acesscontrol/acesscontrolmanager.h"
#include "vault/vaultmanager.h"
#include "disk/diskmanager.h"

AppController::AppController(QObject *parent) : QObject(parent)
{
    initControllers();
    initConnect();
}

AppController::~AppController()
{

}

void AppController::initControllers()
{
    m_acessController = new AcessControlManager(this);
    m_userShareManager = new UserShareManager(this);
    m_tagManagerDaemon = new TagManagerDaemon{ this };
    m_vaultManager = new VaultManager(this);
    m_diskManager = new DiskManager(this);
}

void AppController::initConnect()
{
}

