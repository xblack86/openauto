/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>
#include <QWidget>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <QFileDialog>
#include <QKeyEvent>
#include <sys/sysinfo.h>

class QCheckBox;
class QTimer;

namespace Ui
{
class SettingsWindow;
}

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

class SettingsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWindow(configuration::IConfiguration::Pointer configuration, QWidget *parent = nullptr);
    ~SettingsWindow() override;
    void loadSystemValues();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void unpairAll();
    void onSave();
    void onResetToDefaults();
    void onUpdateAlphaTrans(int value);
    void onUpdateBrightnessDay(int value);
    void onUpdateBrightnessNight(int value);
    void onStartHotspot();
    void onStopHotspot();
    void updateNetworkInfo();
    void on_pushButtonNetwork0_clicked();
    void on_pushButtonNetwork1_clicked();
    void updateSystemInfo();
    void updateInfo();

public slots:
    void show_tab1();
    void show_tab2();

private:
    void showEvent(QShowEvent* event);
    void load();

    Ui::SettingsWindow* ui_;
    configuration::IConfiguration::Pointer configuration_;
};

}
}
}
}
