#include "mount_dialog.h"
#include "global.h"
#include "list_of_job_options.h"
#include "utils.h"

MountDialog::MountDialog(const QString &remote, const QDir &path,
                         const QString &remoteType, const QString &remoteMode,
                         QWidget *parent, JobOptions *task, bool editMode)
    : QDialog(parent), mIsEditMode(editMode), mJobOptions(task) {

  ui.setupUi(this);

  mRemoteMode = remoteMode;
  mRemoteType = remoteType;

  auto settings = GetSettings();

  // Elided....Text base measure
  QFontMetrics metrix(ui.label_remote->font());

  // set minimumWidth based on font size
  int fontsize = 0;
  fontsize = (settings->value("Settings/fontSize").toInt());
  setMinimumWidth(minimumWidth() + (fontsize * 30));
  QTimer::singleShot(0, this, SLOT(shrink()));

#if !defined(Q_OS_WIN)
  ui.groupBox_Win->hide();
  QTimer::singleShot(0, this, SLOT(shrink()));
#endif

#if defined(Q_OS_WIN)
  ui.groupBox_notWin->hide();
  QTimer::singleShot(0, this, SLOT(shrink()));

  if (!mIsEditMode) {
    ui.le_rcPort->setText("0");

    ui.label_mountBase->setDisabled(true);
    ui.le_mountBase->setDisabled(true);
    ui.le_mountBaseBrowse->setDisabled(true);
    ui.label_mountPointWin->setDisabled(true);
    ui.le_mountPointWin->setDisabled(true);
    ui.label_mountPointWinInfo->setDisabled(true);

    ui.label_driveLetter->setDisabled(false);
    ui.combo_driveLetter->setDisabled(false);
    ui.label_driveLetterInfo->setDisabled(false);

  } else {
    // restore from saved job
    putJobOptions();

    if (ui.cb_driveLetter->isChecked()) {

      ui.label_mountBase->setDisabled(true);
      ui.le_mountBase->setDisabled(true);
      ui.le_mountBaseBrowse->setDisabled(true);
      ui.label_mountPointWin->setDisabled(true);
      ui.le_mountPointWin->setDisabled(true);
      ui.label_mountPointWinInfo->setDisabled(true);

      ui.label_driveLetter->setDisabled(false);
      ui.combo_driveLetter->setDisabled(false);
      ui.label_driveLetterInfo->setDisabled(false);

    } else {

      ui.label_mountBase->setDisabled(false);
      ui.le_mountBase->setDisabled(false);
      ui.le_mountBaseBrowse->setDisabled(false);
      ui.label_mountPointWin->setDisabled(false);
      ui.le_mountPointWin->setDisabled(false);
      ui.label_mountPointWinInfo->setDisabled(false);

      ui.label_driveLetter->setDisabled(true);
      ui.combo_driveLetter->setDisabled(true);
      ui.label_driveLetterInfo->setDisabled(true);
    }
  }

  // used drives' letters
  QStringList disksUsed;
  int firstDiskFreeIndex = 0;
  bool freeLetterFound = false;

  // initailize drive letters
  QStringList drivesList;
  for (char l = 'A'; l <= 'Z'; ++l) {
    drivesList << QString(l);
  }
  ui.combo_driveLetter->addItems(drivesList);

  // in edit mode we want all letters available
  if (!mIsEditMode) {
    // get used drives' letters

    // get mounted drives (missing CD ROM but gets mounts)
    foreach (QFileInfo drive, QDir::drives()) {
      if (!QString(drive.absolutePath().front()).isEmpty()) {
        disksUsed << QString(drive.absolutePath().front());
      }
    }

    // get all local storage drives (catches CD ROM but missing mounts)
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
      if (storage.isValid()) {
        disksUsed << QString(storage.rootPath().front());
      }
    }

    // disable used drive letters in combo box
    QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>(ui.combo_driveLetter->model());
    Q_ASSERT(model != nullptr);
    int j = 0;
    for (const auto &i : drivesList) {
      if (disksUsed.contains(i)) {
        QStandardItem *item = model->item(j);
        // disable item
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
      } else {
        if (!freeLetterFound) {
          firstDiskFreeIndex = j;
          freeLetterFound = true;
        }
      }
      j++;
    }
  }

#endif

  setWindowTitle("Mount remote");

  ui.le_defaultMountOptions->setText(metrix.elidedText(
      settings->value("Settings/mount").toString(), Qt::ElideMiddle, 537));
  ui.le_defaultMountOptions->setToolTip(
      "Set in preferences:\n\n" + settings->value("Settings/mount").toString());

  // set combo box tooltips
  ui.combob_cacheLevel->setItemData(0, "default", Qt::ToolTipRole);
  ui.combob_cacheLevel->setItemData(1, "--vfs-cache-mode minimal",
                                    Qt::ToolTipRole);
  ui.combob_cacheLevel->setItemData(2, "--vfs-cache-mode writes",
                                    Qt::ToolTipRole);
  ui.combob_cacheLevel->setItemData(3, "--vfs-cache-mode full",
                                    Qt::ToolTipRole);

  ui.le_rcPort->setValidator(new QIntValidator(1024, 65535, this));

  if (!mIsEditMode) {
    mTarget = remote + ":" + path.path();
    ui.remoteToMount->setText(mTarget);
  } else {
    putJobOptions();
  }

  ui.remoteToMount->setCursorPosition(0);

  if (!mIsEditMode) {

    if (settings->value("Settings/rememberLastOptions", false).toBool()) {
      settings->beginGroup("MountDialog");
      ReadSettings(settings.get(), this);
      settings->endGroup();
    }

#if defined(Q_OS_WIN)
    if (ui.cb_driveLetter->isChecked()) {

      ui.label_mountBase->setDisabled(true);
      ui.le_mountBase->setDisabled(true);
      ui.le_mountBaseBrowse->setDisabled(true);
      ui.label_mountPointWin->setDisabled(true);
      ui.le_mountPointWin->setDisabled(true);
      ui.label_mountPointWinInfo->setDisabled(true);

      ui.label_driveLetter->setDisabled(false);
      ui.combo_driveLetter->setDisabled(false);
      ui.label_driveLetterInfo->setDisabled(false);
    }

    if (ui.cb_mountPointWin->isChecked()) {

      ui.label_mountBase->setDisabled(false);
      ui.le_mountBase->setDisabled(false);
      ui.le_mountBaseBrowse->setDisabled(false);
      ui.label_mountPointWin->setDisabled(false);
      ui.le_mountPointWin->setDisabled(false);
      ui.label_mountPointWinInfo->setDisabled(false);

      ui.label_driveLetter->setDisabled(true);
      ui.combo_driveLetter->setDisabled(true);
      ui.label_driveLetterInfo->setDisabled(true);
    }

    // if letter already used switch to firstDiskFreeIndex
    if (freeLetterFound && disksUsed.contains(ui.combo_driveLetter->currentText())) {
      ui.combo_driveLetter->setCurrentIndex(firstDiskFreeIndex);
    }

#endif
  }

  QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, this,
                   &QDialog::accept);

  QObject::connect(ui.buttonBox, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);

  QObject::connect(ui.le_mountScript, &QLineEdit::textChanged, this, [=]() {
    if (!ui.le_mountScript->text().trimmed().isEmpty() &&
        ui.le_rcPort->text().trimmed().isEmpty()) {
      ui.le_rcPort->setText("0");
    }
  });

  QObject::connect(ui.tb_mountScriptBrowse, &QPushButton::clicked, this, [=]() {
    QString mountScript = QFileDialog::getOpenFileName(
        this, "Select script", ui.le_mountScript->text());

    if (mountScript.isEmpty()) {
      return;
    }

    if (!QFileInfo(mountScript).isExecutable()) {
      QMessageBox::critical(
          this, "Error",
          QString("File\n\n %1\n\n is not executable.").arg(mountScript));
      return;
    }

#if !defined(Q_OS_WIN)
    if (ui.le_rcPort->text().isEmpty()) {
      ui.le_rcPort->setText("0");
    }
#endif

    ui.le_mountScript->setText(mountScript);
  });

  QPushButton *saveTask = ui.buttonBox->addButton(
      "&Save task", QDialogButtonBox::ButtonRole::ActionRole);

  if (!mIsEditMode) {
    ui.buttonBox->addButton("&Run mount", QDialogButtonBox::AcceptRole);
  }

  /*  QPushButton *cancelButton = ui.buttonBox->addButton(
        "&Cancel", QDialogButtonBox::ButtonRole::RejectRole);

    cancelButton->setFocus();
  */

  QObject::connect(saveTask, &QPushButton::clicked, this, [=]() {
    // validate before saving task...
    if (!validateOptions()) {
      return;
    }

    if (ui.taskName->text().isEmpty()) {
      QMessageBox::warning(this, "Warning", "Please enter task name to Save!");
      ui.tabWidget->setCurrentIndex(0);
      ui.taskName->setFocus(Qt::FocusReason::OtherFocusReason);
      return;
    }

    /*
        // even though the below does not match the condition on the Run buttons
        // it SEEMS like blanking either one would be a problem, right?

        if (ui.textDest->text().isEmpty() || ui.textSource->text().isEmpty()) {
          QMessageBox::warning(this, "Error",
                               "Invalid Task, source and destination
       required!"); return;
        }
        JobOptions *jobo = getJobOptions();
        ListOfJobOptions::getInstance()->Persist(jobo);
        // always close on save
        // if (mIsEditMode)
    */

    mountOptionsWriteSettings();

    getOptions();
    JobOptions *jobo = mJobOptions;
    { ListOfJobOptions::getInstance()->Persist(jobo); }
    this->close();
  });

  /*
  QStringList TransferDialog::getOptions() {
    JobOptions *jobo = getJobOptions();
    QStringList newWay = jobo->getOptions();
    return newWay;
  }
  */

  QObject::connect(ui.cb_driveLetter, &QCheckBox::clicked, this, [=]() {
    if (ui.cb_driveLetter->isChecked()) {

      ui.label_mountBase->setDisabled(true);
      ui.le_mountBase->setDisabled(true);
      ui.le_mountBaseBrowse->setDisabled(true);
      ui.label_mountPointWin->setDisabled(true);
      ui.le_mountPointWin->setDisabled(true);
      ui.label_mountPointWinInfo->setDisabled(true);

      ui.label_driveLetter->setDisabled(false);
      ui.combo_driveLetter->setDisabled(false);
      ui.label_driveLetterInfo->setDisabled(false);

    } else {
    }
    //    ui.sizeOnly->setDisabled(false);
  });

  QObject::connect(ui.cb_mountPointWin, &QCheckBox::clicked, this, [=]() {
    if (ui.cb_mountPointWin->isChecked()) {

      ui.label_mountBase->setDisabled(false);
      ui.le_mountBase->setDisabled(false);
      ui.le_mountBaseBrowse->setDisabled(false);
      ui.label_mountPointWin->setDisabled(false);
      ui.le_mountPointWin->setDisabled(false);
      ui.label_mountPointWinInfo->setDisabled(false);

      ui.label_driveLetter->setDisabled(true);
      ui.combo_driveLetter->setDisabled(true);
      ui.label_driveLetterInfo->setDisabled(true);

    } else {
    }
    //    ui.download->setDisabled(false);
  });

  QObject::connect(ui.le_mountBaseBrowse, &QToolButton::clicked, this, [=]() {
    QString sourceDir =
        QFileDialog::getExistingDirectory(this, "Choose mounting base folder");
    if (!sourceDir.isEmpty()) {
      ui.le_mountBase->setText(QDir::toNativeSeparators(sourceDir));
      ui.le_mountBase->setCursorPosition(0);
    }
  });

  QObject::connect(ui.tb_mountPointNotWin, &QToolButton::clicked, this, [=]() {
    QString sourceDir =
        QFileDialog::getExistingDirectory(this, "Choose mounting point");
    if (!sourceDir.isEmpty()) {
      ui.le_mountPointNotWin->setText(QDir::toNativeSeparators(sourceDir));
      ui.le_mountPointNotWin->setCursorPosition(0);
    }
  });
}

MountDialog::~MountDialog() {}

void MountDialog::shrink() {
  resize(0, 0);
  adjustSize();
  setMaximumHeight(this->height());
}

#if !defined(Q_OS_WIN)
QString MountDialog::getMountPoint() const {
  return ui.le_mountPointNotWin->text();
}
#endif

#if defined(Q_OS_WIN)
QString MountDialog::getMountPoint() const {
  QString mountPointWindows;
  if (ui.cb_driveLetter->isChecked()) {
    // drive letter
    mountPointWindows = ui.combo_driveLetter->currentText() + ":";
  } else {
    // base/mountpoint
    mountPointWindows = ui.le_mountBase->text().trimmed() + '\\' +
                        ui.le_mountPointWin->text().trimmed();
  }
  return mountPointWindows;
}
#endif

QString MountDialog::getScript() const {
  return ui.le_mountScript->text().trimmed();
}

QStringList MountDialog::getOptions() {

  /*
   * Apply the displayed/edited values on the UI to the
   * JobOptions object.
   *
   * This needs to be edited whenever options are added or changed.
   */

  if (mJobOptions == nullptr)
    mJobOptions = new JobOptions(true);

  QStringList list;

  list << "mount";
  mJobOptions->operation = JobOptions::Mount;

  list << mTarget;
  mJobOptions->source = mTarget;

#if !defined(Q_OS_WIN)
  list << ui.le_mountPointNotWin->text();
  mJobOptions->dest = ui.le_mountPointNotWin->text();
#endif

#if defined(Q_OS_WIN)
  mJobOptions->mountWinDriveMode = ui.cb_driveLetter->isChecked();
  if (ui.cb_driveLetter->isChecked()) {
    // drive letter
    list << ui.combo_driveLetter->currentText() + ":";
    mJobOptions->dest = ui.combo_driveLetter->currentText() + ":";
  } else {
    // base/mountpoint
    list << ui.le_mountBase->text().trimmed() + '\\' +
                ui.le_mountPointWin->text().trimmed();
    mJobOptions->dest = ui.le_mountBase->text().trimmed() + '\\' +
                        ui.le_mountPointWin->text().trimmed();
  }
#endif
  mJobOptions->mountRcPort = ui.le_rcPort->text().trimmed();
  if (!ui.le_rcPort->text().isEmpty()) {

    list << "--rc";
    list << "--rc-addr";
    list << "localhost:" + QVariant(ui.le_rcPort->text().toInt()).toString();
    mJobOptions->mountRcPort =
        QVariant(ui.le_rcPort->text().toInt()).toString();

    // generate random username and password
    const QString possibleCharacters(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

    QString rcUser;
    for (int i = 0; i < 10; ++i) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
      int index =
          QRandomGenerator::global()->generate() % possibleCharacters.length();
#else
      int index = qrand() % possibleCharacters.length();
#endif

      QChar nextChar = possibleCharacters.at(index);
      rcUser.append(nextChar);
    }

    QString rcPass;
    for (int i = 0; i < 22; ++i) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
      int index =
          QRandomGenerator::global()->generate() % possibleCharacters.length();
#else
      int index = qrand() % possibleCharacters.length();
#endif

      QChar nextChar = possibleCharacters.at(index);
      rcPass.append(nextChar);
    }

    list << "--rc-user=" + rcUser;
    list << "--rc-pass=" + rcPass;
  }

  if (mRemoteType == "drive") {

    if (mRemoteMode == "shared") {
      list << "--drive-shared-with-me";
      if (!ui.cb_readOnly->isChecked()) {
        list << "--read-only";
      }
    }

    if (mRemoteMode == "trash") {
      list << "--drive-trashed-only";
    }
  }

  mJobOptions->remoteMode = mRemoteMode;
  mJobOptions->remoteType = mRemoteType;

  if (ui.cb_readOnly->isChecked()) {
    list << "--read-only";
  }
  mJobOptions->mountReadOnly = ui.cb_readOnly->isChecked();

  mJobOptions->mountVolume = ui.le_volumeName->text().trimmed();
  if (!(ui.le_volumeName->text().trimmed().isEmpty())) {
    list << "--volname";
    list << ui.le_volumeName->text().trimmed();
  }

  switch (ui.combob_cacheLevel->currentIndex()) {
  case 0:
    mJobOptions->mountCacheLevel = JobOptions::Off;
    break;
  case 1:
    list << "--vfs-cache-mode";
    list << "minimal";
    mJobOptions->mountCacheLevel = JobOptions::Minimal;
    break;
  case 2:
    list << "--vfs-cache-mode";
    list << "writes";
    mJobOptions->mountCacheLevel = JobOptions::Writes;
    break;
  case 3:
    list << "--vfs-cache-mode";
    list << "full";
    mJobOptions->mountCacheLevel = JobOptions::Full;
    break;
  }

  mJobOptions->extra = ui.textExtra->toPlainText().trimmed();
  if (!ui.textExtra->toPlainText().trimmed().isEmpty()) {
    for (auto line : ui.textExtra->toPlainText().trimmed().split('\n')) {
      if (!line.isEmpty()) {

        for (QString arg :
             line.split(QRegularExpression(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
          if (!arg.isEmpty()) {
            list << arg.replace("\"", "");
          }
        }
      }
    }
  }

  mJobOptions->mountAutoStart = ui.cb_taskAutostart->isChecked();
  mJobOptions->mountScript = ui.le_mountScript->text().trimmed();
  mJobOptions->description = ui.taskName->text();
  return list;
}

bool MountDialog::validateOptions() {

#if !defined(Q_OS_WIN)
  if (ui.le_mountPointNotWin->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, "Warning",
                         "Please enter mount point directory!");

    ui.tabWidget->setCurrentIndex(0);
    ui.le_mountPointNotWin->setFocus(Qt::FocusReason::OtherFocusReason);
    return false;
  }
#endif

// only on Windows RC mode is mandatory (for unmount to work)
#if !defined(Q_OS_WIN)
  if (!ui.le_mountScript->text().trimmed().isEmpty() &&
      ui.le_rcPort->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, "Warning",
                         "Post mount script requires RC port.\n\nAllowed RC "
                         "ports are between 1024 and "
                         "65535 or 0 for auto.");
    ui.tabWidget->setCurrentIndex(2);
    ui.le_rcPort->setFocus(Qt::FocusReason::OtherFocusReason);
    return false;
  }

  if (!ui.le_rcPort->text().trimmed().isEmpty()) {
    if ((ui.le_rcPort->text().trimmed().toInt() < 1024 &&
         ui.le_rcPort->text().trimmed().toInt() > 0) ||
        ui.le_rcPort->text().trimmed().toInt() > 65535) {
      QMessageBox::warning(
          this, "Warning",
          "Wrong RC port.\n\nAllowed RC ports are between 1024 and "
          "65535, 0 for auto or leave it empty.");
      ui.tabWidget->setCurrentIndex(2);
      ui.le_rcPort->setFocus(Qt::FocusReason::OtherFocusReason);
      return false;
    }
  }
#endif

#if defined(Q_OS_WIN)
  if ((ui.le_rcPort->text().trimmed().toInt() < 1024 &&
       ui.le_rcPort->text().trimmed().toInt() > 0) ||
      ui.le_rcPort->text().trimmed().toInt() > 65535 ||
      ui.le_rcPort->text().trimmed().isEmpty()) {
    QMessageBox::warning(this, "Warning",
                         "Wrong RC port.\n\nAllowed RC ports are between "
                         "1024 and 65535, or 0 for auto.");
    ui.tabWidget->setCurrentIndex(2);
    ui.le_rcPort->setFocus(Qt::FocusReason::OtherFocusReason);
    return false;
  }

  if (!ui.cb_driveLetter->isChecked()) {

    if (ui.le_mountBase->text().trimmed().isEmpty() ||
        ui.le_mountPointWin->text().trimmed().isEmpty()) {

      QMessageBox::warning(
          this, "Warning",
          "Please enter mount base and mount point directories.");
      ui.tabWidget->setCurrentIndex(0);

      if (ui.le_mountPointWin->text().trimmed().isEmpty()) {
        ui.le_mountPointWin->setFocus(Qt::FocusReason::OtherFocusReason);
      }

      if (ui.le_mountBase->text().trimmed().isEmpty()) {

        ui.le_mountBase->setFocus(Qt::FocusReason::OtherFocusReason);
      }

      return false;
    }
  }
#endif

  return true;
}

void MountDialog::done(int r) {
  if (r == QDialog::Accepted) {
    // validate options before run
    if (!validateOptions()) {
      return;
    }
    mountOptionsWriteSettings();
  }

  QDialog::done(r);
}

/*
 * Apply the JobOptions object to the displayed widget values.
 *
 * It could be "better" to use a two-way binding mechanism, but
 * if used that should be global to the app; and anyway doing
 * it this old primitive way makes it easier when the user wants
 * to not save changes...
 */
void MountDialog::putJobOptions() {

  ui.remoteToMount->setText(mJobOptions->source);
  mTarget = mJobOptions->source;
  ui.remoteToMount->setCursorPosition(0);

#if defined(Q_OS_WIN)

  if (mJobOptions->mountWinDriveMode) {
    ui.cb_driveLetter->setChecked(true);

    ui.combo_driveLetter->setCurrentIndex((mJobOptions->dest).at(0).toLatin1() -
                                          65);
  } else {
    ui.cb_mountPointWin->setChecked(true);

    ui.le_mountBase->setText(
        (mJobOptions->dest).left((mJobOptions->dest).lastIndexOf("\\")));
    ui.le_mountPointWin->setText(
        (mJobOptions->dest)
            .right((mJobOptions->dest).length() -
                   (mJobOptions->dest).lastIndexOf("\\") - 1));
  }

#endif

#if !defined(Q_OS_WIN)
  ui.le_mountPointNotWin->setText(mJobOptions->dest);
  ui.le_mountPointNotWin->setCursorPosition(0);
#endif

  //  ui.checkCompare->setChecked(mJobOptions->compare);

  ui.cb_readOnly->setChecked(mJobOptions->mountReadOnly);

  ui.combob_cacheLevel->setCurrentIndex((int)mJobOptions->mountCacheLevel);

  ui.le_volumeName->setText(mJobOptions->mountVolume);

  ui.taskName->setText(mJobOptions->description);

  ui.cb_taskAutostart->setChecked(mJobOptions->mountAutoStart);

  ui.textExtra->setPlainText(mJobOptions->extra);

  ui.le_mountScript->setText(mJobOptions->mountScript);

  ui.le_rcPort->setText(mJobOptions->mountRcPort);

  return;
}

void MountDialog::mountOptionsWriteSettings() {

  auto settings = GetSettings();
  settings->beginGroup("MountDialog");
  WriteSettings(settings.get(), this);
  settings->remove("remoteToMount");
  settings->remove("taskName");
  settings->remove("cb_taskAutostart");
  settings->endGroup();
}
