#include "remote_widget.h"
#include "check_dialog.h"
#include "dedupe_dialog.h"
#include "delete_progress_dialog.h"
#include "export_dialog.h"
#include "global.h"
#include "icon_cache.h"
#include "item_model.h"
#include "list_of_job_options.h"
#include "mount_dialog.h"
#include "progress_dialog.h"
#include "remote_folder_dialog.h"
#include "transfer_dialog.h"
#include "utils.h"

RemoteWidget::RemoteWidget(IconCache *iconCache, const QString &remote,
                           const QString &remoteType, QWidget *parent)
    : QWidget(parent) {

  ui.setupUi(this);

  ui.frameTools->hide();
  ui.elidedMeasure->hide();

  bool isLocal = remoteType == "local";
  bool isGoogle = remoteType == "drive";
  mRemoteType = remoteType;

  QString root = isLocal ? "/" : QString();

  QString remoteMode = "main";
  //  QString remoteMode_ = "main";

  auto settings = GetSettings();

#ifndef Q_OS_WIN
  isLocal = false;
#endif

#ifdef Q_OS_WIN
  // check if required rclone version
  QString rcloneVersion = settings->value("Settings/rcloneVersion").toString();
  unsigned int rcloneVersionResult =
      compareVersion(rcloneVersion.toStdString(), "1.50");

  // as with Fusion style in Windows QTreeView font size does not scale
  // with QApplication::font() changes we control it manually using style sheet

  QFont defaultFont = QApplication::font();
  int fontSize = defaultFont.pointSize() + 3;

  QString fontStyleSheet =
      QString("QTreeView { font-size: %1px;}").arg(fontSize);
  ui.tree->setStyleSheet(fontStyleSheet);
#endif

  if (settings->value("Settings/preemptiveLoading").toBool()) {
    mPreemptiveLoading = true;
  } else {
    mPreemptiveLoading = false;
  }

  int preemptiveLoadingLevel =
      settings->value("Settings/preemptiveLoadingLevel").toInt();
  if (preemptiveLoadingLevel == 0) {
    mMaxRcloneLsProcessCount = 10;
  }
  if (preemptiveLoadingLevel == 1) {
    mMaxRcloneLsProcessCount = 20;
  }
  if (preemptiveLoadingLevel == 2) {
    mMaxRcloneLsProcessCount = 40;
  }

  QString buttonStyle = settings->value("Settings/buttonStyle").toString();
  QString buttonSize = settings->value("Settings/buttonSize").toString();
  QString iconsColour = settings->value("Settings/iconsColour").toString();
  settings->setValue("Settings/remoteMode", "main");
  ui.tree->setAlternatingRowColors(
      settings->value("Settings/rowColors", false).toBool());

  ui.cb_GoogleDriveMode->setDisabled(!isGoogle);
  // hide cb_GoogleDriveMode and Dedupe button for non Google remotes
  if (!isGoogle) {
    ui.cb_GoogleDriveMode->hide();
    ui.buttonDedupe->hide();
  }

  QString img_add = "";

  if (iconsColour == "white") {
    img_add = "_inv";
  }

  ui.refresh->setIcon(
      QIcon(":media/images/qbutton_icons/refresh" + img_add + ".png"));
  ui.mkdir->setIcon(
      QIcon(":media/images/qbutton_icons/mkdir" + img_add + ".png"));
  ui.rename->setIcon(
      QIcon(":media/images/qbutton_icons/rename" + img_add + ".png"));
  ui.move->setIcon(
      QIcon(":media/images/qbutton_icons/move" + img_add + ".png"));
  ui.copy->setIcon(
      QIcon(":media/images/qbutton_icons/copy" + img_add + ".png"));
  ui.purge->setIcon(
      QIcon(":media/images/qbutton_icons/purge" + img_add + ".png"));
  ui.actionNewMount->setIcon(
      QIcon(":media/images/qbutton_icons/mount" + img_add + ".png"));
  ui.stream->setIcon(
      QIcon(":media/images/qbutton_icons/stream" + img_add + ".png"));
  ui.upload->setIcon(
      QIcon(":media/images/qbutton_icons/upload" + img_add + ".png"));
  ui.download->setIcon(
      QIcon(":media/images/qbutton_icons/download" + img_add + ".png"));
  ui.actionCheck->setIcon(
      QIcon(":media/images/qbutton_icons/check" + img_add + ".png"));
  ui.getSize->setIcon(
      QIcon(":media/images/qbutton_icons/getsize" + img_add + ".png"));
  ui.getTree->setIcon(
      QIcon(":media/images/qbutton_icons/gettree" + img_add + ".png"));
  ui.link->setIcon(
      QIcon(":media/images/qbutton_icons/link" + img_add + ".png"));
  ui.export_->setIcon(
      QIcon(":media/images/qbutton_icons/export" + img_add + ".png"));
  ui.buttonTools->setIcon(
      QIcon(":media/images/qbutton_icons/tools" + img_add + ".png"));
  ui.getInfo->setIcon(
      QIcon(":media/images/qbutton_icons/info" + img_add + ".png"));
  ui.actionDedupe->setIcon(
      QIcon(":media/images/qbutton_icons/dedupe" + img_add + ".png"));
  ui.cleanup->setIcon(
      QIcon(":media/images/qbutton_icons/cleanup" + img_add + ".png"));

  ui.buttonRefresh->setDefaultAction(ui.refresh);
  ui.buttonMkdir->setDefaultAction(ui.mkdir);
  ui.buttonRename->setDefaultAction(ui.rename);
  ui.buttonCopy->setDefaultAction(ui.copy);
  ui.buttonMove->setDefaultAction(ui.move);
  ui.buttonPurge->setDefaultAction(ui.purge);
  ui.buttonMount->setDefaultAction(ui.actionNewMount);
  ui.buttonStream->setDefaultAction(ui.stream);
  ui.buttonUpload->setDefaultAction(ui.upload);
  ui.buttonDownload->setDefaultAction(ui.download);
  ui.buttonCheck->setDefaultAction(ui.actionCheck);
  ui.buttonSize->setDefaultAction(ui.getSize);
  ui.buttonTree->setDefaultAction(ui.getTree);
  ui.buttonLink->setDefaultAction(ui.link);
  ui.buttonExport->setDefaultAction(ui.export_);
  ui.buttonInfo->setDefaultAction(ui.getInfo);

  // buttons and icons size
  int icon_w = 16;
  int icon_h = 16;
  if (buttonSize == "0") {
    icon_w = 24;
  }
  if (buttonSize == "1") {
    icon_w = 32;
  }
  if (buttonSize == "2") {
    icon_w = 48;
  }
  if (buttonSize == "3") {
    icon_w = 72;
  }
  if (buttonSize == "4") {
    icon_w = 96;
  }
  icon_h = icon_w;
  int button_width = 61;

  if (buttonStyle == "textandicon") {
    ui.buttonRefresh->setIconSize(QSize(icon_w, icon_h));
    ui.buttonMkdir->setIconSize(QSize(icon_w, icon_h));
    ui.buttonRename->setIconSize(QSize(icon_w, icon_h));
    ui.buttonMove->setIconSize(QSize(icon_w, icon_h));
    ui.buttonPurge->setIconSize(QSize(icon_w, icon_h));
    ui.buttonMount->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStream->setIconSize(QSize(icon_w, icon_h));
    ui.buttonUpload->setIconSize(QSize(icon_w, icon_h));
    ui.buttonDownload->setIconSize(QSize(icon_w, icon_h));
    ui.buttonSize->setIconSize(QSize(icon_w, icon_h));
    ui.buttonTree->setIconSize(QSize(icon_w, icon_h));
    ui.buttonLink->setIconSize(QSize(icon_w, icon_h));
    ui.buttonExport->setIconSize(QSize(icon_w, icon_h));
    ui.buttonCheck->setIconSize(QSize(icon_w, icon_h));

    ui.buttonRefresh->setMinimumWidth(button_width);
    ui.buttonMkdir->setMinimumWidth(button_width);
    ui.buttonRename->setMinimumWidth(button_width);
    ui.buttonMove->setMinimumWidth(button_width);
    ui.buttonPurge->setMinimumWidth(button_width);
    ui.buttonMount->setMinimumWidth(button_width);
    ui.buttonStream->setMinimumWidth(button_width);
    ui.buttonUpload->setMinimumWidth(button_width * 1.4);
    ui.buttonDownload->setMinimumWidth(button_width * 1.4);
    ui.buttonSize->setMinimumWidth(button_width);
    ui.buttonTree->setMinimumWidth(button_width);
    ui.buttonLink->setMinimumWidth(button_width);
    ui.buttonExport->setMinimumWidth(button_width);
    ui.buttonCheck->setMinimumWidth(button_width);

    ui.buttonInfo->setIconSize(QSize(icon_w, icon_h));
    ui.buttonInfo->setMinimumWidth(button_width);
    ui.buttonTools->setIconSize(QSize(icon_w, icon_h));
    ui.buttonTools->setMinimumWidth(button_width);
    ui.buttonDedupe->setIconSize(QSize(icon_w, icon_h));
    ui.buttonDedupe->setMinimumWidth(button_width);
    ui.buttonCopy->setIconSize(QSize(icon_w, icon_h));
    ui.buttonCopy->setMinimumWidth(button_width);

  } else {
    if (buttonStyle == "textonly") {

      ui.buttonRefresh->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonMkdir->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonRename->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonMove->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonPurge->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonMount->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStream->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonUpload->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonDownload->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonSize->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonTree->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonLink->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonExport->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonCheck->setToolButtonStyle(Qt::ToolButtonTextOnly);

      ui.buttonRefresh->setMinimumWidth(button_width);
      ui.buttonMkdir->setMinimumWidth(button_width);
      ui.buttonRename->setMinimumWidth(button_width);
      ui.buttonMove->setMinimumWidth(button_width);
      ui.buttonPurge->setMinimumWidth(button_width);
      ui.buttonMount->setMinimumWidth(button_width);
      ui.buttonStream->setMinimumWidth(button_width);
      ui.buttonUpload->setMinimumWidth(button_width * 1.4);
      ui.buttonDownload->setMinimumWidth(button_width * 1.4);
      ui.buttonSize->setMinimumWidth(button_width);
      ui.buttonTree->setMinimumWidth(button_width);
      ui.buttonLink->setMinimumWidth(button_width);
      ui.buttonExport->setMinimumWidth(button_width);
      ui.buttonCheck->setMinimumWidth(button_width);

      ui.buttonInfo->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonInfo->setMinimumWidth(button_width);
      ui.buttonTools->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonTools->setMinimumWidth(button_width);
      ui.buttonDedupe->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonDedupe->setMinimumWidth(button_width);
      ui.buttonCopy->setMinimumWidth(button_width);
      ui.buttonCopy->setToolButtonStyle(Qt::ToolButtonTextOnly);

    } else {
      // button style - icononly
      ui.buttonRefresh->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonRefresh->setIconSize(QSize(icon_w, icon_h));
      ui.buttonMkdir->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonMkdir->setIconSize(QSize(icon_w, icon_h));
      ui.buttonRename->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonRename->setIconSize(QSize(icon_w, icon_h));
      ui.buttonMove->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonMove->setIconSize(QSize(icon_w, icon_h));
      ui.buttonPurge->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonPurge->setIconSize(QSize(icon_w, icon_h));
      ui.buttonMount->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonMount->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStream->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStream->setIconSize(QSize(icon_w, icon_h));
      ui.buttonUpload->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonUpload->setIconSize(QSize(icon_w, icon_h));
      ui.buttonDownload->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonDownload->setIconSize(QSize(icon_w, icon_h));
      ui.buttonSize->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonSize->setIconSize(QSize(icon_w, icon_h));
      ui.buttonTree->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonTree->setIconSize(QSize(icon_w, icon_h));
      ui.buttonLink->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonLink->setIconSize(QSize(icon_w, icon_h));
      ui.buttonExport->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonExport->setIconSize(QSize(icon_w, icon_h));
      ui.buttonCheck->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonCheck->setIconSize(QSize(icon_w, icon_h));
      ui.buttonInfo->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonInfo->setIconSize(QSize(icon_w, icon_h));
      ui.buttonTools->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonTools->setIconSize(QSize(icon_w, icon_h));
      ui.buttonDedupe->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonDedupe->setIconSize(QSize(icon_w, icon_h));
      ui.buttonCopy->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonCopy->setIconSize(QSize(icon_w, icon_h));
    }
  }

  ui.refresh->setStatusTip("Refresh (F5)");
  ui.mkdir->setStatusTip("New Folder (F7) - rclone mkdir");
  ui.rename->setStatusTip("Rename (F2) - rclone moveto");
  ui.copy->setStatusTip("Server side copy - rclone copy");
  ui.move->setStatusTip("Server side move - rclone move");
  ui.purge->setStatusTip("Delete (Del) - rclone purge|delete");
  ui.actionNewMount->setStatusTip(
      "Mount remote to local filesystem - rclone mount");
  ui.stream->setStatusTip("Stream file - rclone cat | player -");
  ui.upload->setStatusTip("Upload files/directories (ALT-u)");
  ui.download->setStatusTip("Download files/directories (ALT-d)");
  ui.getSize->setStatusTip("Get items size - rclone size");
  ui.getTree->setStatusTip("Show directory tree - rclone tree");
  ui.link->setStatusTip("Fetch public link - rclone link");
  ui.export_->setStatusTip("Export files' list");
  ui.actionCheck->setStatusTip(
      "Check remote's integrity - rclone check/cryptcheck");
  ui.actionDedupe->setStatusTip("Remove duplicated files - rclone dedupe");
  ui.getInfo->setStatusTip("Get remote info - rclone about");
  ui.actionTools->setStatusTip("Show additional tools");
  ui.cleanup->setStatusTip("Clean up the remote if possible. Empty the trash "
                           "or delete old file versions - rclone cleanup");

  QMenu *menuMode = new QMenu(this);
  menuMode->addAction(ui.getTree);
  menuMode->addAction(ui.link);
  menuMode->addAction(ui.export_);
  menuMode->addAction(ui.actionCheck);

  if (remoteType == "drive") {
    menuMode->addAction(ui.actionDedupe);
  }
  //  if (remoteType == "drive" || remoteType == "b2" || remoteType =="mailru"
  //  || remoteType =="mega" || remoteType =="pcloud" || remoteType =="yandex" )
  //  {
  menuMode->addAction(ui.cleanup);
  //  }
  ui.buttonTools->setMenu(menuMode);
  ui.buttonTools->setPopupMode(QToolButton::InstantPopup);

  // set combo box tooltips
  ui.cb_GoogleDriveMode->setItemData(0, "default", Qt::ToolTipRole);
  ui.cb_GoogleDriveMode->setItemData(1, "--drive-shared-with-me",
                                     Qt::ToolTipRole);
  ui.cb_GoogleDriveMode->setItemData(2, "--drive-trashed-only",
                                     Qt::ToolTipRole);

  ui.tree->sortByColumn(0, Qt::AscendingOrder);
  ui.tree->header()->setSectionsMovable(false);

  model = new ItemModel(iconCache, remote, this);
  ui.tree->setModel(model);
  QTimer::singleShot(0, ui.tree, SLOT(setFocus()));

  connect(ui.tree->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          SLOT(processSelection(QItemSelection, QItemSelection)));

  QObject::connect(model, &QAbstractItemModel::layoutChanged, this, [=]() {
    ui.tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.tree->resizeColumnToContents(1);
    ui.tree->resizeColumnToContents(2);
  });

  QObject::connect(ui.tree, &QAbstractItemView::clicked, this, [=]() {
    // not used now

    //    QModelIndex index;
    //    QModelIndexList selection = ui.tree->selectionModel()->selectedRows();
    //    index = selection.at(0);

    //    qDebug() << "index: " << index;
  });

  QObject::connect(
      ui.tree, &QTreeView::expanded, this, [=](const QModelIndex &index) {
        if (!mPreemptiveLoading) {
          return;
        }

        // preemptive loading
        QMutexLocker locker(&preemptiveLoadingProcessorMutex);

        if (!mPreemptiveLoadingListDoneNodes.contains(index)) {
          mPreemptiveLoadingListDoneNodes.append(index);
        }

        if (!mPreemptiveLoadingListDone.contains(index)) {

          if (model->isLoading(model->index(0, 0, index))) {
            mPreemptiveLoadingListPending.append(index);
            mPreemptiveLoadingListDone.append(index);
          } else {
            mPreemptiveLoadingListDone.append(index);
            // preload children
            for (int i = 0; i < model->rowCount(index); ++i) {
              if (model->isFolder(model->index(i, 0, index))) {
                mPreemptiveLoadingList.append(model->index(i, 0, index));
                mPreemptiveLoadingListDups = true;
              }
            }
          }
        }

        /*
              QModelIndex parentIndex = index.parent();
                if (!mPreemptiveLoadingListDone.contains(parentIndex)) {
                  // preload peers
                  for (int i = 0; i < model->rowCount(parentIndex); ++i) {
                    if (model->isFolder(model->index(i, 0, parentIndex))) {
                      mPreemptiveLoadingList.append(model->index(i, 0,
           parentIndex)); mPreemptiveLoadingListDups = true;
                    }
                  }
                  mPreemptiveLoadingListDone.append(parentIndex);
                }
        */

        QTimer::singleShot(0, this, SLOT(preemptiveLoadingProcessor()));
      });

  QObject::connect(
      ui.tree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
      [=]() {
        QModelIndex index;
        QModelIndexList selection = ui.tree->selectionModel()->selectedRows();

        int multiSelectCount = selection.count();

        if (selection.isEmpty()) {
          for (auto child : findChildren<QAction *>()) {
            child->setDisabled(true);
          }
          ui.getInfo->setDisabled(false);
          ui.cleanup->setDisabled(false);
          ui.path->clear();
          return;
        }

        if (multiSelectCount > 1) {
          for (auto child : findChildren<QAction *>()) {
            child->setDisabled(true);
          }
          ui.refresh->setDisabled(false);
          ui.purge->setDisabled(false);
          ui.copy->setDisabled(false);
          ui.move->setDisabled(false);
          ui.download->setDisabled(false);
          ui.getSize->setDisabled(false);
          ui.getInfo->setDisabled(false);
          ui.path->clear();
          return;
        }

        // there is only once item selected
        index = selection.at(0);

        bool topLevel = model->isTopLevel(index);
        bool isFolder = model->isFolder(index);
        bool driveModeButtons = false;
        bool isNotMountable = false;
        QDir path;

        if (remoteType == "drive") {
          // for Google Drive --drive-shared-with-me and --drive-trashed-only is
          // read only
          driveModeButtons = (ui.cb_GoogleDriveMode->currentIndex() == 1 ||
                              ui.cb_GoogleDriveMode->currentIndex() == 2);
        }

#if defined(Q_OS_WIN32)
        // check if required rclone version
        if (rcloneVersionResult == 2) {
          // rclone version before 1.50 - no mount in Windows
          isNotMountable = true;
        } else {
        };
#endif

// mount is not supported by rclone on these systems
#if defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
        isNotMountable = true;
#endif

        // local file system is not mountable
        if (remoteType == "local") {
          isNotMountable = true;
        }

        if (model->isLoading(index)) {
          ui.refresh->setDisabled(true);

          ui.mkdir->setDisabled(true);
          ui.copy->setDisabled(true);
          ui.move->setDisabled(true);
          ui.rename->setDisabled(true);
          ui.purge->setDisabled(true);

          ui.actionNewMount->setDisabled(true);
          ui.stream->setDisabled(true);
          ui.upload->setDisabled(true);
          ui.download->setDisabled(true);

          ui.getSize->setDisabled(true);

          ui.getTree->setDisabled(true);
          ui.link->setDisabled(true);
          ui.export_->setDisabled(true);
          ui.actionCheck->setDisabled(true);
          ui.actionDedupe->setDisabled(true);

          ui.getInfo->setDisabled(false);
          ui.cleanup->setDisabled(false);

          ui.cb_GoogleDriveMode->setDisabled(true);
          path = model->path(model->parent(index));

        } else {

          ui.refresh->setDisabled(false);

          ui.mkdir->setDisabled(driveModeButtons);
          ui.rename->setDisabled(topLevel || driveModeButtons);
          ui.copy->setDisabled(topLevel || driveModeButtons);
          ui.move->setDisabled(topLevel || driveModeButtons);
          ui.purge->setDisabled(topLevel || driveModeButtons);

          ui.actionNewMount->setDisabled(!isFolder || isNotMountable);
          ui.stream->setDisabled(isFolder);
          ui.upload->setDisabled(!isFolder || driveModeButtons);
          ui.download->setDisabled(false);

          ui.getSize->setDisabled(false);
          ui.getTree->setDisabled(!isFolder);
          ui.link->setDisabled(topLevel);
          ui.export_->setDisabled(!isFolder);
          ui.actionCheck->setDisabled(!isFolder);
          ui.actionDedupe->setDisabled(!isFolder || driveModeButtons);

          ui.getInfo->setDisabled(false);
          ui.cleanup->setDisabled(false);

          ui.cb_GoogleDriveMode->setDisabled(!isGoogle);

          path = model->path(index);
        }

        ui.path->setText(
            remote + ":" +
            (isLocal ? QDir::toNativeSeparators(path.path()) : path.path()));
      });

  // QObject::connect(ui.refresh
  QObject::connect(ui.refresh, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndexList multiSelection = ui.tree->selectionModel()->selectedRows();
    int multiSelectCount = multiSelection.count();

    if (multiSelectCount < 2) {
      QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
      model->refresh(index);
    } else {

      bool parentRefreshed = false;
      for (int i = 0; i < multiSelectCount; ++i) {
        QModelIndex multiIndex = multiSelection.at(i);
        if (model->isFolder(multiIndex)) {
          model->refresh(multiIndex);
        } else {
          // refresh parent folder only once
          if (!parentRefreshed) {
            model->refresh(multiIndex);
            parentRefreshed = true;
          };
        }
      }
    }
  });

  // QObject::connect(ui.mkdir
  QObject::connect(ui.mkdir, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    if (!model->isFolder(index)) {
      index = index.parent();
    }
    QDir path = model->path(index);
    QString pathMsg =
        isLocal ? QDir::toNativeSeparators(path.path()) : path.path();

    if (pathMsg.isEmpty()) {
      pathMsg = metrix.elidedText(remote, Qt::ElideMiddle, 500) + ":";
    }

    QString name = QInputDialog::getText(
        this, "New Folder",
        QString("Create folder in %1")
                .arg("\"" + metrix.elidedText(pathMsg, Qt::ElideMiddle, 500)) +
            "\"");
    if (!name.isEmpty()) {
      QString folder = path.filePath(name);

      QString folderMsg = metrix.elidedText(
          (isLocal ? QDir::toNativeSeparators(folder) : folder),
          Qt::ElideMiddle, 500);

      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList()
                           << "mkdir" << GetRcloneConf()
                           << GetRemoteModeRcloneOptions()
                           << GetDefaultOptionsList("defaultRcloneOptions")
                           << remote + ":" + folder);
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress("New Folder", "Creating...",
                              "\"" + folderMsg + "\"", &process, this);
      if (progress.exec() == QDialog::Accepted) {
        model->refresh(index);
      }
    }
  });

  // QObject::connect(ui.rename
  QObject::connect(ui.rename, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

    QString name = model->data(index, Qt::DisplayRole).toString();
    name = QInputDialog::getText(
        this, "Rename",
        QString("New name for %1")
            .arg("\"" + metrix.elidedText(pathMsg, Qt::ElideMiddle, 500) +
                 "\""),
        QLineEdit::Normal, name);
    if (!name.isEmpty()) {
      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList()
                           << "moveto" << GetRcloneConf()
                           << GetRemoteModeRcloneOptions()
                           << GetDefaultOptionsList("defaultRcloneOptions")
                           << remote + ":" + path
                           << remote + ":" +
                                  model->path(index.parent()).filePath(name));
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress(
          "Rename", "Renaming...",
          "\"" + metrix.elidedText(pathMsg, Qt::ElideMiddle, 500) + "\"",
          &process, this);
      if (progress.exec() == QDialog::Accepted) {
        model->rename(index, name);
      }
    }
  });

  //!!! QObject::connect(ui.copy
  QObject::connect(ui.copy, &QAction::triggered, this, [=]() {
    QString rMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QString progressMsg;
    QModelIndexList multiSelection = ui.tree->selectionModel()->selectedRows();
    int multiSelectCount = multiSelection.count();
    bool isMultiSelect = (multiSelection.count() > 1);
    bool isMove = false; // copy operation

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    if (multiSelectCount == 0) {
      return;
    }

    QStringList filterList;
    QStringList filterListFinal;
    QStringList args;

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QModelIndex sourceIndex;
    QModelIndex sourceRefreshIndex;
    QModelIndex destIndex;
    QString path = model->path(index).path();

    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;
    QString destFolder = model->path(index.parent()).path() + "/";
    QString toolTip;

    if (isMultiSelect) {
      sourceIndex = index.parent();
      sourceRefreshIndex = sourceIndex;
      path = model->path(index.parent()).path();

      // get sselection patterns for --filter
      QStringList tmpList;
      tmpList = getSelectionFilteringPatterns(multiSelection);

      for (int i = 0; i < tmpList.count(); ++i) {
        QString tmp = tmpList.at(i);
        tmp = "+ " + tmp;
        filterList << tmp;
      }

      filterList << "- *";

    } else {
      sourceIndex = index;

      if (model->isTopLevel(sourceIndex)) {
        sourceRefreshIndex = sourceIndex;
      } else {
        sourceRefreshIndex = sourceIndex.parent();
      }
    }

    RemoteFolderDialog rfd(isMove, isMultiSelect, filterList, sourceIndex,
                           remote, mRemoteType, rMode, isLocal, mRootIndex,
                           model, this);

    if (rfd.exec() == QDialog::Accepted) {

      destIndex = rfd.pDestIndex;

      args << rfd.getOptions();

      filterListFinal << rfd.getFilterList();

      // as we run rclone directly here we have to add --filter
      for (int i = 0; i < filterListFinal.count(); ++i) {
        args << "--filter";
        args << filterListFinal.at(i);
      }

      if (isMultiSelect) {

        destFolder = model->path(rfd.pDestIndex).path();
        toolTip = QString("%1 items from \"%2\" to \"%3:%4\"")
                      .arg(multiSelectCount)
                      .arg(path)
                      .arg(remote)
                      .arg(destFolder);

        pathMsg = QString("%1 items from \"%2\" to \"%3:%4\"")
                      .arg(multiSelectCount)
                      .arg(metrix.elidedText(path, Qt::ElideMiddle, 250))
                      .arg(metrix.elidedText(remote, Qt::ElideMiddle, 150))
                      .arg(metrix.elidedText(destFolder, Qt::ElideMiddle, 250));

      } else {

        if (model->isFolder(sourceIndex)) {
          // folder
          destFolder = (model->path(rfd.pDestIndex))
                           .filePath((model->path(sourceIndex)).dirName());
          sourceIndex = sourceIndex.parent();

        } else {
          // file
          destFolder = model->path(rfd.pDestIndex).path();
        }
        toolTip = QString("\"%1\" to \"%2:%3\"")
                      .arg(path)
                      .arg(remote)
                      .arg(destFolder);

        pathMsg = QString("\"%1\" to \"%2:%3\"")
                      .arg(metrix.elidedText(path, Qt::ElideMiddle, 250))
                      .arg(metrix.elidedText(remote, Qt::ElideMiddle, 150))
                      .arg(metrix.elidedText(destFolder, Qt::ElideMiddle, 250));
      }

    } else {

      return;
    }

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(QStringList()
                         << "copy" << GetRcloneConf()
                         << GetRemoteModeRcloneOptions()
                         << GetDefaultOptionsList("defaultRcloneOptions")
                         << remote + ":" + path << remote + ":" + destFolder
                         << args);

    process.setProcessChannelMode(QProcess::MergedChannels);

    ProgressDialog progress(
        "Copy", "Copying... ", pathMsg, &process, this,
        !(args.contains("--dry-run") || args.contains("--verbose")), false,
        toolTip);

    if (args.contains("--dry-run") || args.contains("--verbose")) {
      progress.expand();
    }
    progress.allowToClose();

    if (progress.exec() == QDialog::Accepted) {
    }

    // Refresh
    model->refresh(destIndex);
  });

  //!!! QObject::connect(ui.move
  QObject::connect(ui.move, &QAction::triggered, this, [=]() {
    QString rMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QString progressMsg;
    QModelIndexList multiSelection = ui.tree->selectionModel()->selectedRows();
    int multiSelectCount = multiSelection.count();
    bool isMultiSelect = (multiSelection.count() > 1);
    bool isMove = true;

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    if (multiSelectCount == 0) {
      return;
    }

    QStringList filterList;
    QStringList filterListFinal;
    QStringList args;

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QModelIndex sourceIndex;
    QModelIndex sourceRefreshIndex;
    QModelIndex destIndex;
    QString path = model->path(index).path();

    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;
    QString destFolder = model->path(index.parent()).path() + "/";
    QString toolTip;

    if (isMultiSelect) {
      sourceIndex = index.parent();
      sourceRefreshIndex = sourceIndex;
      path = model->path(index.parent()).path();

      // get sselection patterns for --filter
      QStringList tmpList;
      tmpList = getSelectionFilteringPatterns(multiSelection);

      for (int i = 0; i < tmpList.count(); ++i) {
        QString tmp = tmpList.at(i);
        tmp = "+ " + tmp;
        filterList << tmp;
      }

      filterList << "- *";

    } else {
      sourceIndex = index;

      if (model->isTopLevel(sourceIndex)) {
        sourceRefreshIndex = sourceIndex;
      } else {
        sourceRefreshIndex = sourceIndex.parent();
      }
    }

    RemoteFolderDialog rfd(isMove, isMultiSelect, filterList, sourceIndex,
                           remote, mRemoteType, rMode, isLocal, mRootIndex,
                           model, this);

    if (rfd.exec() == QDialog::Accepted) {

      destIndex = rfd.pDestIndex;

      args << rfd.getOptions();

      filterListFinal << rfd.getFilterList();

      // as we run rclone directly here we have to add --filter
      for (int i = 0; i < filterListFinal.count(); ++i) {
        args << "--filter";
        args << filterListFinal.at(i);
      }

      if (isMultiSelect) {

        destFolder = model->path(rfd.pDestIndex).path();
        toolTip = QString("%1 items from \"%2\" to \"%3:%4\"")
                      .arg(multiSelectCount)
                      .arg(path)
                      .arg(remote)
                      .arg(destFolder);

        pathMsg = QString("%1 items from \"%2\" to \"%3:%4\"")
                      .arg(multiSelectCount)
                      .arg(metrix.elidedText(path, Qt::ElideMiddle, 250))
                      .arg(metrix.elidedText(remote, Qt::ElideMiddle, 150))
                      .arg(metrix.elidedText(destFolder, Qt::ElideMiddle, 250));

      } else {

        if (model->isFolder(sourceIndex)) {
          // folder
          destFolder = (model->path(rfd.pDestIndex))
                           .filePath((model->path(sourceIndex)).dirName());
          sourceIndex = sourceIndex.parent();

        } else {
          // file
          destFolder = model->path(rfd.pDestIndex).path();
        }
        toolTip = QString("\"%1\" to \"%2:%3\"")
                      .arg(path)
                      .arg(remote)
                      .arg(destFolder);

        pathMsg = QString("\"%1\" to \"%2:%3\"")
                      .arg(metrix.elidedText(path, Qt::ElideMiddle, 250))
                      .arg(metrix.elidedText(remote, Qt::ElideMiddle, 150))
                      .arg(metrix.elidedText(destFolder, Qt::ElideMiddle, 250));
      }

    } else {

      return;
    }

    QProcess process;
    UseRclonePassword(&process);
    process.setProgram(GetRclone());
    process.setArguments(QStringList()
                         << "move" << GetRcloneConf()
                         << GetRemoteModeRcloneOptions()
                         << GetDefaultOptionsList("defaultRcloneOptions")
                         << remote + ":" + path << remote + ":" + destFolder
                         << args);

    process.setProcessChannelMode(QProcess::MergedChannels);

    ProgressDialog progress(
        "Move", "Moving... ", pathMsg, &process, this,
        !(args.contains("--dry-run") || args.contains("--verbose")), false,
        toolTip);

    if (args.contains("--dry-run") || args.contains("--verbose")) {
      progress.expand();
    }
    progress.allowToClose();

    if (progress.exec() == QDialog::Accepted) {
    }

    // Refresh
    model->refresh(sourceRefreshIndex);
    mSrcIndex = sourceRefreshIndex;
    mDestIndex = destIndex;

    // refresh folders one by one
    QTimer::singleShot(500, Qt::CoarseTimer, this, SLOT(refreshAfterMove()));

    clearPreemptiveQueues();
  });

  //!!! QObject::connect(ui.purge
  QObject::connect(ui.purge, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndexList selection = ui.tree->selectionModel()->selectedRows();
    QModelIndex index;

    int button = QMessageBox::question(
        this, "Delete",
        QString("Are you sure you want to delete %1 selected item(s)?")
            .arg(selection.count()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (button == QMessageBox::Yes) {

      // list of rclone args for delete operations
      QList<QStringList> pDataList;

      for (int i = 0; i < selection.count(); i++) {

        index = selection.at(i);

        QString path = model->path(index).path();
        QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;

        QStringList args;
        args << (model->isFolder(index) ? "purge" : "delete") << GetRcloneConf()
             << GetRemoteModeRcloneOptions()
             << GetDefaultOptionsList("defaultRcloneOptions")
             << remote + ":" + path;

        pDataList.append(args);
      }

      DeleteProgressDialog deleteProgress(pDataList, this, true);
      deleteProgress.allowToClose();
      if (deleteProgress.exec() == QDialog::Accepted) {
        model->refresh(index.parent());
      } else {
        model->refresh(index.parent());
      }
      clearPreemptiveQueues();
    } // yes button
  });

  // QObject::connect(ui.stream,
  QObject::connect(ui.stream, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QString path = model->path(index).path();

    auto settings = GetSettings();
    bool streamConfirmed =
        settings->value("Settings/streamConfirmed", false).toBool();
    QString stream = settings->value("Settings/stream", "vlc -").toString();
    if (!streamConfirmed) {
      QString result = QInputDialog::getText(
          this, "Stream",
          "Enter stream command (file will be passed in STDIN):",
          QLineEdit::Normal, stream);
      if (result.isEmpty()) {
        return;
      }

      stream = result;

      settings->setValue("Settings/stream", stream);
      settings->setValue("Settings/streamConfirmed", true);
    }

    QFileInfo fi(path);
    QString filename = fi.fileName();
    stream.replace("$file_name", filename);

    emit addStream(remote + ":" + path, stream, remoteType);
  });

  // QObject::connect(ui.link
  QObject::connect(ui.link, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    QString toolTip;

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();

    QString pathMsg =
        metrix.elidedText((isLocal ? QDir::toNativeSeparators(path) : path),
                          Qt::ElideMiddle, 500);

    toolTip = "\"" + remote + ":" +
              (isLocal ? QDir::toNativeSeparators(path) : path) + "\"";

    QProcess *process = new QProcess;
    UseRclonePassword(process);
    process->setProgram(GetRclone());
    process->setArguments(QStringList()
                          << "link" << GetRcloneConf()
                          << GetRemoteModeRcloneOptions()
                          << GetDefaultOptionsList("defaultRcloneOptions")
                          << remote + ":" + path);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog *progress =
        new ProgressDialog("Fetch Public Link", "Running... ",
                           QString("Public link for: ") + "\"" +
                               metrix.elidedText(remote, Qt::ElideMiddle, 150) +
                               ":" + pathMsg + "\"",
                           process, NULL, false, true, toolTip);
    progress->expand();
    progress->allowToClose();
    progress->show();
  });

  //!!! QObject::connect(ui.upload
  QObject::connect(ui.upload, &QAction::triggered, this, [=]() {
    QString _remoteMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    if (!model->isFolder(index)) {
      index = index.parent();
    }
    QDir path = model->path(index);

    QStringList empty;
    TransferDialog t(false, false, remote, path, true, remoteType, _remoteMode,
                     false, empty, this);
    if (t.exec() == QDialog::Accepted) {

      if (t.getDryRun() || t.getTaskId() == "") {

        QString src = t.getSource();
        QString dst = t.getDest();
        QStringList args = t.getOptions();
        QString info;

        if (t.getDryRun()) {
          args << "--dry-run";
          info = QString("Dry run, %1 from %2").arg(t.getMode()).arg(src);
        } else {
          info = QString("%1 from %2").arg(t.getMode()).arg(src);
        }
        emit addTransfer(info, src, dst, args, QUuid::createUuid().toString(),
                         "", QUuid::createUuid().toString());

      } else {
        emit addSavedTransfer(t.getTaskId(), t.getDryRun(), t.getAddToQueue());
      }
    }
  });

  //!!! QObject::connect(ui.download
  QObject::connect(ui.download, &QAction::triggered, this, [=]() {
    QString _remoteMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QDir path = model->path(index);

    QModelIndexList multiSelection = ui.tree->selectionModel()->selectedRows();
    int multiSelectCount = multiSelection.count();
    QStringList includedList;
    bool isMultiselect = false;

    if (multiSelectCount > 1) {

      // selection control makes sure that all are on the same level so we can
      // use any to derived download root

      isMultiselect = true;
      path = model->path(index.parent());

      includedList = getSelectionFilteringPatterns(multiSelection);
    }

    TransferDialog t(true, false, remote, path, model->isFolder(index),
                     remoteType, _remoteMode, isMultiselect, includedList,
                     this);
    if (t.exec() == QDialog::Accepted) {

      if (t.getDryRun() || t.getTaskId() == "") {

        QString src = t.getSource();
        QString dst = t.getDest();
        QStringList args = t.getOptions();
        QString info;

        if (t.getDryRun()) {
          args << "--dry-run";
          info = QString("Dry run, %1 from %2").arg(t.getMode()).arg(src);
        } else {
          info = QString("%1 from %2").arg(t.getMode()).arg(src);
        }

        emit addTransfer(info, src, dst, args, QUuid::createUuid().toString(),
                         "", QUuid::createUuid().toString());

      } else {
        emit addSavedTransfer(t.getTaskId(), t.getDryRun(), t.getAddToQueue());
      }
    }
  });

  //!!! QObject::connect(ui.getTree
  QObject::connect(ui.getTree, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path = model->path(index).path();
    QString pathMsg =
        metrix.elidedText((isLocal ? QDir::toNativeSeparators(path) : path),
                          Qt::ElideMiddle, 500);

    QString toolTip = "\"" + remote + ":" +
                      (isLocal ? QDir::toNativeSeparators(path) : path) + "\"";

    QProcess *process = new QProcess;
    UseRclonePassword(process);
    process->setProgram(GetRclone());
    process->setArguments(
        QStringList() << "tree"
                      << "-d" << GetRcloneConf() << GetRemoteModeRcloneOptions()
                      << GetDefaultOptionsList("defaultRcloneOptions")
                      << remote + ":" + path);
    process->setProcessChannelMode(QProcess::MergedChannels);
    ProgressDialog *progress =
        new ProgressDialog("Show directories tree", "Running... ",
                           QString("rclone tree -d ") + "\"" +
                               metrix.elidedText(remote, Qt::ElideMiddle, 150) +
                               ":" + pathMsg + "\"",
                           process, NULL, false, false, toolTip);
    progress->expand();
    progress->allowToClose();
    //    progress->resize(566, 350);
    progress->show();
  });

  //!!! QObject::connect(ui.getSize
  QObject::connect(ui.getSize, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QString progressMsg;
    QModelIndexList multiSelection = ui.tree->selectionModel()->selectedRows();
    int multiSelectCount = multiSelection.count();

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    if (multiSelectCount == 0)
      return;

    QString toolTip;
    QStringList includedList;
    QStringList includedListFinal;

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();
    QString path = model->path(index).path();

    QString pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;
    QProcess *process = new QProcess;

    if (multiSelectCount > 1) {

      path = model->path(index.parent()).path();
      pathMsg = isLocal ? QDir::toNativeSeparators(path) : path;
      progressMsg =
          QString("Size of selected %1 items in ").arg(multiSelectCount) +
          "\"" + metrix.elidedText(remote, Qt::ElideMiddle, 150) + ":" +
          metrix.elidedText(pathMsg, Qt::ElideMiddle, 500) + "\"";

      includedList = getSelectionFilteringPatterns(multiSelection);

      // as we run rclone directly here we have to add --filter
      for (int i = 0; i < includedList.count(); ++i) {
        includedListFinal << "--filter";
        includedListFinal << "+ " + includedList.at(i);
      }
      includedListFinal << "--filter"
                        << "- *";

    } else {

      toolTip = "\"" + remote + ":" + pathMsg + "\"";

      progressMsg = QString("Size of ") + "\"" +
                    metrix.elidedText(remote, Qt::ElideMiddle, 150) + ":" +
                    metrix.elidedText(pathMsg, Qt::ElideMiddle, 500) + "\"";

    } // if (multiSelectCount > 1)

    UseRclonePassword(process);
    process->setProgram(GetRclone());
    process->setArguments(QStringList()
                          << "size" << GetRcloneConf()
                          << GetRemoteModeRcloneOptions()
                          << GetDefaultOptionsList("defaultRcloneOptions")
                          << remote + ":" + path << includedListFinal);
    process->setProcessChannelMode(QProcess::MergedChannels);

    ProgressDialog *progress =
        new ProgressDialog("Get Size", "Running... ", progressMsg, process,
                           NULL, false, false, toolTip);

    progress->expand();
    progress->allowToClose();
    progress->show();
  });

  //!!! Object::connect(ui.export
  QObject::connect(ui.export_, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text base measure
    // progress dialog uses the same fonts
    QFontMetrics metrix(ui.elidedMeasure->font());

    QString toolTip;

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path_info = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path_info) : path_info;

    QDir path = model->path(index);

    ExportDialog e(remote, path, this);

    if (e.exec() == QDialog::Accepted) {
      QString dst = e.getDestination();
      bool txt = e.onlyFilenames();

      QFile *file = new QFile(dst);
      if (!file->open(QFile::WriteOnly)) {
        QMessageBox::warning(
            this, "Error",
            QString("Cannot open file '%1' for writing!").arg(dst));
        delete file;
        return;
      }

      QRegExp re(R"(^\s*(\d+) (\d\d\d\d-\d\d-\d\d \d\d:\d\d:\d\d)\.\d+ (.+)$)");

      QProcess *process = new QProcess;
      UseRclonePassword(process);
      process->setProgram(GetRclone());
      process->setArguments(QStringList()
                            << GetRcloneConf() << GetRemoteModeRcloneOptions()
                            << GetDefaultOptionsList("defaultRcloneOptions")
                            << e.getOptions());
      process->setProcessChannelMode(QProcess::MergedChannels);

      toolTip =
          "\"" + remote + ":" + pathMsg + "\"" + "\nto " + "\"" + dst + "\"";

      ProgressDialog *progress = new ProgressDialog(
          "Export", "Running... ",
          QString("Exporting content of ") + "\"" +
              metrix.elidedText(remote, Qt::ElideMiddle, 150) + ":" +
              metrix.elidedText(pathMsg, Qt::ElideMiddle, 500) + "\"" +
              "\nto " + "\"" + metrix.elidedText(dst, Qt::ElideMiddle, 500) +
              "\"",
          process, NULL, false, false, toolTip);

      file->setParent(progress);

      QObject::connect(progress, &ProgressDialog::outputAvailable, this,
                       [=](const QString &output) {
                         QTextStream out(file);
                         out.setCodec("UTF-8");

                         for (const auto &line : output.split('\n')) {

                           QString lineTmp = line;
                           lineTmp.replace("\n", "");

                           if (re.exactMatch(lineTmp)) {
                             QStringList cap = re.capturedTexts();

                             if (txt) {
                               out << "\"" << cap[3] << "\"" << '\n';
                             } else {
                               QString name = cap[3];
                               out << "\"" << name << "\""
                                   << ","
                                   << "\"" << cap[2] << "\""
                                   << "," << cap[1].toULongLong() << '\n';
                             }
                           }
                         }
                       });

      progress->allowToClose();
      progress->show();
    }
  });

  QObject::connect(ui.actionTools, &QAction::triggered, this, [=]() {
    if (mButtonToolsState) {
      mButtonToolsState = false;
      ui.frameTools->hide();
      ui.buttonTools->setDown(false);

    } else {
      mButtonToolsState = true;
      ui.frameTools->show();
      ui.buttonTools->setDown(true);

      QPropertyAnimation *animation =
          new QPropertyAnimation(ui.frameTools, "maximumHeight");
      animation->setDuration(100);
      animation->setStartValue(0);
      animation->setEndValue(150);
      animation->start(QPropertyAnimation::DeleteWhenStopped);
    }
  });

  QObject::connect(ui.actionNewMount, &QAction::triggered, this, [=]() {
    QString remoteMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path_info = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path_info) : path_info;

    QString path = model->path(index).path();

    QDir path_mount = model->path(index);

#if defined(Q_OS_WIN)
    // on Windows we check if WinFsp is installed
    QSettings winKey("HKEY_CLASSES_ROOT\\Installer\\Dependencies",
                     QSettings::NativeFormat);

    if (winKey.childGroups().contains("WinFsp", Qt::CaseInsensitive)) {
#endif
#if defined(Q_OS_MACOS)
      // on macOS we check if FUSE for macOS is installed

      const QFileInfo outputDir("/Library/Filesystems/macfuse.fs/");

      if (outputDir.exists()) {

#endif

        MountDialog e(remote, path_mount, remoteType, remoteMode, this);

        if (e.exec() == QDialog::Accepted) {

          QStringList args = e.getOptions();

          emit addNewMount(remote + ":" + path, e.getMountPoint(), remoteType,
                           args, e.getScript(), QUuid::createUuid().toString(),
                           "");
        }

#if defined(Q_OS_WIN)
      } else {

        QMessageBox::information(
            this, "FUSE for Windows (WinFsp) warning",
            QString(
                R"(<p>To run "rclone mount" on Windows,<br />you will need to )"
                R"(download and install:<br /><br />)"
                R"(<a href="http://www.secfs.net/winfsp/">FUSE for Windows (WinFsp)</a><br /></p>)"));
      }
#endif

#if defined(Q_OS_MACOS)
    } else {

      QMessageBox::information(
          this, "FUSE for macOS warning",
          QString(
              R"(<p>To run "rclone mount" on macOS,<br />you will need to )"
              R"(download and install:<br /><br />)"
              R"(<a href="https://osxfuse.github.io">FUSE for macOS</a></p>)"));
    }
#endif
  });

  QObject::connect(ui.actionCheck, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path_info = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path_info) : path_info;

    QDir path = model->path(index);
    CheckDialog e(remote, path, remoteType, this);

    if (e.exec() == QDialog::Accepted) {
      QString source = e.getSource();

      QProcess *process = new QProcess;
      UseRclonePassword(process);
      process->setProgram(GetRclone());

      process->setArguments(QStringList()
                            << GetRcloneConf() << e.getOptions()
                            << GetRemoteModeRcloneOptions()
                            << GetDefaultOptionsList("defaultRcloneOptions"));
      process->setProcessChannelMode(QProcess::MergedChannels);

      QString checkcommand = "Integity check";
      if (!e.isCheck()) {
        checkcommand = "Integrity cryptcheck";
      }

      ProgressDialog *progress = new ProgressDialog(
          checkcommand, "Running... ",
          "rclone " + e.getOptions().join(" ") + " " +
              GetRemoteModeRcloneOptions().join(" ") + " " +
              GetDefaultOptionsList("defaultRcloneOptions").join(" "),
          process, NULL, false);

      progress->expand();
      progress->allowToClose();
      progress->show();
    }
  });

  QObject::connect(ui.actionDedupe, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    QModelIndex index = ui.tree->selectionModel()->selectedRows().front();

    QString path_info = model->path(index).path();
    QString pathMsg = isLocal ? QDir::toNativeSeparators(path_info) : path_info;

    QDir path = model->path(index);
    DedupeDialog e(remote, path, remoteType, this);

    if (e.exec() == QDialog::Accepted) {

      QProcess *process = new QProcess;
      UseRclonePassword(process);
      process->setProgram(GetRclone());

      process->setArguments(QStringList()
                            << GetRcloneConf() << e.getOptions()
                            << GetRemoteModeRcloneOptions()
                            << GetDefaultOptionsList("defaultRcloneOptions"));

      process->setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog *progress = new ProgressDialog(
          "rclone dedupe", "Running... ",
          "rclone " + e.getOptions().join(" ") + " " +
              GetRemoteModeRcloneOptions().join(" ") + " " +
              GetDefaultOptionsList("defaultRcloneOptions").join(" "),
          process, NULL, false);

      progress->expand();
      progress->allowToClose();
      progress->show();
    }
  });

  QObject::connect(ui.getInfo, &QAction::triggered, this, [=]() {
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text
    QFontMetrics metrix(ui.elidedMeasure->font());

    QString toolTip = "\"" + remote + ":" + "\"";

    QProcess *process = new QProcess;
    UseRclonePassword(process);
    process->setProgram(GetRclone());
    process->setArguments(QStringList()
                          << "about" << GetRcloneConf()
                          << GetRemoteModeRcloneOptions()
                          << GetDefaultOptionsList("defaultRcloneOptions")
                          << remote + ":");
    process->setProcessChannelMode(QProcess::MergedChannels);

    ProgressDialog *progress = new ProgressDialog(
        "Get remote Info", "Runnning... ",
        "rclone about \"" + metrix.elidedText(remote, Qt::ElideMiddle, 150) +
            ":\"",
        process, NULL, false, false, toolTip);

    progress->expand();
    progress->allowToClose();
    progress->show();
  });

  QObject::connect(ui.cleanup, &QAction::triggered, this, [=]() {
    QString rMode =
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

    // Elided....Text
    QFontMetrics metrix(ui.elidedMeasure->font());

    QString toolTip = "\"" + remote + ":" + "\"";

    int button = QMessageBox::question(
        this, "Cleanup",
        QString("Are you sure you want to cleanup remote: \n\n %1 \n\nThis "
                "action is irreversible.")
            .arg("\"" + metrix.elidedText(remote, Qt::ElideMiddle, 250) +
                 ":\""),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (button == QMessageBox::Yes) {

      QProcess process;
      UseRclonePassword(&process);
      process.setProgram(GetRclone());
      process.setArguments(QStringList()
                           << "cleanup" << GetRcloneConf()
                           << GetRemoteModeRcloneOptions()
                           << GetDefaultOptionsList("defaultRcloneOptions")
                           << remote + ":"
                           << "-vv");
      process.setProcessChannelMode(QProcess::MergedChannels);

      ProgressDialog progress(
          "Cleanup", "Runnning... ",
          "rclone cleanup \"" +
              metrix.elidedText(remote, Qt::ElideMiddle, 150) + ":\"",
          &process, NULL, false, false, toolTip);

      progress.expand();
      progress.allowToClose();
      progress.exec();

      // if view in Google trash we have to refresh it completely
      if (remoteType == "drive" && rMode == "trash") {

        clearPreemptiveQueues();

        // clear top folder's rows
        while (model->removeRow(0, mRootIndex)) {
        }

        ui.tree->selectionModel()->clear();
        ui.tree->selectionModel()->select(mRootIndex,
                                          QItemSelectionModel::Select |
                                              QItemSelectionModel::Rows);
        model->refresh(mRootIndex);
        QTimer::singleShot(0, ui.tree, SLOT(setFocus()));

        ui.path->setAlignment(Qt::AlignLeft);
        ui.path->clear();

        mPreemptiveLoadingListDone.append(mRootIndex);
        QTimer::singleShot(200, Qt::CoarseTimer, this,
                           SLOT(initialModelLoading()));
      }
    }
  });

  QObject::connect(
      model, &ItemModel::drop, this,
      [=](const QDir &path, const QModelIndex &parent) {
        setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), remoteType);

        qApp->setActiveWindow(this);
        QDir destPath = model->path(parent);
        QString dest = QFileInfo(path.path()).isDir()
                           ? destPath.filePath(path.dirName())
                           : destPath.path();

        QStringList empty;
        TransferDialog t(false, true, remote, dest, true, remoteType,
                         remoteMode, false, empty, this);
        t.setSource(path.path());

        if (t.exec() == QDialog::Accepted) {
          QString src = t.getSource();
          QString dst = t.getDest();

          QStringList args = t.getOptions();
          emit addTransfer(QString("%1 from %2").arg(t.getMode()).arg(src), src,
                           dst, args, QUuid::createUuid().toString(), "",
                           QUuid::createUuid().toString());
        }
      });

  QObject::connect(ui.tree, &QWidget::customContextMenuRequested, this,
                   [=](const QPoint &pos) {
                     setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(),
                                   remoteType);

                     QMenu menu;
                     menu.addAction(ui.refresh);
                     menu.addSeparator();
                     menu.addAction(ui.mkdir);
                     menu.addAction(ui.rename);
                     menu.addAction(ui.copy);
                     menu.addAction(ui.move);
                     menu.addAction(ui.purge);
                     menu.addSeparator();
                     menu.addAction(ui.actionNewMount);
                     menu.addAction(ui.stream);
                     menu.addAction(ui.upload);
                     menu.addAction(ui.download);
                     menu.addSeparator();
                     menu.addAction(ui.getSize);
                     menu.addAction(ui.getTree);
                     menu.addAction(ui.link);
                     menu.addAction(ui.export_);
                     menu.addAction(ui.actionCheck);
                     if (remoteType == "drive") {
                       menu.addAction(ui.actionDedupe);
                     }
                     menu.exec(ui.tree->viewport()->mapToGlobal(pos));
                   });

  if (isLocal) {
    QHash<QString, QPersistentModelIndex> drives;

    // QDir::drives is fast
    for (const auto &drive : QDir::drives()) {
      QString path = drive.path();
      QModelIndex index = model->addRoot(QDir::toNativeSeparators(path), path);
      drives.insert(path, index);
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)) && !(defined Q_OS_WIN)
    QThread *thread = new QThread(this);
    thread->start();

    QObject *worker = new QObject();
    worker->moveToThread(thread);

    QTimer::singleShot(0, worker, [=]() {
      QStorageInfo info;
      info.refresh();

      // QStorageInfo::mountedVolumes is slow :(
      for (const auto &volume : info.mountedVolumes()) {
        QString name = volume.name();
        if (!name.isEmpty()) {
          QString path = volume.rootPath();
          QString item =
              QString("%1 (%2)").arg(QDir::toNativeSeparators(path)).arg(name);
          QTimer::singleShot(0, this,
                             [=]() { model->rename(drives[path], item); });
        }
      }

      thread->quit();
      thread->deleteLater();
      worker->deleteLater();
    });
#endif

    ui.tree->selectionModel()->selectionChanged(QItemSelection(),
                                                QItemSelection());
  } else {
    QModelIndex index = model->addRoot("/", root);
    ui.tree->selectionModel()->select(
        index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    mRootIndex = index;

    mPreemptiveLoadingListDone.append(index);

    ui.tree->expand(index);
    QTimer::singleShot(200, Qt::CoarseTimer, this, SLOT(initialModelLoading()));
  }

  QShortcut *close = new QShortcut(QKeySequence::Close, this);
  QObject::connect(close, &QShortcut::activated, this, [=]() {
    auto tabs = qobject_cast<QTabWidget *>(parent);
    tabs->removeTab(tabs->indexOf(this));
  });

  QObject::connect(ui.shared, &QAction::triggered, [=]() {
    ui.cb_GoogleDriveMode->setDisabled(true);

    ui.tree->hideColumn(0);
    ui.tree->hideColumn(1);
    ui.tree->hideColumn(2);

    QTimer::singleShot(0, this, SLOT(switchRemoteType()));
  });
}

RemoteWidget::~RemoteWidget() {}

QString setRemoteMode(int index, QString remoteType) {

  QString mode = "main";

  if (remoteType == "drive") {

    auto settings = GetSettings();
    switch (index) {

    case 0:
      settings->setValue("Settings/remoteMode", "main");
      mode = "main";
      break;
    case 1:
      settings->setValue("Settings/remoteMode", "shared");
      mode = "shared";
      break;
    case 2:
      settings->setValue("Settings/remoteMode", "trash");
      mode = "trash";
      break;
    }
  }

  return mode;
}

void RemoteWidget::initialModelLoading() {
  QMutexLocker locker(&preemptiveLoadingProcessorMutex);
  setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), mRemoteType);

  // model and mRootIndex in private
  QModelIndex index = mRootIndex;

  if (model->rowCount(index) == 1) {
    // try again later
    QTimer::singleShot(200, Qt::CoarseTimer, this, SLOT(initialModelLoading()));
  } else {

    for (int i = 0; i < model->rowCount(index); ++i) {
      if (model->isFolder(model->index(i, 0, index))) {
        mPreemptiveLoadingList.append(model->index(i, 0, index));
      }
    }

    QTimer::singleShot(0, this, SLOT(preemptiveLoadingProcessor()));
  }
  return;
}

void RemoteWidget::preemptiveLoadingProcessor() {

  QMutexLocker locker(&preemptiveLoadingProcessorMutex);

  // don't do preloading for local drives
  if (mRemoteType == "local") {
    clearPreemptiveQueues();
    return;
  }

  if (!mPreemptiveLoading) {
    return;
  }

  // update aggressivness of preloading every few secs
  mCountLevel++;
  if ((mCountLevel % 30) == 0) {

    mCountLevel = 1;

    auto settings = GetSettings();

    if (settings->value("Settings/preemptiveLoading").toBool()) {
      mPreemptiveLoading = true;
    } else {
      mPreemptiveLoading = false;
    }

    int preemptiveLoadingLevel =
        settings->value("Settings/preemptiveLoadingLevel").toInt();

    if (preemptiveLoadingLevel == 0) {
      mMaxRcloneLsProcessCount = 10;
    }
    if (preemptiveLoadingLevel == 1) {
      mMaxRcloneLsProcessCount = 20;
    }
    if (preemptiveLoadingLevel == 2) {
      mMaxRcloneLsProcessCount = 40;
    }
  }

  bool runAgain = false;

  setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), mRemoteType);

  QModelIndex tmpIndex;
  QModelIndexList tmpList;
  int tmpCount;

  tmpCount = mPreemptiveLoadingListPending.count();

  if (tmpCount > 0) {

    for (int i = tmpCount - 1; i >= 0; i--) {

      tmpIndex = mPreemptiveLoadingListPending.at(i);

      if (!model->isLoading(model->index(0, 0, tmpIndex))) {
        tmpIndex = mPreemptiveLoadingListPending.takeAt(i);
        // preload children
        for (int j = 0; j < model->rowCount(tmpIndex); ++j) {
          if (model->isFolder(model->index(i, 0, tmpIndex))) {
            mPreemptiveLoadingList.append(model->index(j, 0, tmpIndex));
            mPreemptiveLoadingListDups = true;
          }
        }
      }
    }
  }

  tmpList.clear();
  tmpCount = mPreemptiveLoadingList.count();

  if (tmpCount > 0) {
    if (global.rcloneLsProcessCount < mMaxRcloneLsProcessCount) {

      // remove duplicates if new indexes were added
      if (mPreemptiveLoadingListDups) {
        for (int i = tmpCount - 1; i >= 0; --i) {
          tmpIndex = mPreemptiveLoadingList.at(i);
          if (!tmpList.contains(tmpIndex)) {
            tmpList.prepend(tmpIndex);
          }
        }
        mPreemptiveLoadingList = tmpList;
        mPreemptiveLoadingListDups = false;
      }

      // one refresh will trigger two rclone processes - lsl and lsd
      int rcloneLsProcessesFreeCount =
          (mMaxRcloneLsProcessCount - global.rcloneLsProcessCount) / 2;

      for (int i = 0; i < rcloneLsProcessesFreeCount; ++i) {
        if (rcloneLsProcessesFreeCount <= 0) {
          break;
        }

        tmpCount = mPreemptiveLoadingList.count();
        for (int j = 0; j < tmpCount; ++j) {
          if (rcloneLsProcessesFreeCount <= 0) {
            break;
          }

          // process from the bottom so last clicked folder is processed first
          QModelIndex index =
              mPreemptiveLoadingList.takeAt(mPreemptiveLoadingList.count() - 1);

          if (!mPreemptiveLoadingListDone.contains(index)) {

            if (!mPreemptiveLoadingListDoneNodes.contains(index)) {
              // force model update
              // model->refresh(index);
              model->index(1, 0, index).isValid();
              rcloneLsProcessesFreeCount--;
              mPreemptiveLoadingListDoneNodes.append(index);
            }
          }
        }
      }
      runAgain = true;
    } else {
      runAgain = true;
    }
  } else {
    if (mPreemptiveLoadingListPending.count() > 0) {
      runAgain = true;
    }
  }

  if (runAgain) {
    QTimer::singleShot(300, Qt::CoarseTimer, this,
                       SLOT(preemptiveLoadingProcessor()));
  }

  return;
}

void RemoteWidget::switchRemoteType() {
  QMutexLocker locker(&preemptiveLoadingProcessorMutex);

  // clear preemptive loading lists
  clearPreemptiveQueues();

  // we can only switch when pending preemptive loading jobs are finished and
  // root is not reloading
  if (global.rcloneLsProcessCount == 0 &&
      !model->isLoading(model->index(0, 0, mRootIndex))) {

    mCount = 0;

    ui.cb_GoogleDriveMode->setDisabled(false);
    setRemoteMode(ui.cb_GoogleDriveMode->currentIndex(), mRemoteType);

    //!!!!!!!!! ???
    // clear top folder's rows
    while (model->removeRow(0, mRootIndex)) {
    }

    ui.tree->selectionModel()->clear();
    ui.tree->selectionModel()->select(
        mRootIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    model->refresh(mRootIndex);
    QTimer::singleShot(0, ui.tree, SLOT(setFocus()));

    ui.tree->showColumn(0);
    ui.tree->showColumn(1);
    ui.tree->showColumn(2);
    ui.path->setAlignment(Qt::AlignLeft);
    ui.path->clear();
    mPreemptiveLoadingListDone.append(mRootIndex);
    QTimer::singleShot(200, Qt::CoarseTimer, this, SLOT(initialModelLoading()));

  } else {

    // there are still rclone ls jobs running - we wait
    mCount++;
    ui.path->setAlignment(Qt::AlignHCenter);

    if (mCount % 2 == 0) {
      if (ui.path->text().isEmpty()) {
        ui.path->setText(
            "*****    Please wait - finishing background jobs    *****");
      } else {
        ui.path->clear();
      }
    }

    QTimer::singleShot(300, Qt::CoarseTimer, this, SLOT(switchRemoteType()));
  }
}

void RemoteWidget::processSelection(const QItemSelection &selected,
                                    const QItemSelection &deselected) {
  if (deselected.empty()) {
  }

  if (selected.empty())
    return;

  QItemSelectionModel *selectionModel = ui.tree->selectionModel();
  QItemSelection selection = selectionModel->selection();

  const QModelIndex parent = ui.tree->currentIndex().parent();
  const QModelIndex currentItem = ui.tree->currentIndex();

  //  QModelIndexList multiSelectionRows =
  //      ui.tree->selectionModel()->selectedRows();
  //  int multiSelectCount = multiSelectionRows.count();

  QItemSelection invalid;
  QItemSelection valid;

  Q_FOREACH (QModelIndex index, selection.indexes()) {

    if (index.parent() == parent) {
      valid.select(index, index);
      continue;
    }

    invalid.select(index, index);
  }

  // for remotes with many roots (e.g. Windows with more than one drive) we dont
  // allow selecting more than one

  if (model->isTopLevel(currentItem)) {

    Q_FOREACH (QModelIndex multiIndex, valid.indexes()) {

      if (multiIndex.row() == currentItem.row())
        continue;

      if (model->isTopLevel(multiIndex)) {
        invalid.select(multiIndex, multiIndex);
      }
    }
  }

  selectionModel->select(invalid, QItemSelectionModel::Deselect);
}

QStringList RemoteWidget::getSelectionFilteringPatterns(
    const QModelIndexList &multiSelection) {

  QStringList includePatternsList;

  int multiSelectionCount = multiSelection.count();

  if (multiSelectionCount == 0) {
    return includePatternsList;
  }

  for (int i = 0; i < multiSelectionCount; ++i) {
    QModelIndex index = multiSelection.at(i);
    QDir path = model->path(index);

    /*
     *  rclone filtering uses its own patterns parser
     *  we have to esacape ? [ ] { } * \
     *  otherwise items containing these characters will be missed
     */
    QString itemName = path.dirName();
    itemName.replace("\\", "\\\\");
    itemName.replace("[", "\\[");
    itemName.replace("]", "\\]");
    itemName.replace("?", "\\?");
    itemName.replace("{", "\\{");
    itemName.replace("}", "\\}");
    itemName.replace("*", "\\*");

    if (model->isFolder(index)) {
      // directory
      includePatternsList << "/" + itemName + "/**";
    } else {
      // file
      includePatternsList << "/" + itemName;
    }
  }

  return includePatternsList;
}

void RemoteWidget::refreshAfterMove() {

  if (model->isLoading(model->index(0, 0, mSrcIndex))) {

    QTimer::singleShot(300, Qt::CoarseTimer, this, SLOT(refreshAfterMove()));

  } else {

    // check if any parent is loading
    // start with mDestIndex and go to the top;
    QModelIndex top = mDestIndex;
    bool isAnyParentLoading = false;
    while (!model->isTopLevel(top)) {
      if (model->isLoading(model->index(0, 0, top))) {
        isAnyParentLoading = true;
      }
      top = top.parent();
    }

    if (!isAnyParentLoading) {
      model->refresh(mDestIndex);
    }
  }

  return;
}

void RemoteWidget::clearPreemptiveQueues() {

  // clear preemptive loading lists
  mPreemptiveLoadingList.clear();
  mPreemptiveLoadingListDone.clear();
  mPreemptiveLoadingListDoneNodes.clear();
  mPreemptiveLoadingList.clear();
  mPreemptiveLoadingListPending.clear();

  return;
}
