#include "transfer_dialog.h"
#include "file_dialog.h"
#include "list_of_job_options.h"
#include "utils.h"

TransferDialog::TransferDialog(bool isDownload, bool isDrop,
                               const QString &remote, const QDir &path,
                               bool isFolder, const QString &remoteType,
                               const QString &remoteMode, bool isMultiItem,
                               const QStringList &includeItems, QWidget *parent,
                               JobOptions *task, bool editMode)
    : QDialog(parent), mIsDownload(isDownload), mIsFolder(isFolder),
      mIsEditMode(editMode), mRemote(remote), mRemoteMode(remoteMode),
      mRemoteType(remoteType), mIsMultiItem(isMultiItem), mJobOptions(task) {

  ui.setupUi(this);

  // remove window close button
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint |
                 Qt::WindowMinMaxButtonsHint);

  mRemote = mRemote.left((mRemote).indexOf(":"));

  if (mJobOptions == nullptr) {
    mJobOptions = new JobOptions(mIsDownload);
  }

  mJobOptionsRcloneCmd = new JobOptions(mIsDownload);

  auto settings = GetSettings();

  // set minimumWidth based on font size
  int fontsize = 0;
  fontsize = (settings->value("Settings/fontSize").toInt());

#ifdef Q_OS_WIN
  // fusion on Windows is smaller
  setMinimumWidth(minimumWidth() * .9 + (fontsize * 20));
#else
  setMinimumWidth(minimumWidth() + (fontsize * 50));
#endif

  // Elided....Text base measure
  QFontMetrics metrix(ui.l_sourceRemote->font());

  if (mIsEditMode) {
    ui.cb_taskAutoName->hide();
    ui.cb_taskAddToQueue->hide();
  }

  // lock vertical resizing
  resize(0, 0);
  adjustSize();

  setMaximumHeight(this->height());
  setMinimumHeight(this->height());
  QTimer::singleShot(0, this, SLOT(size()));

  setWindowTitle(isDownload ? "Rclone Browser - Download"
                            : "Rclone Browser - Upload");

  QString iconsColour = settings->value("Settings/iconsColour").toString();

  QString img_add = "";

  if (iconsColour == "white") {
    img_add = "_inv";
  }

  ui.buttonSourceFile->setIcon(
      QIcon(":media/images/qbutton_icons/file" + img_add + ".png"));
  ui.buttonSourceItems->setIcon(
      QIcon(":media/images/qbutton_icons/folder" + img_add + ".png"));
  ui.buttonDest->setIcon(
      QIcon(":media/images/qbutton_icons/folder" + img_add + ".png"));
  ui.buttonDefaultSource->setIcon(
      QIcon(":media/images/qbutton_icons/restore" + img_add + ".png"));
  ui.buttonDefaultDest->setIcon(
      QIcon(":media/images/qbutton_icons/restore" + img_add + ".png"));

  ui.buttonSourceFile->setIconSize(QSize(24, 24));
  ui.buttonSourceItems->setIconSize(QSize(24, 24));
  ui.buttonDest->setIconSize(QSize(24, 24));
  ui.buttonDefaultSource->setIconSize(QSize(24, 24));
  ui.buttonDefaultDest->setIconSize(QSize(24, 24));

  // only used for layout
  ui.comboBoxSpacer1->hide();
  ui.comboBoxSpacer2->hide();
  ui.comboBoxSpacer3->hide();
  ui.comboBoxSpacer4->hide();

  QPushButton *saveTask = ui.buttonBox->addButton(
      "&Save task", QDialogButtonBox::ButtonRole::ActionRole);

  QPushButton *dryRun =
      ui.buttonBox->addButton("Dry run", QDialogButtonBox::AcceptRole);

  dryRun->setToolTip("trial run with no permanent changes (--dry-run)");

  QObject::connect(dryRun, &QPushButton::clicked, this,
                   [=]() { mDryRun = true; });

  QPushButton *run =
      ui.buttonBox->addButton("&Run", QDialogButtonBox::AcceptRole);

  QObject::connect(run, &QPushButton::clicked, this, [=]() {});

  run->setToolTip("ALT-r");

  saveTask->setDefault(true);

  if (!mIsEditMode) {

    saveTask->setEnabled(false);
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    QObject::connect(ui.cb_taskAutoName, &QCheckBox::clicked, this, [=]() {
      if (ui.cb_taskAutoName->isChecked()) {
        saveTask->setEnabled(true);
        saveTask->setDefault(true);
        run->setText("Save and &Run");
        dryRun->setText("Save and Dry run");
      } else {

        if (ui.le_taskName->text().trimmed().isEmpty()) {
          run->setText("&Run");
          dryRun->setText("Dry run");
          saveTask->setEnabled(false);
          ui.buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
        } else {
          run->setText("Save and &Run");
          dryRun->setText("Save and Dry run");
          saveTask->setEnabled(true);
          saveTask->setDefault(true);
        }
      }
    });

    QObject::connect(ui.cb_taskAddToQueue, &QCheckBox::clicked, this, [=]() {
      auto settings = GetSettings();
      if (ui.cb_taskAddToQueue->isChecked()) {
      } else {
      }
    });

    QObject::connect(ui.le_taskName, &QLineEdit::textChanged, this, [=]() {
      if (ui.le_taskName->text().trimmed().isEmpty()) {

        if (ui.cb_taskAutoName->isChecked()) {
          run->setText("Save and &Run");
          dryRun->setText("Save and Dry run");
          saveTask->setEnabled(true);
          saveTask->setDefault(true);
        } else {
          run->setText("&Run");
          dryRun->setText("Dry run");
          saveTask->setEnabled(false);
          ui.buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
        }

      } else {
        run->setText("Save and &Run");
        dryRun->setText("Save and Dry run");
        saveTask->setEnabled(true);
        saveTask->setDefault(true);
      }
    });

  } // if (!mIsEditMode)
  else {
    // edit mode

    QObject::connect(ui.le_taskName, &QLineEdit::textChanged, this, [=]() {
      if (ui.le_taskName->text().trimmed().isEmpty()) {
        saveTask->setEnabled(false);
        ui.buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
      } else {
        saveTask->setEnabled(true);
        saveTask->setDefault(true);
      }
    });

    dryRun->setEnabled(false);
    run->setEnabled(false);
    dryRun->hide();
    run->hide();
  } // if (!mIsEditMode) else

  QObject::connect(ui.pte_textExtra, &QPlainTextEdit::textChanged, this, [=]() {
    if (ui.pte_textExtra->toPlainText().trimmed().isEmpty()) {
      ui.tabWidget->setTabText(2, "Extra Options");
    } else {
      ui.tabWidget->setTabText(2, "Extra Options(x)");
    }
  });

  QObject::connect(ui.textInclude, &QPlainTextEdit::textChanged, this, [=]() {
    if (ui.textInclude->toPlainText().trimmed().isEmpty()) {
      ui.tabWidget->setTabText(3, "Include");
    } else {
      ui.tabWidget->setTabText(3, "Include(x)");
    }
  });

  QObject::connect(ui.textExclude, &QPlainTextEdit::textChanged, this, [=]() {
    if (ui.textExclude->toPlainText().trimmed().isEmpty()) {
      ui.tabWidget->setTabText(4, "Exclude");
    } else {
      ui.tabWidget->setTabText(4, "Exclude(x)");
    }
  });

  QObject::connect(ui.textFilter, &QPlainTextEdit::textChanged, this, [=]() {
    if (ui.textFilter->toPlainText().trimmed().isEmpty()) {
      ui.tabWidget->setTabText(5, "Filter");
    } else {
      ui.tabWidget->setTabText(5, "Filter(x)");
    }
  });

  QObject::connect(ui.textSource, &QLineEdit::textChanged, this,
                   [=]() { rcloneCmdUpdate(); });

  QObject::connect(ui.textDest, &QLineEdit::textChanged, this,
                   [=]() { rcloneCmdUpdate(); });

  QObject::connect(
      ui.buttonBox->button(QDialogButtonBox::RestoreDefaults),
      &QPushButton::clicked, this, [=]() {
        ui.rbCopy->setChecked(true);
        ui.cbSyncDelete->setCurrentIndex(0);
        // set combobox tooltips
        ui.cbSyncDelete->setItemData(0, "--delete-during", Qt::ToolTipRole);
        ui.cbSyncDelete->setItemData(1, "--delete-after", Qt::ToolTipRole);
        ui.cbSyncDelete->setItemData(2, "--delete-before", Qt::ToolTipRole);
        ui.checkSkipNewer->setChecked(false);
        ui.checkSkipNewer->setChecked(false);
        ui.checkSkipExisting->setChecked(false);
        ui.checkCompare->setChecked(true);
        ui.cbCompare->setCurrentIndex(0);
        // set combobox tooltips
        ui.cbCompare->setItemData(0, "default", Qt::ToolTipRole);
        ui.cbCompare->setItemData(1, "--checksum", Qt::ToolTipRole);
        ui.cbCompare->setItemData(2, "--ignore-size", Qt::ToolTipRole);
        ui.cbCompare->setItemData(3, "--size-only", Qt::ToolTipRole);
        ui.cbCompare->setItemData(4, "--checksum --ignore-size",
                                  Qt::ToolTipRole);
        //      ui.checkVerbose->setChecked(false);
        ui.checkSameFilesystem->setChecked(false);
        ui.noTraverse->setChecked(false);
        ui.createEmptySrcDirs->setChecked(true);
        ui.deleteEmptySrcDirs->setChecked(false);
        ui.checkDontUpdateModified->setChecked(false);
        ui.spinTransfers->setValue(4);
        ui.spinCheckers->setValue(8);
        ui.textBandwidth->clear();
        ui.textMinSize->clear();
        ui.textMinAge->clear();
        ui.textMaxAge->clear();
        ui.spinMaxDepth->setValue(0);
        ui.spinConnectTimeout->setValue(60);
        ui.spinIdleTimeout->setValue(300);
        ui.spinRetries->setValue(3);
        ui.spinLowLevelRetries->setValue(10);
        ui.checkDeleteExcluded->setChecked(false);
        ui.textInclude->clear();
        ui.textExclude->clear();
        ui.pte_textExtra->clear();

        if (!mIsMultiItem || mIsEditMode) {
          //          ui.textInclude->clear();
        }

        rcloneCmdUpdate();
      });

  ui.buttonBox->button(QDialogButtonBox::RestoreDefaults)->click();

  QObject::connect(saveTask, &QPushButton::clicked, this, [=]() {
    if (saveTaskToFile()) {

      // always close on save
      // if (mIsEditMode)
      this->close();
    } else {
      return;
    }
  });

  QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, this,
                   &QDialog::accept);
  QObject::connect(ui.buttonBox, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);

  QObject::connect(ui.tabWidget, &QTabWidget::currentChanged, this,
                   [=](const int &index) {
                     if (index == 6) {
                       rcloneCmdUpdate();
                     }
                   });

  QObject::connect(ui.buttonSourceFile, &QToolButton::clicked, this, [=]() {
    QString file = QFileDialog::getOpenFileName(this, "Choose file to upload");
    if (!file.isEmpty()) {
      if (!mIsEditMode) {
        ui.textSource->setText(QDir::toNativeSeparators(file));
        ui.textDest->setText(path.path());
      } else {
        ui.textSource->setText(QDir::toNativeSeparators(file));
      }
    }
  });

  QObject::connect(ui.buttonSourceItems, &QToolButton::clicked, this, [=]() {
    auto settings = GetSettings();
    QString last_used_source_folder =
        (settings->value("Settings/lastUsedSourceFolder").toString());

    QStringList itemsToUpload;
    QString itemToUpload;
    QString folder;
    QString file;

    FileDialog fileDialog(false);
    fileDialog.setWindowTitle(
        "Rclone Browser - Upload - choose items (one or many)");
    fileDialog.setMinimumWidth(760);
    fileDialog.setMinimumHeight(470);

    if (fileDialog.exec()) {

      if (!ui.textFilter->toPlainText().isEmpty() &&
          fileDialog.selectedUrls().count() > 1) {

        int currentTab = ui.tabWidget->currentIndex();

        ui.tabWidget->setCurrentIndex(5);
        ui.textFilter->setFocus(Qt::FocusReason::OtherFocusReason);

        int button = QMessageBox::question(
            this, "New Filter rules",
            QString(
                "Choosing new multi source will overwrite existing --filter "
                "patterns. Are you sure?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        ui.tabWidget->setCurrentIndex(currentTab);

        if (button == QMessageBox::Yes) {
          ui.textFilter->clear();
        } else {
          return;
        }
      }

      QList<QUrl> listSelectedUrls = fileDialog.selectedUrls();
      itemToUpload = listSelectedUrls.at(0).path();

#ifdef Q_OS_WIN
      // remove leading / e.g. "/C:/"
      itemToUpload.remove(0, 1);
#endif

      if (listSelectedUrls.count() == 1) {
        ui.multiInfo->hide();
        if (mIsMultiItem) {
          ui.textFilter->clear();
        }
        mIsMultiItem = false;

        // file or folder?
        if (QDir(itemToUpload).exists()) {
          folder = itemToUpload;

          if (!folder.isEmpty()) {
            // store new folder in lastUsedSourceFolder
            settings->setValue("Settings/lastUsedSourceFolder", folder);
            ui.textSource->setText(QDir::toNativeSeparators(folder));

            if (!mIsEditMode) {
              ui.textDest->setText(path.filePath(QFileInfo(folder).fileName()));
            }
          }

        } else {
          file = itemToUpload;

          if (!file.isEmpty()) {
            if (!mIsEditMode) {
              ui.textSource->setText(QDir::toNativeSeparators(file));
              ui.textDest->setText(path.path());
            } else {
              ui.textSource->setText(QDir::toNativeSeparators(file));
            }
          }
        }
      } else {
        // multiple items
        mIsMultiItem = true;
        ui.multiInfo->show();

        if (!mIsEditMode) {
          ui.textDest->setText(path.path());
        }
        // selection control makes sure that all are on the same level so we can
        // use any to derived download root
        QFileInfo source(itemToUpload);
        QDir sourceDir = source.absoluteDir();
        ui.textSource->setText(QDir::toNativeSeparators(sourceDir.path()));

        QStringList includePatternsList;

        for (int i = 0; i < listSelectedUrls.count(); ++i) {

          QString item = listSelectedUrls.at(i).path();

#ifdef Q_OS_WIN
          // remove leading / e.g. "/C:/"
          item.remove(0, 1);
#endif

          QString itemName = QDir(item).dirName();

          /*
           *  rclone filtering uses its own patterns parser
           *  we have to esacape ? [ ] { } * \
           *  otherwise items containing these characters will be missed
           */

          itemName.replace("\\", "\\\\");
          itemName.replace("[", "\\[");
          itemName.replace("]", "\\]");
          itemName.replace("?", "\\?");
          itemName.replace("{", "\\{");
          itemName.replace("}", "\\}");
          itemName.replace("*", "\\*");

          QFileInfo checkItem(item);

          if (checkItem.isFile()) {
            includePatternsList << "+ /" + itemName;
          }

          if (checkItem.isDir()) {
            includePatternsList << "+ /" + itemName + "/**";
            /*
             */
          }
        }

        includePatternsList << "- *";

        QString includeItemsText;
        for (int i = 0; i < includePatternsList.count(); ++i) {

          includeItemsText =
              includeItemsText + includePatternsList.at(i) + "\n";
        }
        ui.textFilter->setPlainText(includeItemsText);
      }

      rcloneCmdUpdate();
    } // if (fileDialog->exec())
  });

  QObject::connect(ui.buttonDefaultSource, &QToolButton::clicked, this, [=]() {
    auto settings = GetSettings();
    QString default_folder =
        (settings->value("Settings/defaultUploadDir").toString());
    // store default folder in lastUsedSourceFolder
    settings->setValue("Settings/lastUsedSourceFolder", default_folder);
    ui.textSource->setText(QDir::toNativeSeparators(default_folder));

    if (!mIsEditMode) {
      ui.textFilter->clear();
      ui.multiInfo->hide();
    }

    if (!default_folder.isEmpty()) {
      if (!mIsEditMode) {
        ui.textDest->setText(
            path.filePath(QFileInfo(default_folder).fileName()));
      }
    } else {
      if (!mIsEditMode) {
        ui.textDest->setText(path.path());
      }
    };
  });

  //!!! QObject::connect(ui.buttonDest
  QObject::connect(ui.buttonDest, &QToolButton::clicked, this, [=]() {
    auto settings = GetSettings();
    QString last_used_dest_folder =
        (settings->value("Settings/lastUsedDestFolder").toString());

    QString folder;

    /*
            FileDialog *fileDialog = new FileDialog(true);
            fileDialog->setWindowTitle("Choose destination directory for
       download"); fileDialog->setMinimumWidth(850);
            fileDialog->setMinimumHeight(525);
    */

    FileDialog fileDialog(true);
    fileDialog.setWindowTitle(
        "Rclone Browser - Download - choose destination directory");
    fileDialog.setMinimumWidth(760);
    fileDialog.setMinimumHeight(470);

    if (fileDialog.exec()) {
      QList<QUrl> listSelectedUrls = fileDialog.selectedUrls();
      folder = listSelectedUrls.at(0).path();

#ifdef Q_OS_WIN
      // remove leading / e.g. "/C:/"
      folder.remove(0, 1);
#endif

      if (!folder.isEmpty()) {
        // store new folder in lastUsedDestFolder
        settings->setValue("Settings/lastUsedDestFolder", folder);
        // if one or many

        if (!mIsMultiItem) {
          if (!mIsEditMode) {
            if (isFolder) {
              ui.textDest->setText(
                  QDir::toNativeSeparators(folder + "/" + path.dirName()));
            } else {
              ui.textDest->setText(QDir::toNativeSeparators(folder));
            }
          } else {
            ui.textDest->setText(QDir::toNativeSeparators(folder));
          }

        } else {
          ui.textDest->setText(QDir::toNativeSeparators(folder));
        }
        rcloneCmdUpdate();
      } // if (!folder.isEmpty()

    } // if (fileDialog->exec())
  });

  QObject::connect(ui.buttonDefaultDest, &QToolButton::clicked, this, [=]() {
    auto settings = GetSettings();
    QString default_folder =
        (settings->value("Settings/defaultDownloadDir").toString());
    // store default_folder in lastUsedDestFolder
    settings->setValue("Settings/lastUsedDestFolder", default_folder);
    if (!default_folder.isEmpty()) {

      if (!mIsMultiItem) {
        if (!mIsEditMode) {

          if (isFolder) {
            ui.textDest->setText(QDir::toNativeSeparators(default_folder + "/" +
                                                          path.dirName()));
          } else {
            ui.textDest->setText(QDir::toNativeSeparators(default_folder));
          }
        } else {
          ui.textDest->setText(QDir::toNativeSeparators(default_folder));
        }

      } else {
        ui.textDest->setText(QDir::toNativeSeparators(default_folder));
      }

    } else {
      ui.textDest->setText("");
    };
  });

  if (!mIsMultiItem || mIsEditMode) {
    ui.multiInfo->hide();

  } else {
    ui.multiInfo->show();
    QString includeItemsText;

    for (int i = 0; i < includeItems.count(); ++i) {
      includeItemsText = includeItemsText + "+ " + includeItems.at(i) + "\n";
    }

    includeItemsText = includeItemsText + "- *\n";
    ui.textFilter->setPlainText(includeItemsText);
  }

  if (!mIsEditMode) {

    if (settings->value("Settings/rememberLastOptions", false).toBool()) {
      settings->beginGroup("Transfer");
      ReadSettings(settings.get(), this);
      settings->endGroup();
    }
  }

  ui.buttonSourceFile->setVisible(!isDownload);
  ui.buttonSourceItems->setVisible(!isDownload);
  ui.buttonDefaultSource->setVisible(!isDownload);

  ui.buttonDest->setVisible(isDownload);

  // not used with new multi items transfers
  //  ui.buttonDefaultDest->hide();
  //  ui.buttonDefaultSource->hide();

  // not in use
  ui.buttonSourceFile->hide();

  if (mRemoteType != "drive") {
    ui.remoteMode->setText(mRemoteType);
  } else {
    if (mRemoteMode == "main") {
      ui.remoteMode->setText("drive");
    }
    if (mRemoteMode == "trash") {
      ui.remoteMode->setText("drive, --drive-trashed-only");
    }
    if (mRemoteMode == "shared") {
      ui.remoteMode->setText("drive, --drive-shared-with-me");
    }
  }

  if (mIsEditMode && mJobOptions != nullptr) {
    // it's not really valid for only one of these things to be true.
    // when operating on an existing instance i.e. a saved task,

    if (isDownload) {
      ui.l_destRemote->hide();
      ui.buttonSourceFile->setVisible(false);
      ui.buttonSourceItems->setVisible(false);
      ui.buttonDefaultSource->hide();
      ui.l_sourceRemote->setEnabled(false);
    } else {
      ui.l_sourceRemote->hide();
      ui.buttonDefaultDest->hide();
      ui.buttonDest->setVisible(false);
      ui.l_destRemote->setEnabled(false);
    }
    putJobOptions();

  } else {

    // set source and destination using defaults
    if (isDownload) {
      // download
      ui.buttonDefaultSource->hide();
      ui.l_destRemote->hide();
      ui.l_sourceRemote->setEnabled(false);

      ui.l_sourceRemote->setText(
          metrix.elidedText(remote + ":", Qt::ElideMiddle, 150));
      ui.l_sourceRemote->setToolTip(remote + ":");

      ui.textSource->setText(path.path());
      QString folder;
      QString default_folder =
          (settings->value("Settings/defaultDownloadDir").toString());
      QString last_used_dest_folder =
          (settings->value("Settings/lastUsedDestFolder").toString());

      if (last_used_dest_folder.isEmpty()) {
        folder = default_folder;
      } else {
        folder = last_used_dest_folder;
      };

      //!!! download
      if (!folder.isEmpty()) {

        if (!mIsMultiItem) {

          if (isFolder) {
            ui.textDest->setText(
                QDir::toNativeSeparators(folder + "/" + path.dirName()));
          } else {
            ui.textDest->setText(QDir::toNativeSeparators(folder));
          }
        } else {
          ui.textDest->setText(QDir::toNativeSeparators(folder));
        }
      }

      ui.textDest->setFocus(Qt::FocusReason::OtherFocusReason);

    } else {
      // upload
      ui.l_sourceRemote->hide();
      ui.l_destRemote->setEnabled(false);
      ui.buttonDefaultDest->hide();
      ui.l_destRemote->setText(
          metrix.elidedText(remote + ":", Qt::ElideMiddle, 150));
      ui.l_destRemote->setToolTip(remote + ":");

      QString folder;
      QString default_folder =
          (settings->value("Settings/defaultUploadDir").toString());
      QString last_used_source_folder =
          (settings->value("Settings/lastUsedSourceFolder").toString());

      if (last_used_source_folder.isEmpty()) {
        folder = default_folder;
      } else {
        folder = last_used_source_folder;
      };

      // if upload initiated from drag and drop we dont use default upload
      // folder

      //!!! upload
      if (!isDrop) {

        if (!isMultiItem) {

          /*
                  ui.textSource->setText(QDir::toNativeSeparators(folder));
                  if (!folder.isEmpty()) {
                    ui.textDest->setText(path.filePath(QFileInfo(folder).fileName()));
                  } else {
                    ui.textDest->setText(path.path());
                  }
          */
          ui.textDest->setText(path.path());

        } else {

          ui.textDest->setText(path.path());
        }

        ui.textSource->setFocus(Qt::FocusReason::OtherFocusReason);

      } else {
        // when dropping to root folder
        if (path.path() == ".") {
          ui.textDest->setText("");
        } else {
          ui.textDest->setText(path.path());
        }
      };
    };
  }

  if (!mIsEditMode) {
    // restore from settings
    if (settings->value("Settings/transferAutoName").toBool()) {
      ui.cb_taskAutoName->setChecked(true);
      run->setText("Save and &Run");
      dryRun->setText("Save and Dry run");
      saveTask->setEnabled(true);
      saveTask->setDefault(true);
    } else {

      ui.cb_taskAutoName->setChecked(false);
      run->setText("&Run");
      dryRun->setText("Dry run");
      saveTask->setEnabled(false);
    }

    ui.cb_taskAddToQueue->setChecked(
        settings->value("Settings/transferAddToQueue").toBool());

  } else {
    ui.cb_taskAutoName->setChecked(false);
    ui.cb_taskAddToQueue->setChecked(false);
  }

  ui.le_defaultRcloneOptions->setText(metrix.elidedText(
      settings->value("Settings/defaultRcloneOptions").toString(),
      Qt::ElideMiddle, 537));
  ui.le_defaultRcloneOptions->setToolTip(
      "Set in preferences:\n\n" +
      settings->value("Settings/defaultRcloneOptions").toString());

  if (isDownload) {
    ui.label_defaultTransferOptions->setText("Default download options:");
    ui.le_defaultTransferOptions->setText(metrix.elidedText(
        settings->value("Settings/defaultDownloadOptions").toString(),
        Qt::ElideMiddle, 537));
    ui.le_defaultTransferOptions->setToolTip(
        "Set in preferences:\n\n" +
        settings->value("Settings/defaultDownloadOptions").toString());

  } else {
    ui.label_defaultTransferOptions->setText("Default upload options:");
    ui.le_defaultTransferOptions->setText(metrix.elidedText(
        settings->value("Settings/defaultUploadOptions").toString(),
        Qt::ElideMiddle, 537));
    ui.le_defaultTransferOptions->setToolTip(
        "Set in preferences:\n\n" +
        settings->value("Settings/defaultUploadOptions").toString());
  }

  QTimer::singleShot(500, Qt::CoarseTimer, this, SLOT(showToolTip()));

  // Task saving/naming has been removed: this dialog only runs transfers.
  // Hide the "Save as a task" group and the Save task button, and force the
  // task controls off so TransferDialog::done() always takes the plain run
  // path and never persists a task (regardless of any leftover
  // transferAutoName setting). "Run" and "Dry run" remain.
  ui.groupBox_3->hide();
  ui.cb_taskAutoName->setChecked(false);
  ui.cb_taskAddToQueue->setChecked(false);
  ui.le_taskName->clear();
  saveTask->hide();
  run->setText("&Run");
  dryRun->setText("Dry run");
}

TransferDialog::~TransferDialog() {
  if (result() == QDialog::Accepted) {
    transferWriteSettings();
  }
}

void TransferDialog::size() { resize(0, maximumHeight()); }

void TransferDialog::setSource(const QString &path) {
  ui.textSource->setText(QDir::toNativeSeparators(path));
}

QString TransferDialog::getMode() const {
  if (ui.rbCopy->isChecked()) {
    return "Copy";
  } else if (ui.rbMove->isChecked()) {
    return "Move";
  } else if (ui.rbSync->isChecked()) {
    return "Sync";
  }

  return QString();
}

QString TransferDialog::getSource() const {
  if (mIsDownload) {
    return mRemote + ":" + ui.textSource->text();
  } else {
    return ui.textSource->text();
  }
}

QString TransferDialog::getDest() const {
  if (mIsDownload) {
    return ui.textDest->text();
  } else {
    return mRemote + ":" + ui.textDest->text();
  }
}

QStringList TransferDialog::getOptions() {
  JobOptions *jobo = getJobOptions(mJobOptions);
  QStringList newWay = jobo->getOptions();
  return newWay;
}

bool TransferDialog::getDryRun() { return mDryRun; }

bool TransferDialog::getAddToQueue() { return mAddToQueue; }

QString TransferDialog::getTaskId() { return mTaskId; }

/*
 * Apply the displayed/edited values on the UI to the
 * JobOptions object.
 *
 * This needs to be edited whenever options are added or changed.
 */
JobOptions *TransferDialog::getJobOptions(JobOptions *taskOptions) {

  //!!!
  //  if (mJobOptions == nullptr)
  //    mJobOptions = new JobOptions(mIsDownload);

  if (ui.rbCopy->isChecked()) {
    taskOptions->operation = JobOptions::Copy;
    taskOptions->sync = false;
  } else if (ui.rbMove->isChecked()) {
    taskOptions->operation = JobOptions::Move;
    taskOptions->sync = false;
  } else if (ui.rbSync->isChecked()) {
    taskOptions->operation = JobOptions::Sync;
  }

  //  taskOptions->dryRun = mDryRun;
  taskOptions->dryRun = false;

  if (ui.rbSync->isChecked()) {
    taskOptions->sync = true;
    switch (ui.cbSyncDelete->currentIndex()) {
    case 0:
      taskOptions->syncTiming = JobOptions::During;
      break;
    case 1:
      taskOptions->syncTiming = JobOptions::After;
      break;
    case 2:
      taskOptions->syncTiming = JobOptions::Before;
      break;
    }
  }

  taskOptions->skipNewer = ui.checkSkipNewer->isChecked();
  taskOptions->skipExisting = ui.checkSkipExisting->isChecked();

  if (ui.checkCompare->isChecked()) {
    taskOptions->compare = true;
    switch (ui.cbCompare->currentIndex()) {
    case 0:
      taskOptions->compareOption = JobOptions::SizeAndModTime;
      break;
    case 1:
      taskOptions->compareOption = JobOptions::Checksum;
      break;
    case 2:
      taskOptions->compareOption = JobOptions::IgnoreSize;
      break;
    case 3:
      taskOptions->compareOption = JobOptions::SizeOnly;
      break;
    case 4:
      taskOptions->compareOption = JobOptions::ChecksumIgnoreSize;
      break;
    }
  } else {
    taskOptions->compare = false;
  };

  //    taskOptions->verbose = ui.checkVerbose->isChecked();
  taskOptions->sameFilesystem = ui.checkSameFilesystem->isChecked();
  taskOptions->dontUpdateModified = ui.checkDontUpdateModified->isChecked();
  taskOptions->noTraverse = ui.noTraverse->isChecked();
  taskOptions->createEmptySrcDirs = ui.createEmptySrcDirs->isChecked();
  taskOptions->deleteEmptySrcDirs = ui.deleteEmptySrcDirs->isChecked();

  taskOptions->transfers = ui.spinTransfers->text();
  taskOptions->checkers = ui.spinCheckers->text();
  taskOptions->bandwidth = ui.textBandwidth->text();
  taskOptions->minSize = ui.textMinSize->text();
  taskOptions->minAge = ui.textMinAge->text();
  taskOptions->maxAge = ui.textMaxAge->text();
  taskOptions->maxDepth = ui.spinMaxDepth->value();

  taskOptions->connectTimeout = ui.spinConnectTimeout->text();
  taskOptions->idleTimeout = ui.spinIdleTimeout->text();
  taskOptions->retries = ui.spinRetries->text();
  taskOptions->lowLevelRetries = ui.spinLowLevelRetries->text();
  taskOptions->deleteExcluded = ui.checkDeleteExcluded->isChecked();

  taskOptions->extra = ui.pte_textExtra->toPlainText().trimmed();
  taskOptions->included = ui.textInclude->toPlainText().trimmed();
  taskOptions->excluded = ui.textExclude->toPlainText().trimmed();
  taskOptions->filtered = ui.textFilter->toPlainText().trimmed();

  taskOptions->isFolder = mIsFolder;

  if (mIsDownload) {
    taskOptions->source = mRemote + ":" + ui.textSource->text();
    taskOptions->dest = ui.textDest->text();
  } else {
    taskOptions->source = ui.textSource->text();
    taskOptions->dest = mRemote + ":" + ui.textDest->text();
  }

  taskOptions->description = ui.le_taskName->text();
  taskOptions->remoteType = mRemoteType;
  taskOptions->remoteMode = mRemoteMode;

  return taskOptions;
}

/*
 * Apply the JobOptions object to the displayed widget values.
 *
 * It could be "better" to use a two-way binding mechanism, but
 * if used that should be global to the app; and anyway doing
 * it this old primitive way makes it easier when the user wants
 * to not save changes...
 */
void TransferDialog::putJobOptions() {
  ui.rbCopy->setChecked(mJobOptions->operation == JobOptions::Copy);
  ui.rbMove->setChecked(mJobOptions->operation == JobOptions::Move);
  ui.rbSync->setChecked(mJobOptions->operation == JobOptions::Sync);

  mDryRun = mJobOptions->dryRun;
  ui.rbSync->setChecked(mJobOptions->sync);

  ui.cbSyncDelete->setCurrentIndex((int)mJobOptions->syncTiming);
  // set combobox tooltips
  ui.cbSyncDelete->setItemData(0, "--delete-during", Qt::ToolTipRole);
  ui.cbSyncDelete->setItemData(1, "--delete-after", Qt::ToolTipRole);
  ui.cbSyncDelete->setItemData(2, "--delete-before", Qt::ToolTipRole);

  ui.checkSkipNewer->setChecked(mJobOptions->skipNewer);
  ui.checkSkipExisting->setChecked(mJobOptions->skipExisting);

  ui.checkCompare->setChecked(mJobOptions->compare);

  ui.cbCompare->setCurrentIndex(mJobOptions->compareOption);
  // set combobox tooltips
  ui.cbCompare->setItemData(0, "default", Qt::ToolTipRole);
  ui.cbCompare->setItemData(1, "--checksum", Qt::ToolTipRole);
  ui.cbCompare->setItemData(2, "--ignore-size", Qt::ToolTipRole);
  ui.cbCompare->setItemData(3, "--size-only", Qt::ToolTipRole);
  ui.cbCompare->setItemData(4, "--checksum --ignore-size", Qt::ToolTipRole);
  // ui.checkVerbose->setChecked(mJobOptions->verbose);

  ui.checkSameFilesystem->setChecked(mJobOptions->sameFilesystem);
  ui.checkDontUpdateModified->setChecked(mJobOptions->dontUpdateModified);
  ui.noTraverse->setChecked(mJobOptions->noTraverse);
  ui.createEmptySrcDirs->setChecked(mJobOptions->createEmptySrcDirs);
  ui.deleteEmptySrcDirs->setChecked(mJobOptions->deleteEmptySrcDirs);

  ui.spinTransfers->setValue(mJobOptions->transfers.toInt());
  ui.spinCheckers->setValue(mJobOptions->checkers.toInt());
  ui.textBandwidth->setText(mJobOptions->bandwidth);
  ui.textMinSize->setText(mJobOptions->minSize);
  ui.textMinAge->setText(mJobOptions->minAge);
  ui.textMaxAge->setText(mJobOptions->maxAge);
  ui.spinMaxDepth->setValue(mJobOptions->maxDepth);

  ui.spinConnectTimeout->setValue(mJobOptions->connectTimeout.toInt());
  ui.spinIdleTimeout->setValue(mJobOptions->idleTimeout.toInt());
  ui.spinRetries->setValue(mJobOptions->retries.toInt());
  ui.spinLowLevelRetries->setValue(mJobOptions->lowLevelRetries.toInt());
  ui.checkDeleteExcluded->setChecked(mJobOptions->deleteExcluded);

  ui.textInclude->setPlainText(mJobOptions->included);
  ui.textExclude->setPlainText(mJobOptions->excluded);
  ui.textFilter->setPlainText(mJobOptions->filtered);

  ui.pte_textExtra->setPlainText(mJobOptions->extra);

  // Elided....Text base measure
  QFontMetrics metrix(ui.l_sourceRemote->font());

  if (mJobOptions->jobType == JobOptions::JobType::Download) {
    ui.l_sourceRemote->setText(metrix.elidedText(
        (mJobOptions->source).left((mJobOptions->source).indexOf(":") + 1),
        Qt::ElideMiddle, 150));
    ui.l_sourceRemote->setToolTip(
        (mJobOptions->source).left((mJobOptions->source).indexOf(":") + 1));

    ui.textSource->setText((mJobOptions->source)
                               .right((mJobOptions->source).length() -
                                      (mJobOptions->source).indexOf(":") - 1));

    ui.textDest->setText(mJobOptions->dest);
  } else {
    ui.textSource->setText(mJobOptions->source);

    ui.l_destRemote->setText(metrix.elidedText(
        (mJobOptions->dest).left((mJobOptions->dest).indexOf(":") + 1),
        Qt::ElideMiddle, 150));
    ui.l_destRemote->setToolTip(
        (mJobOptions->dest).left((mJobOptions->dest).indexOf(":") + 1));

    ui.textDest->setText((mJobOptions->dest)
                             .right((mJobOptions->dest).length() -
                                    (mJobOptions->dest).indexOf(":") - 1));
  }

  ui.le_taskName->setText(mJobOptions->description);

  if (mJobOptions->remoteMode == "") {
    // task saved before googleDriveMode was added
    if (mJobOptions->DriveSharedWithMe) {
      ui.remoteMode->setText("drive, --drive-shared-with-me");
    } else {
      ui.remoteMode->setText("drive");
    }
  } else {
    if (mJobOptions->remoteType != "drive") {
      ui.remoteMode->setText(mJobOptions->remoteType);
    } else {
      if (mJobOptions->remoteMode == "main") {
        ui.remoteMode->setText("drive");
      }
      if (mJobOptions->remoteMode == "trash") {
        ui.remoteMode->setText("drive, --drive-trashed-only");
      }
      if (mJobOptions->remoteMode == "shared") {
        ui.remoteMode->setText("drive, --drive-shared-with-me");
      }
    }
  }
}

void TransferDialog::done(int r) {

  if (r == QDialog::Accepted) {
    if (mIsDownload) {
      if (ui.textDest->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter destination.");
        ui.textDest->setFocus(Qt::FocusReason::OtherFocusReason);
        return;
      }
    } else {
      if (ui.textSource->text().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please enter source.");
        ui.textSource->setFocus(Qt::FocusReason::OtherFocusReason);
        return;
      }
    }
    if (!mIsEditMode) {
      save_AutoName_AddToQueue();
    }

    if (ui.cb_taskAutoName->isChecked() ||
        !ui.le_taskName->text().trimmed().isEmpty()) {
      // save before run
      generateAutoTaskName();
      if (saveTaskToFile()) {
        mAddToQueue = ui.cb_taskAddToQueue->isChecked();
      } else {
        return;
      }
    }
  }
  QDialog::done(r);
}

void TransferDialog::save_AutoName_AddToQueue(void) {

  auto settings = GetSettings();

  if (ui.cb_taskAutoName->isChecked()) {
    settings->setValue("Settings/transferAutoName", "true");
  } else {
    settings->setValue("Settings/transferAutoName", "false");
  }

  if (ui.cb_taskAddToQueue->isChecked()) {
    settings->setValue("Settings/transferAddToQueue", "true");
  } else {
    settings->setValue("Settings/transferAddToQueue", "false");
  }
}

bool TransferDialog::saveTaskToFile() {

  // only generated if needed
  generateAutoTaskName();

  // validate before saving task...
  if (ui.le_taskName->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, "Error", "Please enter task name to save.");
    ui.tabWidget->setCurrentIndex(0);
    ui.le_taskName->setFocus(Qt::FocusReason::OtherFocusReason);
    return false;
  }

  // even though the below does not match the condition on the Run buttons
  // it SEEMS like blanking either one would be a problem, right?
  if (mIsDownload) {
    if (ui.textDest->text().isEmpty()) {
      QMessageBox::warning(this, "Error",
                           "Invalid task, destination is required.");
      ui.tabWidget->setCurrentIndex(0);
      ui.textDest->setFocus(Qt::FocusReason::OtherFocusReason);
      return false;
    }
  } else {
    if (ui.textSource->text().isEmpty()) {
      QMessageBox::warning(this, "Error", "Invalid task, source is  required.");
      ui.tabWidget->setCurrentIndex(0);
      ui.textSource->setFocus(Qt::FocusReason::OtherFocusReason);
      return false;
    }
  }

  if (!mIsEditMode) {
    save_AutoName_AddToQueue();
    transferWriteSettings();
  }

  JobOptions *jobo = getJobOptions(mJobOptions);
  ListOfJobOptions::getInstance()->Persist(jobo);

  mTaskId = jobo->uniqueId.toString();

  return true;
}

void TransferDialog::transferWriteSettings() {
  auto settings = GetSettings();

  settings->beginGroup("Transfer");

  WriteSettings(settings.get(), this);
  settings->remove("textSource");
  settings->remove("textDest");
  settings->remove("cb_taskAddToQueue");
  settings->remove("cb_taskAutoName");
  settings->remove("le_taskName");
  settings->remove("le_defaultRcloneOptions");
  settings->remove("le_defaultTransferOptions");
  settings->remove("textFilter");
  settings->remove("rcloneCmd");
  settings->endGroup();
}

void TransferDialog::generateAutoTaskName() {

  if (ui.cb_taskAutoName->isChecked() &&
      ui.le_taskName->text().trimmed().isEmpty()) {
    // generate auto task name

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();

    QString name = "_tmp_" + date.toString("ddMMMyyyy") + "_" +
                   time.toString("HHmmss") + "_";

    if (mIsDownload) {
      name = name + mRemote;
    } else {
      name = name + mRemote;
    }

    ui.le_taskName->setText(name);
  }
}

void TransferDialog::rcloneCmdUpdate() {

  // use temporary JobOptions object to get rclone options
  QStringList rcloneOptions;
  //  if (mJobOptionsRcloneCmd == nullptr)
  //    mJobOptionsRcloneCmd = new JobOptions(mIsDownload);
  JobOptions *jobo = getJobOptions(mJobOptionsRcloneCmd);
  rcloneOptions = jobo->getOptions();

  ui.rcloneCmd->setPlainText(GetRcloneCmd(rcloneOptions).join(" "));
}

void TransferDialog::showToolTip() {

  if (!mIsDownload) {
    if (!mIsEditMode) {
      QToolTip::showText(ui.buttonSourceItems->mapToGlobal(QPoint(-195, 4)),
                         "Choose items to upload");
    }
    ui.buttonSourceItems->setToolTip("Choose items to upload");
  } else {
    if (!mIsEditMode) {
      QToolTip::showText(ui.buttonDest->mapToGlobal(QPoint(-230, 4)),
                         "Choose destination directory");
    }
    ui.buttonDest->setToolTip("Choose destination directory");
  }

  // Choose items to upload
  // Choose download destination folder
}
