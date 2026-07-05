#pragma once
#include "icon_cache.h"
#include "job_options.h"
#include "pch.h"
#include "ui_main_window.h"
#ifdef Q_OS_MACOS
#include "mac_os_power_saving.h"
#endif

class JobWidget;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

private slots:
  void rcloneGetVersion();
  void rcloneConfig();
  void rcloneListRemotes();
  void listTasks();

  void addTransfer(const QString &message, const QString &source,
                   const QString &dest, const QStringList &args,
                   const QString &uniqueId, const QString &transferMode,
                   const QString &requestId);
  void addStream(const QString &remote, const QString &stream,
                 const QString &remoteType);

  void addNewMount(const QString &remote, const QString &folder,
                   const QString &remoteType, const QStringList &args,
                   const QString &script, const QString &uniqueId,
                   const QString &info);

  void addScheduler(const QString &taskId, const QString &taskName,
                    const QStringList &args);

  void addSavedTransfer(const QString &uniqueId, bool dryRun, bool addToQueue);

  void runScript(const QString &script);

  void slotCloseTab(int index);

  bool saveQueueFile(void);
  bool saveSchedulerFile(void);

  void autoStartMounts(void);

  // quit RB but only when all processes finished
  void quitApp(void);

private:
  Ui::MainWindow ui;

  QSystemTrayIcon mSystemTray;
  // Qt 6 removed QSound; QSoundEffect replaces it for the finished-transfer
  // notification chime (only when the Multimedia module is available).
#ifdef RB_HAVE_MULTIMEDIA
  QSoundEffect mNotificationSound;
#endif
  JobWidget *mLastFinished = nullptr;

  bool mAlwaysShowInTray;
  bool mCloseToTray;
  bool mNotifyFinishedTransfers;
  bool mSoundNotif;

  QLabel *mStatusMessage;

  IconCache mIcons;

  bool mFirstTime = true;
  int mJobCount = 0;

  // keep track of number of active transfers
  int mTransferJobCount = 0;

  // false = Queue Paused, true = Queue running
  bool mQueueStatus = false;

  // number of schedulers
  int mSchedulersCount = 0;
  int mRunningSchedulersCount = 0;

  // number of queued tasks
  int mQueueCount = 0;
  // is queued task running
  bool mQueueTaskRunning = false;

  // make queue logic aware that app is quiting
  // so job is not removed from the queue
  bool mAppQuittingStatus = false;

  // don't sort then stopping all transfers
  bool mDoNotSort = false;

  bool canClose();
  void closeEvent(QCloseEvent *ev) override;
  bool getConfigPassword(QProcess *p);

  // sort QListWidget view/selection
  QList<QListWidgetItem *> sortListWidget(const QList<QListWidgetItem *> &list,
                                          bool sortOrder = false);

  // set screen buttons logic mess in one place
  void setQueueButtons(void);
  void setTasksButtons(void);

  void addEmptyJobsMessage();

  void runItem(JobOptionsListWidgetItem *item, const QString &transferMode,
               const QString &requestId, bool dryrun = false);
  void editSelectedTask();
  QIcon mUploadIcon;
  QIcon mDownloadIcon;
  QIcon mMountIcon;
  QMessageBox *mQuittingErrorMsgBox = NULL;

#ifdef Q_OS_MACOS
  MacOsPowerSaving *mMacOsPowerSaving;
#endif

  // used for tasks transitions - prevent race conditions
  QMutex mMutex;
  QMutex mSaveQueueFileMutex;
  QMutex mSaveSchedulerFileMutex;
  QMutex mStopTaskMutex;
  QMutex mRunTaskMutex;
  QMutex mRunItemMutex;
  QMutex mJobsSortMutex;

  // if waiting for processes we show dialog - this is used to calculate delay
  int mQuitInfoDelay = 0;

  void addTasksToQueue();

  void restoreSchedulersFromFile();

  void sortJobs();
  bool mJobsTimeSortOrder = false;
  bool mJobsStatusSortOrder = false;
  QString mJobsSort = "byDate";
  QString mIconsLayout;
};
