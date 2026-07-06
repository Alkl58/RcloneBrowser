#pragma once

#include "pch.h"
#include "ui_job_widget.h"

class JobWidget : public QWidget {
  Q_OBJECT

public:
  JobWidget(QProcess *process, const QString &info, const QStringList &args,
            const QString &source, const QString &dest, const QString &uniqueID,
            const QString &transferMode, const QString &requestId,
            QWidget *parent = nullptr);
  ~JobWidget();

  void showDetails();
  bool isRunning = true;
  QDateTime getStartDateTime();
  QString getStatus();

public slots:
  void cancel();
  QString getUniqueID();
  QString getRequestId();
  QString getTransferMode();

signals:
  void finished(const QString &info, const QString &jobFinalStatus);
  void closed();

private:
  Ui::JobWidget ui;

  QProcess *mProcess;
  int mLines = 0;

  QStringList mArgs;
  QHash<QString, QLabel *> mActive;
  QSet<QLabel *> mUpdated;

  QString mUniqueID = "";
  QString mTransferMode = "";
  QString mRequestId = "";
  QString mJobFinalStatus = "";

  // 0 - running, 1 - finished, 2 - error
  QString mStatus = "0_transfer_running";

  QDateTime mStartDateTime = QDateTime::currentDateTime();
  QDateTime mFinishDateTime;
  void updateProgress(const QString &name, int value, const QString &toolTip);
  void updateStartFinishInfo();
};
