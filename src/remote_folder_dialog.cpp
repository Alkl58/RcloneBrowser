#include "remote_folder_dialog.h"
#include "global.h"
#include "list_of_job_options.h"
#include "utils.h"

RemoteFolderDialog::RemoteFolderDialog(
    bool isMove, bool isMultiSelect, QStringList includedList,
    const QModelIndex &sourceIndex, const QString &remote,
    const QString &remoteType, const QString &remoteMode, bool isLocal,
    const QModelIndex &rootIndex, ItemModel *model, QWidget *parent)
    : QDialog(parent) {

  pSourceIndex = sourceIndex;
  mRootIndex = rootIndex;

  ui.setupUi(this);

  // Elided....Text
  QFontMetrics metrix(ui.l_destRemote->font());

  auto settings = GetSettings();

  // set minimumWidth based on font size
  int fontsize = 0;
  fontsize = (settings->value("Settings/fontSize").toInt());
  setMinimumWidth(minimumWidth() + (fontsize * 30));
  resize(0, 0);
  adjustSize();
  resize(0, 0);

  QString root = isLocal ? "/" : QString();

  ui.l_destRemote->setText(
      metrix.elidedText(remote + ":", Qt::ElideMiddle, 150));
  ui.l_destRemote->setToolTip(remote + ":");

  ui.l_sourceRemote->setText(
      metrix.elidedText(remote + ":", Qt::ElideMiddle, 150));
  ui.l_sourceRemote->setToolTip(remote + ":");

  ui.source->setText(model->path(sourceIndex).path());
  ui.source->setCursorPosition(0);

  if (remoteType != "drive") {
    ui.remoteMode->setText(remoteType);
  } else {
    if (remoteMode == "main") {
      ui.remoteMode->setText("drive");
    }
    if (remoteMode == "trash") {
      ui.remoteMode->setText("drive, --drive-trashed-only");
    }
    if (remoteMode == "shared") {
      ui.remoteMode->setText("drive, --drive-shared-with-me");
    }
  }

  ui.verbose->setChecked(true);

  if (isMove) {
    ui.deleteEmptySrcDirs->setChecked(true);
    ui.createEmptySrcDirs->setChecked(true);

  } else {

    ui.deleteEmptySrcDirs->setChecked(false);
    ui.deleteEmptySrcDirs->setEnabled(false);
    ;
    ui.createEmptySrcDirs->setChecked(true);
  }

  ui.le_defaultRcloneOptions->setText(metrix.elidedText(
      settings->value("Settings/defaultRcloneOptions").toString(),
      Qt::ElideMiddle, 537));
  ui.le_defaultRcloneOptions->setToolTip(
      "Set in preferences:\n\n" +
      settings->value("Settings/defaultRcloneOptions").toString());

#ifdef Q_OS_WIN
  // as with Fusion style in Windows QTreeView font size does not scale
  // with QApplication::font() changes we control it manually using style sheet
  QFont defaultFont = QApplication::font();
  int fontSize = defaultFont.pointSize() + 3;

  QString fontStyleSheet =
      QString("QTreeView { font-size: %1px;}").arg(fontSize);
  ui.tree->setStyleSheet(fontStyleSheet);
#endif

  ui.tree->setAlternatingRowColors(
      settings->value("Settings/rowColors", false).toBool());

  ui.tree->sortByColumn(0, Qt::AscendingOrder);
  ui.tree->header()->setSectionsMovable(false);

  ui.tree->setModel(model);
  QTimer::singleShot(0, ui.tree, SLOT(setFocus()));

  ui.buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

  QObject::connect(model, &QAbstractItemModel::layoutChanged, this, [=]() {
    ui.tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.tree->resizeColumnToContents(1);
    ui.tree->resizeColumnToContents(2);
  });

  QObject::connect(
      ui.tree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
      [=]() {
        QModelIndex index;
        QModelIndexList selection = ui.tree->selectionModel()->selectedRows();

        int multiSelectCount = selection.count();

        if (multiSelectCount == 0) {
          return;
        }

        index = selection.at(0);

        bool isFolder = model->isFolder(index);

        if (!isFolder) {
          ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
          ui.destination->clear();

        } else {
          ui.buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);

          QDir path;
          path = model->path(index);

          if (isMultiSelect) {
            path = model->path(index);
            ui.destination->setText(
                isLocal ? QDir::toNativeSeparators(path.path()) : path.path());
          } else {

            if (model->isFolder(sourceIndex)) {
              // folder
              path = model->path(index);
              ui.destination->setText(
                  path.filePath((model->path(sourceIndex)).dirName()));
            } else {
              // file
              path = model->path(index);
              ui.destination->setText(
                  isLocal ? QDir::toNativeSeparators(path.path())
                          : path.path());
            }
          }

          pDestIndex = index;
          ui.destination->setCursorPosition(0);
        }
      });

  QObject::connect(
      ui.tabWidget, &QTabWidget::currentChanged, this, [=](const int &index) {
        if (index == 3) {

          QString rcloneCmd = (QDir::toNativeSeparators(GetRclone()));

          QStringList tmp;

          QStringList rcloneConf = GetRcloneConf();

          QStringList rcloneTransferCmd;

          // rclone executable
          if (!rcloneCmd.isEmpty()) {
            if (rcloneCmd.contains(" ")) {
              rcloneTransferCmd << "\"" + rcloneCmd + "\"";
            } else {
              rcloneTransferCmd << rcloneCmd;
            }
          }

          // rclone config
          if (!rcloneConf.isEmpty()) {
            // --config
            rcloneTransferCmd << rcloneConf.at(0);
            // file location
            if (rcloneConf.at(1).contains(" ")) {
              rcloneTransferCmd
                  << "\"" + QDir::toNativeSeparators(rcloneConf.at(1)) + "\"";
            } else {
              rcloneTransferCmd << QDir::toNativeSeparators(rcloneConf.at(1));
            }
          }

          if (isMove) {
            rcloneTransferCmd << "move";
          } else {
            rcloneTransferCmd << "copy";
          }

          QString src = remote + ":" + ui.source->text();
          QString dest = remote + ":" + ui.destination->text();

          rcloneTransferCmd << GetRemoteModeRcloneOptions();

          if (src.contains(" ")) {
            rcloneTransferCmd << "\"" + src + "\"";
          } else {
            rcloneTransferCmd << src;
          }

          if (dest.contains(" ")) {
            rcloneTransferCmd << "\"" + dest + "\"";
          } else {
            rcloneTransferCmd << dest;
          }

          tmp = getFilterList();

          if (!tmp.isEmpty()) {

            for (int i = 0; i < tmp.count(); i++) {

              rcloneTransferCmd << "--filter";

              if (tmp.at(i).contains(" ")) {
                rcloneTransferCmd << "\"" + tmp.at(i) + "\"";
              } else {
                rcloneTransferCmd << tmp.at(i);
              }
            }
          }

          rcloneTransferCmd << GetDefaultOptionsList("defaultRcloneOptions");

          rcloneTransferCmd << getOptions();

          ui.rcloneCmd->setPlainText(rcloneTransferCmd.join(" "));
        }
      });

  QObject::connect(ui.textFilter, &QPlainTextEdit::textChanged, this, [=]() {
    if (ui.textFilter->toPlainText().trimmed().isEmpty()) {
      ui.tabWidget->setTabText(2, "Filter");
    } else {
      ui.tabWidget->setTabText(2, "Filter(x)");
    }
  });

  if (isMultiSelect) {
    ui.multiInfo->show();

    QString includeItemsText;
    for (int i = 0; i < includedList.count(); ++i) {
      includeItemsText = includeItemsText + includedList.at(i) + "\n";
    }

    ui.textFilter->setPlainText(includeItemsText);

  } else {
    ui.multiInfo->hide();
  }

  if (isMove) {
    setWindowTitle("Rclone Browser - " +
                   metrix.elidedText(remote + ":", Qt::ElideMiddle, 150) +
                   " - choose server side move destination");
  } else {
    setWindowTitle("Rclone Browser - " +
                   metrix.elidedText(remote + ":", Qt::ElideMiddle, 150) +
                   " - choose server side copy destination");
  }

  QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, this,
                   &QDialog::accept);

  QObject::connect(ui.buttonBox, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);

  ui.tree->selectionModel()->select(mRootIndex,
                                    QItemSelectionModel::SelectCurrent |
                                        QItemSelectionModel::Rows);

  ui.tree->expand(mRootIndex);

  ui.tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  ui.tree->resizeColumnToContents(1);
  ui.tree->resizeColumnToContents(2);

  if (settings->value("Settings/rememberLastOptions", false).toBool()) {
    settings->beginGroup("RemoteFolder");
    ReadSettings(settings.get(), this);
    settings->endGroup();
  }

  if (!isMove) {
    ui.deleteEmptySrcDirs->setChecked(false);
  }
}

RemoteFolderDialog::~RemoteFolderDialog() {
  if (result() == QDialog::Accepted) {
    // save dialog options
    auto settings = GetSettings();

    settings->beginGroup("RemoteFolder");

    WriteSettings(settings.get(), this);
    settings->remove("source");
    settings->remove("destination");
    settings->remove("textFilter");
    settings->remove("rcloneCmd");
    settings->endGroup();
  }
}

QStringList RemoteFolderDialog::getOptions() {

  QStringList args;

  if (ui.createEmptySrcDirs->isChecked()) {
    args << "--create-empty-src-dirs";
  }

  if (ui.deleteEmptySrcDirs->isChecked()) {
    args << "--delete-empty-src-dirs";
  }

  if (ui.noTraverse->isChecked()) {
    args << "--no-traverse";
  }

  if (ui.dryRun->isChecked()) {
    args << "--dry-run";
  }

  if (ui.verbose->isChecked()) {
    args << "--verbose";
  }

  if (!ui.pte_textExtra->toPlainText().trimmed().isEmpty()) {
    for (auto line : ui.pte_textExtra->toPlainText().trimmed().split('\n')) {
      if (!line.isEmpty()) {

        for (QString arg :
             line.split(QRegularExpression(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
          if (!arg.isEmpty()) {
            args << arg.replace("\"", "");
          }
        }
      }
    }
  }

  return args;
}

QStringList RemoteFolderDialog::getFilterList() {

  QStringList list;

  if (!ui.textFilter->toPlainText().trimmed().isEmpty()) {

    for (auto line : ui.textFilter->toPlainText().trimmed().split('\n')) {
      if (!line.isEmpty()) {
        list << line;
      }
    }
  }

  return list;
}
