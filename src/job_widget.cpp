#include "job_widget.h"
#include "utils.h"

JobWidget::JobWidget(QProcess *process, const QString &info,
                     const QStringList &args, const QString &source,
                     const QString &dest, const QString &uniqueID,
                     const QString &transferMode, const QString &requestId,
                     QWidget *parent)
    : QWidget(parent), mProcess(process) {
  ui.setupUi(this);

  updateStartFinishInfo();

  mArgs = GetRcloneCmd(args);

  ui.showOutput->setToolTip(mArgs.join(" "));

  ui.source->setText(source);
  ui.source->setCursorPosition(0);
  ui.source->setToolTip(source);

  ui.dest->setText(dest);
  ui.dest->setCursorPosition(0);
  ui.dest->setToolTip(dest);

  QString infoTrimmed;

  mRequestId = requestId;
  mUniqueID = uniqueID;
  mTransferMode = transferMode;

  if (info.length() > 140) {
    infoTrimmed = info.left(57) + "..." + info.right(80);
  } else {
    infoTrimmed = info;
  }

  ui.info->setText(infoTrimmed);
  ui.info->setCursorPosition(0);

  ui.details->setVisible(false);

  auto settings = GetSettings();

  int fontsize = 0;
  fontsize = (settings->value("Settings/fontSize").toInt());

#if !defined(Q_OS_MACOS)
  fontsize--;
#endif

  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  QFontMetrics fm(font);

  font.setPointSize(font.pointSize() + fontsize);

  ui.output->setFont(font);
  ui.output->setVisible(false);

  QString iconsColour = settings->value("Settings/iconsColour").toString();

  QString img_add = "";

  if (iconsColour == "white") {
    img_add = "_inv";
  }

  ui.showDetails->setIcon(
      QIcon(":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));
  ui.showOutput->setIcon(
      QIcon(":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));

  ui.showDetails->setIconSize(QSize(24, 24));
  ui.showOutput->setIconSize(QSize(24, 24));

  QObject::connect(
      ui.showDetails, &QToolButton::toggled, this, [=](bool checked) {
        ui.details->setVisible(checked);

        if (checked) {
          ui.showDetails->setIcon(QIcon(
              ":media/images/qbutton_icons/vdownarrow" + img_add + ".png"));
          ui.showDetails->setIconSize(QSize(24, 24));
        } else {
          ui.showDetails->setIcon(QIcon(
              ":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));
          ui.showDetails->setIconSize(QSize(24, 24));
        }
      });

  QObject::connect(
      ui.showOutput, &QToolButton::toggled, this, [=](bool checked) {
        ui.output->setVisible(checked);

        if (checked) {
          ui.showOutput->setIcon(QIcon(
              ":media/images/qbutton_icons/vdownarrow" + img_add + ".png"));
          ui.showOutput->setIconSize(QSize(24, 24));
        } else {
          ui.showOutput->setIcon(QIcon(
              ":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));
          ui.showOutput->setIconSize(QSize(24, 24));
        }
      });

  ui.cancel->setIcon(
      QIcon(":media/images/qbutton_icons/cancel" + img_add + ".png"));
  ui.cancel->setIconSize(QSize(24, 24));

  QObject::connect(ui.cancel, &QToolButton::clicked, this, [=]() {
    if (isRunning) {
      int button = QMessageBox::question(
          this, "Transfer",
          QString(
              "rclone process is still running.\n\nDo you want to stop it?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (button == QMessageBox::Yes) {
        cancel();
      }
    } else {
      emit closed();
    }
  });

  ui.copy->setIcon(
      QIcon(":media/images/qbutton_icons/copy" + img_add + ".png"));
  ui.copy->setIconSize(QSize(24, 24));

  QObject::connect(ui.copy, &QToolButton::clicked, this, [=]() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(mArgs.join(" "));
  });

  QObject::connect(mProcess, &QProcess::readyRead, this, [=]() {
    // regex101.com great for testing regexp
    QRegularExpression rxSize(QRegularExpression::anchoredPattern(
        R"(Transferred:\s+(\S+ \S+) \(([^)]+)\))")); // Until rclone 1.42
    QRegularExpression rxSize2(QRegularExpression::anchoredPattern(
        R"(Transferred:\s+([0-9.]+)(\S)? \/ (\S+) (\S+), ([0-9%-]+), (\S+ \S+), (\S+) (\S+))")); // Starting with rclone 1.43
    QRegularExpression rxSize3(QRegularExpression::anchoredPattern(
        R"(Transferred:\s+([0-9.]+ \w+) \/ ([0-9.]+ \w+), ([0-9%-]+), ([0-9.]+ \w+\/s), \w+ (\S+))")); // Starting with rclone 1.57
    QRegularExpression rxErrors(QRegularExpression::anchoredPattern(
        R"(Errors:\s+(\d+)(.*))")); // captures also following variant:
                                    // "Errors: 123 (bla bla bla)"
    QRegularExpression rxChecks(QRegularExpression::anchoredPattern(
        R"(Checks:\s+(\S+))")); // Until rclone 1.42
    QRegularExpression rxChecks2(QRegularExpression::anchoredPattern(
        R"(Checks:\s+(\S+) \/ (\S+), ([0-9%-]+))"));   // Starting with
                                                       // rclone 1.43
    QRegularExpression rxTransferred(QRegularExpression::anchoredPattern(
        R"(Transferred:\s+(\S+))")); // Until rclone 1.42
    QRegularExpression rxTransferred2(QRegularExpression::anchoredPattern(
        R"(Transferred:\s+(\d+) \/ (\d+), ([0-9%-]+))")); // Starting with
                                                          // rclone 1.43
    QRegularExpression rxTime(
        QRegularExpression::anchoredPattern(R"(Elapsed time:\s+(\S+))"));
    QRegularExpression rxProgress(QRegularExpression::anchoredPattern(
        R"(\*([^:]+):\s*([^%]+)% done.+(ETA: [^)]+))")); // Until rclone 1.38
    QRegularExpression rxProgress2(QRegularExpression::anchoredPattern(
        R"(\*([^:]+):\s*([^%]+)% \/[a-zA-z0-9.]+, [a-zA-z0-9.]+\/s, (\w+))")); // Starting with rclone 1.39
    QRegularExpression rxProgress3(QRegularExpression::anchoredPattern(
        R"(\* ([^:]+):\s*([^%]+%) \/([0-9.]+\w+), ([0-9.]*[a-zA-Z\/]+s)*,)")); // Starting with rclone 1.56
    while (mProcess->canReadLine()) {
      QString line = mProcess->readLine().trimmed();
      if (++mLines == 10000) {
        ui.output->clear();
        mLines = 1;
      }
      ui.output->appendPlainText(line);

      if (line.isEmpty()) {
        for (auto it = mActive.begin(), eit = mActive.end(); it != eit;
             /* empty */) {
          auto label = it.value();
          if (mUpdated.contains(label)) {
            ++it;
          } else {
            it = mActive.erase(it);
            ui.progress->removeWidget(label->buddy());
            ui.progress->removeWidget(label);
            delete label->buddy();
            delete label;
          }
        }
        mUpdated.clear();
        continue;
      }

      QRegularExpressionMatch m;
      if ((m = rxSize.match(line)).hasMatch()) {
        ui.size->setText(m.captured(1));

        ui.progress_info->setStyleSheet(
            "QLabel { color: green; font-weight: bold;}");
        ui.progress_info->setText("(" + m.captured(1) + ")");

        ui.bandwidth->setText(m.captured(2));
      } else if ((m = rxSize2.match(line)).hasMatch()) {
        ui.size->setText(m.captured(1) + " " + m.captured(2) + "B" + ", " +
                         m.captured(5));
        ui.bandwidth->setText(m.captured(6));
        ui.eta->setText(m.captured(8));
        ui.totalsize->setText(m.captured(3) + " " + m.captured(4));

        ui.progress_info->setStyleSheet(
            "QLabel { color: green; font-weight: bold;}");
        ui.progress_info->setText("(" + m.captured(5) + ")");

      } else if ((m = rxSize3.match(line)).hasMatch()) {
        ui.size->setText(m.captured(1) + ", " + m.captured(3));
        ui.bandwidth->setText(m.captured(4));
        ui.eta->setText(m.captured(5));
        ui.totalsize->setText(m.captured(2));
      } else if ((m = rxErrors.match(line)).hasMatch()) {
        ui.errors->setText(m.captured(1));

        if (!(m.captured(1).toInt() == 0)) {
          ui.progress_info->setStyleSheet(
              "QLabel { color: red; font-weight: bold;}");
          ui.errors->setStyleSheet(
              "QLineEdit { color: red; font-weight: normal;}");
        }
      } else if ((m = rxChecks.match(line)).hasMatch()) {
        ui.checks->setText(m.captured(1));
      } else if ((m = rxChecks2.match(line)).hasMatch()) {
        ui.checks->setText(m.captured(1) + " / " + m.captured(2) + ", " +
                           m.captured(3));
      } else if ((m = rxTransferred.match(line)).hasMatch()) {
        ui.transferred->setText(m.captured(1));
      } else if ((m = rxTransferred2.match(line)).hasMatch()) {
        ui.transferred->setText(m.captured(1) + " / " + m.captured(2) + ", " +
                                m.captured(3));
      } else if ((m = rxTime.match(line)).hasMatch()) {
        ui.elapsed->setText(m.captured(1));
      } else if ((m = rxProgress.match(line)).hasMatch()) {
        QString name = m.captured(1).trimmed();

        auto it = mActive.find(name);

        QLabel *label;
        QProgressBar *bar;
        if (it == mActive.end()) {
          label = new QLabel();
          label->setText(name);

          bar = new QProgressBar();
          bar->setMinimum(0);
          bar->setMaximum(100);
          bar->setTextVisible(true);

          label->setBuddy(bar);

          ui.progress->addRow(label, bar);

          mActive.insert(name, label);
        } else {
          label = it.value();
          bar = static_cast<QProgressBar *>(label->buddy());
        }

        bar->setValue(m.captured(2).toInt());
        bar->setToolTip(m.captured(3));

        mUpdated.insert(label);
      } else if ((m = rxProgress2.match(line)).hasMatch()) {
        QString name = m.captured(1).trimmed();

        auto it = mActive.find(name);

        QLabel *label;
        QProgressBar *bar;
        if (it == mActive.end()) {
          label = new QLabel();

          QString nameTrimmed;

          if (name.length() > 47) {
            nameTrimmed = name.left(25) + "..." + name.right(19);
          } else {
            nameTrimmed = name;
          }

          label->setText(nameTrimmed);

          bar = new QProgressBar();
          bar->setMinimum(0);
          bar->setMaximum(100);
          bar->setTextVisible(true);

          label->setBuddy(bar);

          ui.progress->addRow(label, bar);

          mActive.insert(name, label);
        } else {
          label = it.value();
          bar = static_cast<QProgressBar *>(label->buddy());
        }

        bar->setValue(m.captured(2).toInt());
        bar->setToolTip("File name: " + name + "\nFile stats" +
                        m.captured(0).mid(m.captured(0).indexOf(':')));

        mUpdated.insert(label);
      } else if ((m = rxProgress3.match(line)).hasMatch()) {
        QString name = m.captured(1).trimmed();

        auto it = mActive.find(name);

        QLabel *label;
        QProgressBar *bar;
        if (it == mActive.end()) {
          label = new QLabel();

          QString nameTrimmed;

          if (name.length() > 47) {
            nameTrimmed = name.left(25) + "..." + name.right(19);
          } else {
            nameTrimmed = name;
          }

          label->setText(nameTrimmed);

          bar = new QProgressBar();
          bar->setMinimum(0);
          bar->setMaximum(100);
          bar->setTextVisible(true);

          label->setBuddy(bar);

          ui.progress->addRow(label, bar);

          mActive.insert(name, label);
        } else {
          label = it.value();
          bar = static_cast<QProgressBar *>(label->buddy());
        }

        bar->setValue(m.captured(2).toInt());
        bar->setToolTip("File name: " + name + "\nFile stats" +
                        m.captured(0).mid(m.captured(0).indexOf(':')));

        mUpdated.insert(label);
      }
    }
  });

  QObject::connect(
      mProcess,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int status, QProcess::ExitStatus) {
        mProcess->deleteLater();
        for (auto label : mActive) {
          ui.progress->removeWidget(label->buddy());
          ui.progress->removeWidget(label);
          delete label->buddy();
          delete label;
        }

        isRunning = false;
        if (status == 0) {
          if (iconsColour == "white") {
            ui.showDetails->setStyleSheet(
                "QToolButton { border: 0; font-weight: bold;}");
          } else {
            ui.showDetails->setStyleSheet(
                "QToolButton { border: 0; color: black; font-weight: bold;}");
          }
          ui.showDetails->setText("  Finished");
          mJobFinalStatus = "finished";
          mStatus = "1_transfer_finished";
          ui.progress_info->hide();
        } else {
          ui.showDetails->setStyleSheet(
              "QToolButton { border: 0; color: red; font-weight: bold;}");
          ui.showDetails->setText("  Error");

          if (mJobFinalStatus == "stopped") {
          } else {
            mJobFinalStatus = "error";
            mStatus = "2_transfer_error";
          }

          ui.progress_info->hide();
        }

        mFinishDateTime = QDateTime::currentDateTime();
        updateStartFinishInfo();

        ui.cancel->setToolTip("Close");
        ui.cancel->setStatusTip("Close");
        emit finished(ui.info->text(), mJobFinalStatus);
      });

  ui.showDetails->setStyleSheet(
      "QToolButton { border: 0; color: green; font-weight: bold;}");
  ui.showDetails->setText("  Running");
}

JobWidget::~JobWidget() {}

void JobWidget::showDetails() { ui.showDetails->setChecked(true); }

void JobWidget::cancel() {
  if (!isRunning) {
    return;
  }

  mJobFinalStatus = "stopped";
  mStatus = "2_transfer_stopped";

  mProcess->kill();
  mProcess->waitForFinished();

  ui.showDetails->setStyleSheet(
      "QToolButton { border: 0; color: red; font-weight: bold;}");
  ui.showDetails->setText("  Stopped");
  ui.progress_info->hide();
  ui.cancel->setToolTip("Close");
  ui.cancel->setStatusTip("Close");
}

QString JobWidget::getUniqueID() { return mUniqueID; }

QString JobWidget::getRequestId() { return mRequestId; }

QString JobWidget::getTransferMode() { return mTransferMode; }

QDateTime JobWidget::getStartDateTime() { return mStartDateTime; }

void JobWidget::updateStartFinishInfo() {

  ui.le_StartFinishInfo->setText(
      "Started:   " +
      QLocale(QLocale::English)
          .toString(mStartDateTime, "ddd, dd/MMM/yyyy HH:mm:ss t") +
      "              " + "Finished:  " +
      QLocale(QLocale::English)
          .toString(mFinishDateTime, "ddd, dd/MMM/yyyy HH:mm:ss t"));
}

QString JobWidget::getStatus() { return mStatus; }
