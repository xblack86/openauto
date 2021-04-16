/*subfolder
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

#include <QMessageBox>
#include <f1x/openauto/autoapp/UI/SettingsWindow.hpp>
#include "ui_settingswindow.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <string>
#include <QTimer>
#include <QDateTime>
#include <QNetworkInterface>
#include <fstream>
#include <QStorageInfo>
#include <QProcess>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

SettingsWindow::SettingsWindow(configuration::IConfiguration::Pointer configuration, QWidget *parent)
    : QWidget(parent)
    , ui_(new Ui::SettingsWindow)
    , configuration_(std::move(configuration))
{
    ui_->setupUi(this);
    connect(ui_->pushButtonCancel, &QPushButton::clicked, this, &SettingsWindow::close);
    connect(ui_->pushButtonSave, &QPushButton::clicked, this, &SettingsWindow::onSave);
    connect(ui_->pushButtonUnpair , &QPushButton::clicked, this, &SettingsWindow::unpairAll);
    connect(ui_->pushButtonUnpair , &QPushButton::clicked, this, &SettingsWindow::close);
    connect(ui_->horizontalSliderAlphaTrans, &QSlider::valueChanged, this, &SettingsWindow::onUpdateAlphaTrans);
    connect(ui_->horizontalSliderDay, &QSlider::valueChanged, this, &SettingsWindow::onUpdateBrightnessDay);
    connect(ui_->horizontalSliderNight, &QSlider::valueChanged, this, &SettingsWindow::onUpdateBrightnessNight);
    connect(ui_->radioButtonUseExternalBluetoothAdapter, &QRadioButton::clicked, [&](bool checked) { ui_->lineEditExternalBluetoothAdapterAddress->setEnabled(checked); });
    connect(ui_->radioButtonDisableBluetooth, &QRadioButton::clicked, [&]() { ui_->lineEditExternalBluetoothAdapterAddress->setEnabled(false); });
    connect(ui_->radioButtonUseLocalBluetoothAdapter, &QRadioButton::clicked, [&]() { ui_->lineEditExternalBluetoothAdapterAddress->setEnabled(false); });
    connect(ui_->pushButtonResetToDefaults, &QPushButton::clicked, this, &SettingsWindow::onResetToDefaults);
    connect(ui_->radioButtonHotspot, &QPushButton::clicked, this, &SettingsWindow::onStartHotspot);
    connect(ui_->radioButtonClient, &QPushButton::clicked, this, &SettingsWindow::onStopHotspot);
    connect(ui_->pushButtonCheckNow, &QPushButton::clicked, [&]() { system("/usr/local/bin/crankshaft update check &"); });
    connect(ui_->pushButtonNetwork0, &QPushButton::clicked, this, &SettingsWindow::on_pushButtonNetwork0_clicked);
    connect(ui_->pushButtonNetwork1, &QPushButton::clicked, this, &SettingsWindow::on_pushButtonNetwork1_clicked);
    

    // menu
    ui_->tab1->show();
    ui_->tab2->hide();
     
    ui_->labelBluetoothAdapterAddress->hide();
    ui_->lineEditExternalBluetoothAdapterAddress->hide();

    connect(ui_->pushButtonTab1, &QPushButton::clicked, this, &SettingsWindow::show_tab1);
    connect(ui_->pushButtonTab2, &QPushButton::clicked, this, &SettingsWindow::show_tab2);
    connect(ui_->pushButtonTab2, &QPushButton::clicked, this, &SettingsWindow::updateNetworkInfo);


    ui_->label_modeswitchprogress->setText("Ok");
    ui_->label_notavailable->hide();

    QString wifi_ssid = configuration_->getCSValue("WIFI_SSID");
    QString wifi2_ssid = configuration_->getCSValue("WIFI2_SSID");

    ui_->pushButtonNetwork0->setText(wifi_ssid);
    ui_->pushButtonNetwork1->setText(wifi2_ssid);

    if (!std::ifstream("/boot/crankshaft/network1.conf")) {
        ui_->pushButtonNetwork1->hide();
        ui_->pushButtonNetwork0->show();
    }
    if (!std::ifstream("/boot/crankshaft/network0.conf")) {
        ui_->pushButtonNetwork1->hide();
        ui_->pushButtonNetwork0->setText(configuration_->getCSValue("WIFI2_SSID"));
    }
    if (!std::ifstream("/boot/crankshaft/network0.conf") && !std::ifstream("/boot/crankshaft/network1.conf")) {
        ui_->pushButtonNetwork0->hide();
        ui_->pushButtonNetwork1->hide();
        //ui_->pushButtonNetworkAuto->hide();
        ui_->label_notavailable->show();
    }

    if (std::ifstream("/tmp/hotspot_active")) {
        ui_->radioButtonClient->setChecked(0);
        ui_->radioButtonHotspot->setChecked(1);
        ui_->lineEditWifiSSID->setText(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid"));
        ui_->lineEditPassword->show();
        ui_->label_password->show();
        ui_->lineEditPassword->setText("1234567890");
        ui_->clientNetworkSelect->hide();
        ui_->label_notavailable->show();
    } else {
        ui_->radioButtonClient->setChecked(1);
        ui_->radioButtonHotspot->setChecked(0);
        ui_->lineEditWifiSSID->setText(configuration_->readFileContent("/tmp/wifi_ssid"));
        ui_->lineEditPassword->hide();
        ui_->label_password->hide();
        ui_->lineEditPassword->setText("");
        ui_->clientNetworkSelect->hide();
        ui_->label_notavailable->show();
    }

   
    QTimer *refresh=new QTimer(this);
    connect(refresh, SIGNAL(timeout()),this,SLOT(updateInfo()));
    refresh->start(5000);
}

SettingsWindow::~SettingsWindow()
{
    delete ui_;
}

void SettingsWindow::updateInfo()
{
    if (ui_->tab2->isVisible() == true) {
        updateNetworkInfo();
    }
}

void SettingsWindow::onSave()
{
    configuration_->setHandednessOfTrafficType(ui_->radioButtonLeftHandDrive->isChecked() ? configuration::HandednessOfTrafficType::LEFT_HAND_DRIVE : configuration::HandednessOfTrafficType::RIGHT_HAND_DRIVE);

    configuration_->setAlphaTrans(static_cast<size_t>(ui_->horizontalSliderAlphaTrans->value()));
    configuration_->hideMenuToggle(ui_->checkBoxHideMenuToggle->isChecked());
    configuration_->hideBrightnessControl(ui_->checkBoxHideBrightnessControl->isChecked());
    configuration_->showNetworkinfo(ui_->checkBoxNetworkinfo->isChecked());
    configuration_->hideWarning(ui_->checkBoxDontShowAgain->isChecked());

    if(ui_->radioButtonDisableBluetooth->isChecked())
    {
        // set bluetooth
        if (configuration_->getCSValue("ENABLE_BLUETOOTH") == "1") {
                    system("/usr/local/bin/crankshaft bluetooth disable");
        }
        configuration_->setBluetoothAdapterType(configuration::BluetoothAdapterType::NONE);
        
    }
    else if(ui_->radioButtonUseLocalBluetoothAdapter->isChecked())
    {
        // set bluetooth
        if (configuration_->getCSValue("ENABLE_BLUETOOTH") == "0" || configuration_->getCSValue("EXTERNAL_BLUETOOTH") == "1") {
            system("/usr/local/bin/crankshaft bluetooth builtin");
        }
        configuration_->setBluetoothAdapterType(configuration::BluetoothAdapterType::LOCAL);
        
    }
    else if(ui_->radioButtonUseExternalBluetoothAdapter->isChecked())
    {
        if (configuration_->getCSValue("ENABLE_BLUETOOTH") == "0" || configuration_->getCSValue("EXTERNAL_BLUETOOTH") == "0") {
            system("/usr/local/bin/crankshaft bluetooth external");
        }
        configuration_->setBluetoothAdapterType(configuration::BluetoothAdapterType::REMOTE);
        
    }

    configuration_->setBluetoothRemoteAdapterAddress(ui_->lineEditExternalBluetoothAdapterAddress->text().toStdString());

    configuration_->save();

    // generate param string for autoapp_helper
    std::string params;
    
    if (ui_->checkBoxHotspot->isChecked()) {//1
        params.append("1");
    } else {
        params.append("0");
    }
    params.append("#");
    if (ui_->checkBoxBluetoothAutoPair->isChecked()) {//2
        params.append("1");
    } else {
        params.append("0");
    }
    params.append("#");
    params.append( std::to_string(ui_->horizontalSliderDay->value()) );//3
    params.append("#");
    params.append( std::to_string(ui_->horizontalSliderNight->value()) );//4
    params.append("#");
    params.append( std::string(ui_->comboBoxCountryCode->currentText().split("|")[0].replace(" ","").toStdString()) );//5
    params.append("#");
    system((std::string("/usr/local/bin/autoapp_helper setparams#") + std::string(params) + std::string(" &") ).c_str());

    this->close();
}

void SettingsWindow::onResetToDefaults()
{
    QMessageBox confirmationMessage(QMessageBox::Question, "Confirmation", "Are you sure you want to reset settings?", QMessageBox::Yes | QMessageBox::Cancel);
    //confirmationMessage.setWindowFlags(Qt::WindowStaysOnTopHint);
    if(confirmationMessage.exec() == QMessageBox::Yes)
    {
        configuration_->reset();
        this->load();
    }
}

void SettingsWindow::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    this->load();
}

void SettingsWindow::load()
{
    ui_->radioButtonLeftHandDrive->setChecked(configuration_->getHandednessOfTrafficType() == configuration::HandednessOfTrafficType::LEFT_HAND_DRIVE);
    ui_->radioButtonRightHandDrive->setChecked(configuration_->getHandednessOfTrafficType() == configuration::HandednessOfTrafficType::RIGHT_HAND_DRIVE);
    
    ui_->horizontalSliderAlphaTrans->setValue(static_cast<int>(configuration_->getAlphaTrans()));

    ui_->checkBoxHideMenuToggle->setChecked(configuration_->hideMenuToggle());
    ui_->checkBoxHideBrightnessControl->setChecked(configuration_->hideBrightnessControl());
    ui_->checkBoxNetworkinfo->setChecked(configuration_->showNetworkinfo());
    ui_->checkBoxDontShowAgain->setChecked(configuration_->hideWarning());

    ui_->radioButtonDisableBluetooth->setChecked(configuration_->getBluetoothAdapterType() == configuration::BluetoothAdapterType::NONE);
    ui_->radioButtonUseLocalBluetoothAdapter->setChecked(configuration_->getBluetoothAdapterType() == configuration::BluetoothAdapterType::LOCAL);
    ui_->radioButtonUseExternalBluetoothAdapter->setChecked(configuration_->getBluetoothAdapterType() == configuration::BluetoothAdapterType::REMOTE);
    ui_->lineEditExternalBluetoothAdapterAddress->setEnabled(configuration_->getBluetoothAdapterType() == configuration::BluetoothAdapterType::REMOTE);
    ui_->lineEditExternalBluetoothAdapterAddress->setText(QString::fromStdString(configuration_->getBluetoothRemoteAdapterAddress()));

}


void SettingsWindow::onUpdateAlphaTrans(int value)
{
    double alpha = value/100.0;
    ui_->labelAlphaTransValue->setText(QString::number(alpha));
}

void SettingsWindow::onUpdateBrightnessDay(int value)
{
    ui_->labelBrightnessDay->setText(QString::number(value));
}

void SettingsWindow::onUpdateBrightnessNight(int value)
{
    ui_->labelBrightnessNight->setText(QString::number(value));
}

void SettingsWindow::unpairAll()
{
    system("/usr/local/bin/crankshaft bluetooth unpair &");
}

void SettingsWindow::loadSystemValues()
{
    // set brightness slider attribs
    ui_->horizontalSliderDay->setMinimum(configuration_->getCSValue("BR_MIN").toInt());
    ui_->horizontalSliderDay->setMaximum(configuration_->getCSValue("BR_MAX").toInt());
    ui_->horizontalSliderDay->setSingleStep(configuration_->getCSValue("BR_STEP").toInt());
    ui_->horizontalSliderDay->setTickInterval(configuration_->getCSValue("BR_STEP").toInt());
    ui_->horizontalSliderDay->setValue(configuration_->getCSValue("BR_DAY").toInt());

    ui_->horizontalSliderNight->setMinimum(configuration_->getCSValue("BR_MIN").toInt());
    ui_->horizontalSliderNight->setMaximum(configuration_->getCSValue("BR_MAX").toInt());
    ui_->horizontalSliderNight->setSingleStep(configuration_->getCSValue("BR_STEP").toInt());
    ui_->horizontalSliderNight->setTickInterval(configuration_->getCSValue("BR_STEP").toInt());
    ui_->horizontalSliderNight->setValue(configuration_->getCSValue("BR_NIGHT").toInt());

   
    if (std::ifstream("/tmp/return_value")) {
        QString return_values = configuration_->readFileContent("/tmp/return_value");
        QStringList getparams = return_values.split("#");

        // Wifi Hotspot
        if (configuration_->getCSValue("ENABLE_HOTSPOT") == "1") {
            ui_->checkBoxHotspot->setChecked(true);
        } else {
            ui_->checkBoxHotspot->setChecked(false);
        }

        // set bluetooth
        if (configuration_->getCSValue("ENABLE_BLUETOOTH") == "1") {
            // check external bluetooth enabled
            if (configuration_->getCSValue("EXTERNAL_BLUETOOTH") == "1") {
                ui_->radioButtonUseExternalBluetoothAdapter->setChecked(true);
            } else {
                ui_->radioButtonUseLocalBluetoothAdapter->setChecked(true);
            }
            // mac
            //ui_->lineEditExternalBluetoothAdapterAddress->setText(getparams[37]);
        } else {
            ui_->radioButtonDisableBluetooth->setChecked(true);
            ui_->lineEditExternalBluetoothAdapterAddress->setText("");
        }
        if (configuration_->getCSValue("ENABLE_PAIRABLE") == "1") {
            ui_->checkBoxBluetoothAutoPair->setChecked(true);
        } else {
            ui_->checkBoxBluetoothAutoPair->setChecked(false);
        }
        // set bluetooth type
        if (configuration_->getCSValue("ENABLE_BLUETOOTH") == "1") {
            QString bt = configuration_->getParamFromFile("/boot/config.txt","dtoverlay=pi3-disable-bt");
            if (bt.contains("pi3-disable-bt")) {
                ui_->comboBoxBluetooth->setCurrentText("external");
            } else {
                ui_->comboBoxBluetooth->setCurrentText("builtin");
            }
        } else {
            ui_->comboBoxBluetooth->setCurrentText("none");
        }

        // wifi country code
        ui_->comboBoxCountryCode->setCurrentIndex(ui_->comboBoxCountryCode->findText(configuration_->getCSValue("WIFI_COUNTRY"), Qt::MatchFlag::MatchStartsWith));
    }
    // update network info
    updateNetworkInfo();
}

void SettingsWindow::onStartHotspot()
{
    ui_->label_modeswitchprogress->setText("Wait ...");
    ui_->clientNetworkSelect->hide();
    ui_->label_notavailable->show();
    ui_->radioButtonClient->setEnabled(0);
    ui_->radioButtonHotspot->setEnabled(0);
    ui_->lineEdit_wlan0->setText("");
    ui_->lineEditWifiSSID->setText("");
    //ui_->pushButtonNetworkAuto->hide();
    qApp->processEvents();
    std::remove("/tmp/manual_hotspot_control");
    std::ofstream("/tmp/manual_hotspot_control");
    system("/opt/crankshaft/service_hotspot.sh start &");
}

void SettingsWindow::onStopHotspot()
{
    ui_->label_modeswitchprogress->setText("Wait ...");
    ui_->clientNetworkSelect->hide();
    ui_->label_notavailable->show();
    ui_->radioButtonClient->setEnabled(0);
    ui_->radioButtonHotspot->setEnabled(0);
    ui_->lineEdit_wlan0->setText("");
    ui_->lineEditWifiSSID->setText("");
    ui_->lineEditPassword->setText("");
    //ui_->pushButtonNetworkAuto->hide();
    qApp->processEvents();
    system("/opt/crankshaft/service_hotspot.sh stop &");
}

void SettingsWindow::show_tab1()
{
    ui_->tab2->hide();
    ui_->tab1->show();
}

void SettingsWindow::show_tab2()
{
    ui_->tab1->hide();
    ui_->tab2->show();
}


}
}
}
}


void f1x::openauto::autoapp::ui::SettingsWindow::updateNetworkInfo()
{

    if (!std::ifstream("/tmp/mode_change_progress")) {
        QNetworkInterface eth0if = QNetworkInterface::interfaceFromName("eth0");
        if (eth0if.flags().testFlag(QNetworkInterface::IsUp)) {
            QList<QNetworkAddressEntry> entrieseth0 = eth0if.addressEntries();
            if (!entrieseth0.isEmpty()) {
                QNetworkAddressEntry eth0 = entrieseth0.first();
                //qDebug() << "eth0: " << eth0.ip();
                ui_->lineEdit_eth0->setText(eth0.ip().toString());
            }
        } else {
            //qDebug() << "eth0: down";
            ui_->lineEdit_eth0->setText("interface down");
        }

        QNetworkInterface wlan0if = QNetworkInterface::interfaceFromName("wlan0");
        if (wlan0if.flags().testFlag(QNetworkInterface::IsUp)) {
            QList<QNetworkAddressEntry> entrieswlan0 = wlan0if.addressEntries();
            if (!entrieswlan0.isEmpty()) {
                QNetworkAddressEntry wlan0 = entrieswlan0.first();
                //qDebug() << "wlan0: " << wlan0.ip();
                ui_->lineEdit_wlan0->setText(wlan0.ip().toString());
            }
        } else {
            //qDebug() << "wlan0: down";
            ui_->lineEdit_wlan0->setText("interface down");
        }

        if (std::ifstream("/tmp/hotspot_active")) {
            ui_->radioButtonClient->setEnabled(1);
            ui_->radioButtonHotspot->setEnabled(1);
            ui_->radioButtonHotspot->setChecked(1);
            ui_->radioButtonClient->setChecked(0);
            ui_->label_modeswitchprogress->setText("Ok");
            ui_->lineEditWifiSSID->setText(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","ssid"));
            ui_->lineEditPassword->show();
            ui_->label_password->show();
            ui_->lineEditPassword->setText(configuration_->getParamFromFile("/etc/hostapd/hostapd.conf","wpa_passphrase"));
            ui_->clientNetworkSelect->hide();
            //ui_->pushButtonNetworkAuto->hide();
            ui_->label_notavailable->show();
        } else {
            ui_->radioButtonClient->setEnabled(1);
            ui_->radioButtonHotspot->setEnabled(1);
            ui_->radioButtonHotspot->setChecked(0);
            ui_->radioButtonClient->setChecked(1);
            ui_->label_modeswitchprogress->setText("Ok");
            ui_->lineEditWifiSSID->setText(configuration_->readFileContent("/tmp/wifi_ssid"));
            ui_->lineEditPassword->hide();
            ui_->label_password->hide();
            ui_->lineEditPassword->setText("");
            ui_->clientNetworkSelect->show();
            ui_->label_notavailable->hide();
            //ui_->pushButtonNetworkAuto->show();

            if (!std::ifstream("/boot/crankshaft/network1.conf")) {
                ui_->pushButtonNetwork1->hide();
                ui_->pushButtonNetwork0->show();
            }
            if (!std::ifstream("/boot/crankshaft/network0.conf")) {
                ui_->pushButtonNetwork1->hide();
                ui_->pushButtonNetwork0->setText(configuration_->getCSValue("WIFI2_SSID"));
            }
            if (!std::ifstream("/boot/crankshaft/network0.conf") && !std::ifstream("/boot/crankshaft/network1.conf")) {
                ui_->pushButtonNetwork0->hide();
                ui_->pushButtonNetwork1->hide();
                //ui_->pushButtonNetworkAuto->hide();
                ui_->label_notavailable->show();
            }
        }
    }
}

void f1x::openauto::autoapp::ui::SettingsWindow::on_pushButtonNetwork0_clicked()
{
    ui_->lineEdit_wlan0->setText("");
    ui_->lineEditWifiSSID->setText("");
    ui_->lineEditPassword->setText("");
    qApp->processEvents();
    system("/usr/local/bin/crankshaft network 0 >/dev/null 2>&1 &");

}

void f1x::openauto::autoapp::ui::SettingsWindow::on_pushButtonNetwork1_clicked()
{
    ui_->lineEdit_wlan0->setText("");
    ui_->lineEditWifiSSID->setText("");
    ui_->lineEditPassword->setText("");
    qApp->processEvents();
    system("/usr/local/bin/crankshaft network 1 >/dev/null 2>&1 &");
}

void f1x::openauto::autoapp::ui::SettingsWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        f1x::openauto::autoapp::ui::SettingsWindow::close();
    }
    if (event->key() == Qt::Key_Return) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier));
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier));
    }    
    if (event->key() == Qt::Key_1) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier));
    }
    if (event->key() == Qt::Key_2) {
        QApplication::postEvent (QApplication::focusWidget(), new QKeyEvent ( QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier));
    }
}
