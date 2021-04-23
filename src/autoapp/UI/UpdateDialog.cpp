#include <f1x/openauto/autoapp/UI/UpdateDialog.hpp>
#include "ui_updatedialog.h"
#include <QFileInfo>
#include <QTextStream>
#include <QStorageInfo>
#include <fstream>
#include <cstdio>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
    , ui_(new Ui::UpdateDialog)
{
    ui_->setupUi(this);
    connect(ui_->pushButtonUpdateCsmt, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCsmt_clicked);
    connect(ui_->pushButtonUpdateUdev, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateUdev_clicked);
    connect(ui_->pushButtonUpdateOpenauto, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateOpenauto_clicked);
    connect(ui_->pushButtonUpdateCheck, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCheck_clicked);
    connect(ui_->pushButtonUpdateCancel, &QPushButton::clicked, this, &UpdateDialog::on_pushButtonUpdateCancel_clicked);
    connect(ui_->pushButtonClose, &QPushButton::clicked, this, &UpdateDialog::close);
    
    ui_->pushButtonClose->setFocus();
    ui_->progressBarCsmt->hide();
    ui_->progressBarUdev->hide();
    ui_->progressBarOpenauto->hide();
    ui_->labelUpdateChecking->hide();
    ui_->pushButtonUpdateCancel->hide();
    updateCheck();

    watcher_tmp = new QFileSystemWatcher(this);
    watcher_tmp->addPath("/tmp");
    connect(watcher_tmp, &QFileSystemWatcher::directoryChanged, this, &UpdateDialog::updateCheck);

}

UpdateDialog::~UpdateDialog()
{
    delete ui_;
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCsmt_clicked()
{
    ui_->pushButtonUpdateCsmt->hide();
    ui_->progressBarCsmt->show();
    qApp->processEvents();
    system("crankshaft update csmt &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateUdev_clicked()
{
    ui_->pushButtonUpdateUdev->hide();
    ui_->progressBarUdev->show();
    qApp->processEvents();
    system("crankshaft update udev &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateOpenauto_clicked()
{
    ui_->pushButtonUpdateOpenauto->hide();
    ui_->progressBarOpenauto->show();
    qApp->processEvents();
    system("crankshaft update openauto &");
}



void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCheck_clicked()
{
    ui_->pushButtonUpdateCheck->hide();
    ui_->labelUpdateChecking->show();
    qApp->processEvents();
    system("/usr/local/bin/crankshaft update check");
    updateCheck();
    ui_->labelUpdateChecking->hide();
    ui_->pushButtonUpdateCheck->show();
}

void f1x::openauto::autoapp::ui::UpdateDialog::on_pushButtonUpdateCancel_clicked()
{
    ui_->pushButtonUpdateCancel->hide();
    system("crankshaft update cancel &");
}

void f1x::openauto::autoapp::ui::UpdateDialog::downloadCheck()
{
    QDir directory("/media/USBDRIVES/CSSTORAGE");
    QStringList files = directory.entryList(QStringList() << "*.zip", QDir::AllEntries, QDir::Name);
    foreach(QString filename, files) {
        if (filename != "") {
            ui_->labelDownload->setText(filename);
        }
    }
}

void f1x::openauto::autoapp::ui::UpdateDialog::updateCheck()
{
    if (!std::ifstream("/tmp/csmt_updating")) {
        if (std::ifstream("/tmp/csmt_update_available")) {
            ui_->labelCsmtOK->hide();
            ui_->pushButtonUpdateCsmt->show();
        } else {
            ui_->pushButtonUpdateCsmt->hide();
            ui_->progressBarCsmt->hide();
            ui_->labelCsmtOK->show();
        }
    }

    if (!std::ifstream("/tmp/udev_updating")) {
        if (std::ifstream("/tmp/udev_update_available")) {
            ui_->labelUdevOK->hide();
            ui_->pushButtonUpdateUdev->show();
        } else {
            ui_->pushButtonUpdateUdev->hide();
            ui_->progressBarUdev->hide();
            ui_->labelUdevOK->show();
        }
    }

    if (!std::ifstream("/tmp/openauto_updating")) {
        if (std::ifstream("/tmp/openauto_update_available")) {
            ui_->labelOpenautoOK->hide();
            ui_->pushButtonUpdateOpenauto->show();
        } else {
            ui_->pushButtonUpdateOpenauto->hide();
            ui_->progressBarOpenauto->hide();
            ui_->labelOpenautoOK->show();
        }
    } else {
        ui_->labelOpenautoOK->hide();
        ui_->pushButtonUpdateOpenauto->hide();
    }

}

void f1x::openauto::autoapp::ui::UpdateDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        UpdateDialog::close();
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

}
}
}
}

