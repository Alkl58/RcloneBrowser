#include "main_window.h"
#include "job_options.h"
#include "job_widget.h"
#include "list_of_job_options.h"
#include "mount_dialog.h"
#include "mount_widget.h"
#include "preferences_dialog.h"
#include "remote_widget.h"
#include "scheduler_widget.h"
#include "stream_widget.h"
#include "transfer_dialog.h"
#include "utils.h"
#ifdef Q_OS_MACOS
#include "global.h"
#include "mac_os_notifications.h"
#include "mac_os_power_saving.h"
#include "osx_helper.h"
#endif
#include "file_dialog.h"

MainWindow::MainWindow() {

  ui.setupUi(this);

#ifdef Q_OS_MACOS
  // macOS power saving control object
  mMacOsPowerSaving = new MacOsPowerSaving();
#endif

  if (IsPortableMode()) {
    this->setWindowTitle("Rclone Browser - portable mode - BETA release");
  } else {
    this->setWindowTitle("Rclone Browser - BETA release");
  }

  auto settings = GetSettings();

#if defined(Q_OS_WIN)
  // disable "?" WindowContextHelpButton
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

#if !defined(Q_OS_MACOS)
  qApp->setStyle(QStyleFactory::create("Fusion"));

  bool darkMode = settings->value("Settings/darkMode").toBool();

  // enable dark mode for Windows and Linux
  if (darkMode) {

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                         QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text,
                         QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Disabled, QPalette::Light,
                         QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                         QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                         QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                         QColor(127, 127, 127));
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; "
                        "border: 1px solid white;}");
  }

#else

  QString sysInfo = QSysInfo::productVersion();
  // enable dark mode for older macOS
  if (sysInfo == "10.9" || sysInfo == "10.10" || sysInfo == "10.11" ||
      sysInfo == "10.12" || sysInfo == "10.13") {

    qApp->setStyle(QStyleFactory::create("Fusion"));

    auto settings = GetSettings();
    bool darkMode = settings->value("Settings/darkMode").toBool();
    if (darkMode) {

      QPalette darkPalette;
      darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
      darkPalette.setColor(QPalette::WindowText, Qt::white);
      darkPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                           QColor(127, 127, 127));
      darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
      darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
      darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
      darkPalette.setColor(QPalette::ToolTipText, Qt::white);
      darkPalette.setColor(QPalette::Text, Qt::white);
      darkPalette.setColor(QPalette::Disabled, QPalette::Text,
                           QColor(127, 127, 127));
      darkPalette.setColor(QPalette::Disabled, QPalette::Light,
                           QColor(35, 35, 35));
      darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
      darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
      darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
      darkPalette.setColor(QPalette::ButtonText, Qt::white);
      darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                           QColor(127, 127, 127));
      darkPalette.setColor(QPalette::BrightText, Qt::red);
      darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
      darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
      darkPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                           QColor(80, 80, 80));
      darkPalette.setColor(QPalette::HighlightedText, Qt::white);
      darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                           QColor(127, 127, 127));
      qApp->setPalette(darkPalette);
      qApp->setStyleSheet(
          "QToolTip { color: #ffffff; background-color: #2a82da; "
          "border: 1px solid white;}");
    }
  }
#endif

  mSystemTray.setIcon(qApp->windowIcon());
  if (settings->contains("MainWindow/geometry")) {
    restoreGeometry(settings->value("MainWindow/geometry").toByteArray());
  }
  SetRclone(settings->value("Settings/rclone").toString());
  SetRcloneConf(settings->value("Settings/rcloneConf").toString());

  mAlwaysShowInTray =
      settings->value("Settings/alwaysShowInTray", false).toBool();
  mCloseToTray = settings->value("Settings/closeToTray", false).toBool();
  mNotifyFinishedTransfers =
      settings->value("Settings/notifyFinishedTransfers", true).toBool();
  mSoundNotif = settings->value("Settings/soundNotif", false).toBool();

  mSystemTray.setVisible(mAlwaysShowInTray);

  // during first run the lastUsed keys might not exist
  if (!(settings->contains("Settings/lastUsedSourceFolder"))) {
    // if lastUsedSourceFolder does not exist create new empty key
    settings->setValue("Settings/lastUsedSourceFolder", "");
  };
  if (!(settings->contains("Settings/lastUsedDestFolder"))) {
    // if lastUsedDestFolder does not exist create new empty key
    settings->setValue("Settings/lastUsedDestFolder", "");
  };
  if (!(settings->contains("Settings/defaultDownloadOptions"))) {
    // if defaultDownloadOptions does not exist create new empty key
    settings->setValue("Settings/defaultDownloadOptions", "");
  };
#ifdef Q_OS_MACOS
  // for macOS by default exclude .DS_Store files from uploads
  if (!(settings->contains("Settings/defaultUploadOptions"))) {
    // if defaultDownloadOptions does not exist create new empty key
    settings->setValue("Settings/defaultUploadOptions", "--exclude .DS_Store");
  };
#else
  if (!(settings->contains("Settings/defaultUploadOptions"))) {
    // if defaultDownloadOptions does not exist create new empty key
    settings->setValue("Settings/defaultUploadOptions", "");
  };
#endif
  if (!(settings->contains("Settings/defaultRcloneOptions"))) {
    // if defaultRcloneOptions does not exist create new empty key
    settings->setValue("Settings/defaultRcloneOptions", "--fast-list");
  };

  QString buttonStyle = settings->value("Settings/buttonStyle").toString();
  QString buttonSize = settings->value("Settings/buttonSize").toString();
  QString iconsColour = settings->value("Settings/iconsColour").toString();

  QString img_add = "";

  if (iconsColour == "white") {
    img_add = "_inv";
  }

  ui.actionRefresh->setIcon(
      QIcon(":media/images/qbutton_icons/refresh" + img_add + ".png"));
  ui.actionOpen->setIcon(
      QIcon(":media/images/qbutton_icons/open_remote" + img_add + ".png"));
  ui.actionConfig->setIcon(
      QIcon(":media/images/qbutton_icons/rclone_config" + img_add + ".png"));
  // Preferences button action is triggered via slot defined in ui file
  // as we dont want pref icon in the menu
  ui.buttonPrefs->setIcon(
      QIcon(":media/images/qbutton_icons/preferences" + img_add + ".png"));

  ui.actionStopAllTransfers->setIcon(
      QIcon(":media/images/qbutton_icons/stop" + img_add + ".png"));
  ui.actionCleanNotRunning->setIcon(
      QIcon(":media/images/qbutton_icons/purge" + img_add + ".png"));
  // triggered via slot so button text can be different than action menu
  ui.buttonSortByTime->setIcon(
      QIcon(":media/images/qbutton_icons/sortTimeZA" + img_add + ".png"));
  ui.buttonSortByStatus->setIcon(
      QIcon(":media/images/qbutton_icons/sortZA" + img_add + ".png"));

  ui.actionDryRun->setIcon(
      QIcon(":media/images/qbutton_icons/dryrun" + img_add + ".png"));
  ui.actionRun->setIcon(
      QIcon(":media/images/qbutton_icons/run" + img_add + ".png"));
  ui.actionEdit->setIcon(
      QIcon(":media/images/qbutton_icons/edit" + img_add + ".png"));
  ui.actionDelete->setIcon(
      QIcon(":media/images/qbutton_icons/purge" + img_add + ".png"));
  ui.actionStop->setIcon(
      QIcon(":media/images/qbutton_icons/stop" + img_add + ".png"));

  // triggered via slot so button text can be different than action menu
  ui.buttonAddToQueue->setIcon(
      QIcon(":media/images/qbutton_icons/addtoqueue" + img_add + ".png"));
  ui.actionAddToQueue->setIcon(
      QIcon(":media/images/qbutton_icons/addtoqueue" + img_add + ".png"));
  ui.buttonAddToScheduler->setIcon(
      QIcon(":media/images/qbutton_icons/addtoscheduler" + img_add + ".png"));
  ui.actionAddToScheduler->setIcon(
      QIcon(":media/images/qbutton_icons/addtoscheduler" + img_add + ".png"));

  ui.actionStartQueue->setIcon(
      QIcon(":media/images/qbutton_icons/run" + img_add + ".png"));
  ui.actionStopQueue->setIcon(
      QIcon(":media/images/qbutton_icons/stop" + img_add + ".png"));
  ui.actionPurgeQueue->setIcon(
      QIcon(":media/images/qbutton_icons/purge" + img_add + ".png"));
  ui.actionRemoveFromQueue->setIcon(
      QIcon(":media/images/qbutton_icons/removefromqueue" + img_add + ".png"));
  ui.actionUpQueue->setIcon(
      QIcon(":media/images/qbutton_icons/vuparrow" + img_add + ".png"));
  ui.actionDownQueue->setIcon(
      QIcon(":media/images/qbutton_icons/vdownarrow" + img_add + ".png"));

  ui.actionStartScheduler->setIcon(
      QIcon(":media/images/qbutton_icons/run" + img_add + ".png"));
  ui.actionStopScheduler->setIcon(
      QIcon(":media/images/qbutton_icons/stop" + img_add + ".png"));

  QPixmap arrowDownPixmap(":media/images/qbutton_icons/arrowdown" + img_add +
                          ".png");
  QPixmap arrowUpPixmap(":media/images/qbutton_icons/arrowup" + img_add +
                        ".png");
  QPixmap mount1Pixmap(":media/images/qbutton_icons/mount1" + img_add + ".png");

  QPixmap sortZAPixmap(":media/images/qbutton_icons/sortZA" + img_add + ".png");
  QPixmap sortAZPixmap(":media/images/qbutton_icons/sortAZ" + img_add + ".png");
  QPixmap sortTimeZAPixmap(":media/images/qbutton_icons/sortTimeZA" + img_add +
                           ".png");
  QPixmap sortTimeAZPixmap(":media/images/qbutton_icons/sortTimeAZ" + img_add +
                           ".png");

  QIcon arrowDownIcon(arrowDownPixmap);
  QIcon arrowUpIcon(arrowUpPixmap);
  QIcon mount1Icon(mount1Pixmap);
  QIcon sortZAIcon(sortZAPixmap);
  QIcon sortAZIcon(sortAZPixmap);
  QIcon sortTimeZAIcon(sortTimeZAPixmap);
  QIcon sortTimeAZIcon(sortTimeAZPixmap);

  ui.buttonDryrunTask->setDefaultAction(ui.actionDryRun);
  ui.buttonRunTask->setDefaultAction(ui.actionRun);
  ui.buttonStop->setDefaultAction(ui.actionStop);
  ui.buttonEditTask->setDefaultAction(ui.actionEdit);
  ui.buttonDeleteTask->setDefaultAction(ui.actionDelete);
  ui.buttonSortTask->setDefaultAction(ui.actionSortTask);
  ui.refresh->setDefaultAction(ui.actionRefresh);
  ui.open->setDefaultAction(ui.actionOpen);
  ui.config->setDefaultAction(ui.actionConfig);
  //  ui.buttonPrefs->setDefaultAction(ui.preferences);
  ui.buttonStopAllJobs->setDefaultAction(ui.actionStopAllTransfers);
  ui.buttonCleanNotRunning->setDefaultAction(ui.actionCleanNotRunning);
  // ui.buttonAddToQueue->setDefaultAction(ui.actionAddToQueue);
  ui.buttonStartQueue->setDefaultAction(ui.actionStartQueue);
  ui.buttonStopQueue->setDefaultAction(ui.actionStopQueue);
  ui.buttonPurgeQueue->setDefaultAction(ui.actionPurgeQueue);
  ui.buttonRemoveFromQueue->setDefaultAction(ui.actionRemoveFromQueue);
  ui.buttonDownQueue->setDefaultAction(ui.actionDownQueue);
  ui.buttonUpQueue->setDefaultAction(ui.actionUpQueue);
  ui.buttonStartScheduler->setDefaultAction(ui.actionStartScheduler);
  ui.buttonStopScheduler->setDefaultAction(ui.actionStopScheduler);

  // overwrite button text, we want different menu name and different conextual
  // menu
  ui.buttonPrefs->setText("Prefs");
  ui.buttonAddToQueue->setText("Add");
  ui.buttonAddToScheduler->setText("Add");

  // open remote should be not active when there is
  // no foucs on any e.g. after start
  ui.open->setEnabled(false);

  // jobs buttons inactive after start
  ui.buttonStopAllJobs->setEnabled(false);
  ui.buttonCleanNotRunning->setEnabled(false);
  ui.buttonSortByTime->setEnabled(false);
  ui.buttonSortByStatus->setEnabled(false);

  int icon_w = 16;
  int icon_h = 16;
  //  int button_width = 32;

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

    ui.buttonDryrunTask->setIconSize(QSize(icon_w, icon_h));
    ui.buttonRunTask->setIconSize(QSize(icon_w, icon_h));
    ui.buttonEditTask->setIconSize(QSize(icon_w, icon_h));
    ui.buttonDeleteTask->setIconSize(QSize(icon_w, icon_h));
    ui.buttonSortTask->setIconSize(QSize(icon_w, icon_h));
    ui.buttonPrefs->setIconSize(QSize(icon_w, icon_h));
    ui.refresh->setIconSize(QSize(icon_w, icon_h));
    ui.open->setIconSize(QSize(icon_w, icon_h));
    ui.config->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStopAllJobs->setIconSize(QSize(icon_w, icon_h));
    ui.buttonCleanNotRunning->setIconSize(QSize(icon_w, icon_h));
    ui.buttonAddToQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStartQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStopQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonPurgeQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonRemoveFromQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonDownQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonUpQueue->setIconSize(QSize(icon_w, icon_h));

    ui.buttonDryrunTask->setMinimumWidth(button_width);
    ui.buttonRunTask->setMinimumWidth(button_width);
    ui.buttonEditTask->setMinimumWidth(button_width);
    ui.buttonDeleteTask->setMinimumWidth(button_width);
    ui.buttonSortTask->setMinimumWidth(button_width);
    ui.buttonPrefs->setMinimumWidth(button_width);
    ui.refresh->setMinimumWidth(button_width);
    ui.open->setMinimumWidth(button_width);
    ui.config->setMinimumWidth(button_width);
    ui.buttonStopAllJobs->setMinimumWidth(button_width);
    ui.buttonCleanNotRunning->setMinimumWidth(button_width);
    ui.buttonAddToQueue->setMinimumWidth(button_width);
    ui.buttonStartQueue->setMinimumWidth(button_width);
    ui.buttonStopQueue->setMinimumWidth(button_width);
    ui.buttonPurgeQueue->setMinimumWidth(button_width);
    ui.buttonRemoveFromQueue->setMinimumWidth(button_width);
    ui.buttonDownQueue->setMinimumWidth(button_width);

    ui.buttonUpQueue->setIconSize(QSize(icon_w, icon_h));
    ui.buttonUpQueue->setMinimumWidth(button_width);
    ui.buttonStop->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStop->setMinimumWidth(button_width);
    ui.buttonAddToScheduler->setIconSize(QSize(icon_w, icon_h));
    ui.buttonAddToScheduler->setMinimumWidth(button_width);
    ui.buttonStopScheduler->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStopScheduler->setMinimumWidth(button_width);
    ui.buttonStartScheduler->setIconSize(QSize(icon_w, icon_h));
    ui.buttonStartScheduler->setMinimumWidth(button_width);
    ui.buttonSortByTime->setIconSize(QSize(icon_w, icon_h));
    ui.buttonSortByTime->setMinimumWidth(button_width);
    ui.buttonSortByStatus->setIconSize(QSize(icon_w, icon_h));
    ui.buttonSortByStatus->setMinimumWidth(button_width);

  } else {
    if (buttonStyle == "textonly") {
      ui.buttonDryrunTask->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonRunTask->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonEditTask->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonDeleteTask->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonSortTask->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonPrefs->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.refresh->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.open->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.config->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStopAllJobs->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonCleanNotRunning->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonAddToQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStartQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStopQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonPurgeQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonRemoveFromQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonDownQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);

      ui.buttonDryrunTask->setMinimumWidth(button_width);
      ui.buttonRunTask->setMinimumWidth(button_width);
      ui.buttonEditTask->setMinimumWidth(button_width);
      ui.buttonDeleteTask->setMinimumWidth(button_width);
      ui.buttonSortTask->setMinimumWidth(button_width);
      ui.buttonPrefs->setMinimumWidth(button_width);
      ui.refresh->setMinimumWidth(button_width);
      ui.open->setMinimumWidth(button_width);
      ui.config->setMinimumWidth(button_width);
      ui.buttonStopAllJobs->setMinimumWidth(button_width);
      ui.buttonCleanNotRunning->setMinimumWidth(button_width);
      ui.buttonAddToQueue->setMinimumWidth(button_width);
      ui.buttonStartQueue->setMinimumWidth(button_width);
      ui.buttonStopQueue->setMinimumWidth(button_width);
      ui.buttonPurgeQueue->setMinimumWidth(button_width);
      ui.buttonRemoveFromQueue->setMinimumWidth(button_width);
      ui.buttonDownQueue->setMinimumWidth(button_width);
      ui.buttonUpQueue->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonUpQueue->setMinimumWidth(button_width);
      ui.buttonStop->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStop->setMinimumWidth(button_width);
      ui.buttonAddToScheduler->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonAddToScheduler->setMinimumWidth(button_width);
      ui.buttonStopScheduler->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStopScheduler->setMinimumWidth(button_width);
      ui.buttonStartScheduler->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonStartScheduler->setMinimumWidth(button_width);
      ui.buttonSortByTime->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonSortByTime->setMinimumWidth(button_width);
      ui.buttonSortByStatus->setToolButtonStyle(Qt::ToolButtonTextOnly);
      ui.buttonSortByStatus->setMinimumWidth(button_width);

    } else {
      // button style - icononly
      ui.buttonDryrunTask->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonDryrunTask->setIconSize(QSize(icon_w, icon_h));
      ui.buttonRunTask->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonRunTask->setIconSize(QSize(icon_w, icon_h));
      ui.buttonEditTask->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonEditTask->setIconSize(QSize(icon_w, icon_h));
      ui.buttonDeleteTask->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonDeleteTask->setIconSize(QSize(icon_w, icon_h));
      ui.buttonSortTask->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonSortTask->setIconSize(QSize(icon_w, icon_h));
      ui.buttonPrefs->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonPrefs->setIconSize(QSize(icon_w, icon_h));
      ui.refresh->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.refresh->setIconSize(QSize(icon_w, icon_h));
      ui.open->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.open->setIconSize(QSize(icon_w, icon_h));
      ui.config->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.config->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStopAllJobs->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStopAllJobs->setIconSize(QSize(icon_w, icon_h));
      ui.buttonCleanNotRunning->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonCleanNotRunning->setIconSize(QSize(icon_w, icon_h));
      ui.buttonAddToQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonAddToQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStartQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStartQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStopQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStopQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonPurgeQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonPurgeQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonRemoveFromQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonRemoveFromQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonDownQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonDownQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonUpQueue->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonUpQueue->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStop->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStop->setIconSize(QSize(icon_w, icon_h));
      ui.buttonAddToScheduler->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonAddToScheduler->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStopScheduler->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStopScheduler->setIconSize(QSize(icon_w, icon_h));
      ui.buttonStartScheduler->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonStartScheduler->setIconSize(QSize(icon_w, icon_h));
      ui.buttonSortByTime->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonSortByTime->setIconSize(QSize(icon_w, icon_h));
      ui.buttonSortByStatus->setToolButtonStyle(Qt::ToolButtonIconOnly);
      ui.buttonSortByStatus->setIconSize(QSize(icon_w, icon_h));
    }
  }

  // initial sortByTime
  ui.buttonSortByStatus->setStyleSheet("QToolButton {border: 0;}");

  // layout spacers - otherwise layout of different tabs differ due to checkbox
  // on queue tab
  ui.layoutcontrol1->hide();
  ui.layoutcontrol2->hide();
  ui.layoutcontrol3->hide();
  ui.layoutcontrol1_sch->hide();
  ui.layoutcontrol1_queue->hide();

  // statusTips
  ui.actionRefresh->setStatusTip("Refresh remotes view");
  ui.actionConfig->setStatusTip("rclone config");
  ui.actionOpen->setStatusTip("Open remote");
  ui.preferences->setStatusTip("Rclone Browser preferences (ALT-p)");

  ui.actionStopAllTransfers->setStatusTip("Stop all running transfer jobs");
  ui.actionCleanNotRunning->setStatusTip("Remove all not running jobs");

  ui.actionDryRun->setStatusTip("Dry Run all selected tasks at once");
  ui.actionRun->setStatusTip("Run all selected tasks at once");
  ui.actionStop->setStatusTip("Stop all selected tasks");

  ui.actionEdit->setStatusTip("Edit selected task");
  ui.actionDelete->setStatusTip(
      "Delete selected tasks - only not running tasks can be deleted.");

  ui.actionAddToQueue->setStatusTip("Add selected transfer tasks to the queue");
  ui.actionAddToScheduler->setStatusTip(
      "Add selected transfer tasks to the scheduler");
  ui.actionSortTask->setStatusTip("Sort tasks by name");

  ui.actionStartQueue->setStatusTip(
      "Start actively processing items from the queue");
  ui.actionStopQueue->setStatusTip(
      "Stop the queue processing. Active task will be cancelled.");
  ui.actionUpQueue->setStatusTip("Move task up in the queue");
  ui.actionDownQueue->setStatusTip("Move task down in the queue");
  ui.actionRemoveFromQueue->setStatusTip("Remove from the queue");
  ui.actionPurgeQueue->setStatusTip(
      "Remove all not running tasks from the queue");

  ui.buttonStopScheduler->setStatusTip("Stop scheduler");
  ui.buttonStartScheduler->setStatusTip(
      "Start all previously active schedulers");

  ui.buttonStartScheduler->setEnabled(false);
  ui.buttonStopAllJobs->setEnabled(false);
  ui.buttonCleanNotRunning->setEnabled(false);

  // initial queue status
  ui.labelQueueInfoStop->setText("Queue is not running.");
  setQueueButtons();

  ui.tabs->setTabText(4, QString("Scheduler (0)>>(0)"));
  ui.labelSchedulerInfoStart->setText("Scheduler is running.");
  ui.labelSchedulerInfoStop->setText("Scheduler is not running.");
  ui.labelSchedulerInfoStart->show();
  ui.labelSchedulerInfoStop->hide();

  QObject::connect(ui.preferences, &QAction::triggered, this, [=]() {
    PreferencesDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
      auto settings = GetSettings();
      settings->setValue("Settings/rclone", dialog.getRclone().trimmed());
      settings->setValue("Settings/rcloneConf",
                         dialog.getRcloneConf().trimmed());
      settings->setValue("Settings/stream", dialog.getStream());
      settings->setValue("Settings/mount", dialog.getMount());
      settings->setValue("Settings/defaultDownloadDir",
                         dialog.getDefaultDownloadDir().trimmed());
      settings->setValue("Settings/defaultUploadDir",
                         dialog.getDefaultUploadDir().trimmed());
      settings->setValue("Settings/defaultDownloadOptions",
                         dialog.getDefaultDownloadOptions().trimmed());
      settings->setValue("Settings/defaultUploadOptions",
                         dialog.getDefaultUploadOptions().trimmed());
      settings->setValue("Settings/defaultRcloneOptions",
                         dialog.getDefaultRcloneOptions().trimmed());

      settings->setValue("Settings/checkRcloneBrowserUpdates",
                         dialog.getCheckRcloneBrowserUpdates());
      settings->setValue("Settings/checkRcloneUpdates",
                         dialog.getCheckRcloneUpdates());

      settings->setValue("Settings/alwaysShowInTray",
                         dialog.getAlwaysShowInTray());
      settings->setValue("Settings/closeToTray", dialog.getCloseToTray());
      settings->setValue("Settings/startMinimisedToTray",
                         dialog.getStartMinimisedToTray());

      settings->setValue("Settings/notifyFinishedTransfers",
                         dialog.getNotifyFinishedTransfers());
      settings->setValue("Settings/soundNotif", dialog.getSoundNotif());

      settings->setValue("Settings/showFolderIcons",
                         dialog.getShowFolderIcons());
      settings->setValue("Settings/showFileIcons", dialog.getShowFileIcons());
      settings->setValue("Settings/rowColors", dialog.getRowColors());
      settings->setValue("Settings/showHidden", dialog.getShowHidden());

      settings->setValue("Settings/darkMode", dialog.getDarkMode());
      settings->setValue("Settings/rememberLastOptions",
                         dialog.getRememberLastOptions());

      settings->setValue("Settings/buttonStyle",
                         dialog.getButtonStyle().trimmed());
      settings->setValue("Settings/iconsLayout",
                         dialog.getIconsLayout().trimmed());
      settings->setValue("Settings/iconsColour",
                         dialog.getIconsColour().trimmed());

      settings->setValue("Settings/fontSize", dialog.getFontSize().trimmed());
      settings->setValue("Settings/buttonSize",
                         dialog.getButtonSize().trimmed());
      settings->setValue("Settings/iconSize", dialog.getIconSize().trimmed());

      settings->setValue("Settings/useProxy", dialog.getUseProxy());
      settings->setValue("Settings/http_proxy",
                         dialog.getHttpProxy().trimmed());
      settings->setValue("Settings/https_proxy",
                         dialog.getHttpsProxy().trimmed());
      settings->setValue("Settings/no_proxy", dialog.getNoProxy().trimmed());

      settings->setValue("Settings/preemptiveLoading",
                         dialog.getPreemptiveLoading());
      settings->setValue("Settings/preemptiveLoadingLevel",
                         dialog.getPreemptiveLoadingLevel().trimmed());

      settings->setValue("Settings/queueScript",
                         dialog.getQueueScript().trimmed());
      settings->setValue("Settings/transferOnScript",
                         dialog.getTransferOnScript().trimmed());
      settings->setValue("Settings/transferOffScript",
                         dialog.getTransferOffScript().trimmed());

      settings->setValue("Settings/queueScriptRun", dialog.getQueueScriptRun());
      settings->setValue("Settings/jobStartScriptRun",
                         dialog.getJobStartScriptRun());
      settings->setValue("Settings/jobLastFinishedScriptRun",
                         dialog.getJobLastFinishedScriptRun());

      SetRclone(dialog.getRclone());
      SetRcloneConf(dialog.getRcloneConf());
      mFirstTime = true;
      rcloneGetVersion();

      mAlwaysShowInTray = dialog.getAlwaysShowInTray();
      mCloseToTray = dialog.getCloseToTray();
      mNotifyFinishedTransfers = dialog.getNotifyFinishedTransfers();
      mSoundNotif = dialog.getSoundNotif();

      mSystemTray.setVisible(mAlwaysShowInTray);
    }
  });

  // intercept tab closure
  MainWindow::connect(ui.tabs, SIGNAL(tabCloseRequested(int)), this,
                      SLOT(slotCloseTab(int)));

  QObject::connect(ui.quit, &QAction::triggered, this, [=]() {
    mCloseToTray = false;
    close();
  });

  QObject::connect(ui.about, &QAction::triggered, this, [=]() {
    QMessageBox::about(
        this, "Rclone Browser",
        QString(
            R"(<h3>GUI for rclone, v)" RCLONE_BROWSER_VERSION "</h3>"

            R"(<p>Copyright &copy; 2019-2020 <a href="https://github.com/kapitainsky/RcloneBrowser/blob/master/LICENSE">kapitainsky</a></p>)"

            R"(<p>Current development and maintenance<br /><a href="https://github.com/kapitainsky/RcloneBrowser">kapitainsky</a></p>)"

            R"(<p>New features and fixes<br /><a href="https://github.com/kapitainsky/RcloneBrowser/graphs/contributors">contributors</a></p>)"

            R"(<p>Original version<br /><a href="https://mmozeiko.github.io/RcloneBrowser">Martins Mozeiko</a></p>)"));
  });
  QObject::connect(ui.aboutQt, &QAction::triggered, qApp,
                   &QApplication::aboutQt);

  QObject::connect(ui.remotes, &QListWidget::currentItemChanged, this, [=]() {
    if (ui.remotes->selectedItems().empty()) {
      ui.open->setEnabled(false);
    } else {
      ui.open->setEnabled(true);
    }
  });

  QObject::connect(ui.remotes, &QListWidget::itemSelectionChanged, this, [=]() {
    if (ui.remotes->selectedItems().empty()) {
      ui.open->setEnabled(false);
    } else {
      ui.open->setEnabled(true);
    }
  });

  QObject::connect(ui.remotes, &QListWidget::itemChanged, this, [=]() {
    if (ui.remotes->selectedItems().empty()) {
      ui.open->setEnabled(false);
    } else {
      ui.open->setEnabled(true);
    }
  });

  QObject::connect(ui.remotes, &QListWidget::itemClicked, this, [=]() {
    if (ui.remotes->selectedItems().empty()) {
      ui.open->setEnabled(false);
    } else {
      ui.open->setEnabled(true);
    }
  });

  QObject::connect(ui.remotes, &QListWidget::itemActivated, ui.open,
                   &QPushButton::clicked);

  QObject::connect(ui.config, &QPushButton::clicked, this,
                   &MainWindow::rcloneConfig);

  QObject::connect(ui.refresh, &QPushButton::clicked, this,
                   &MainWindow::rcloneListRemotes);

  QObject::connect(ui.open, &QPushButton::clicked, this, [=]() {
    if (ui.remotes->selectedItems().size() != 0) {
      auto item = ui.remotes->selectedItems().front();
      QString type = item->data(Qt::UserRole).toString();
      QString name = item->text();
      QString remoteType = type;

      auto remote = new RemoteWidget(&mIcons, name, remoteType, ui.tabs);

      QObject::connect(remote, &RemoteWidget::addNewMount, this,
                       &MainWindow::addNewMount);
      QObject::connect(remote, &RemoteWidget::addStream, this,
                       &MainWindow::addStream);
      QObject::connect(remote, &RemoteWidget::addTransfer, this,
                       &MainWindow::addTransfer);
      QObject::connect(remote, &RemoteWidget::addSavedTransfer, this,
                       &MainWindow::addSavedTransfer);

      QString nameTrimmed = name;

      if (name.length() > 15) {
        nameTrimmed = nameTrimmed.left(12) + "...";
      }

      int index = ui.tabs->addTab(remote, nameTrimmed);
      ui.tabs->setTabToolTip(index,
                             "type: " + remoteType + "\n\nname: " + name);
      ui.tabs->setCurrentIndex(index);
    }
  });

  QObject::connect(ui.tabs, &QTabWidget::tabCloseRequested, this,
                   [=](const int &index) {
                     // delete remote widget when tab closed
                     ui.tabs->widget(index)->deleteLater();
                   });

  QObject::connect(ui.tabs, &QTabWidget::tabCloseRequested, ui.tabs,
                   &QTabWidget::removeTab);

  QObject::connect(ui.tasksListWidget, &QListWidget::itemClicked, this,
                   [=]() { setTasksButtons(); });

  QObject::connect(ui.tasksListWidget, &QListWidget::currentItemChanged, this,
                   [=]() { setTasksButtons(); });

  QObject::connect(ui.tasksListWidget, &QListWidget::itemChanged, this,
                   [=]() { setTasksButtons(); });

  QObject::connect(ui.tasksListWidget, &QListWidget::itemSelectionChanged, this,
                   [=]() { setTasksButtons(); });

  QObject::connect(ui.tasksListWidget, &QListWidget::itemDoubleClicked, this,
                   [=]() { editSelectedTask(); });

  bool sortTask = settings->value("Settings/sortTask").toBool();

  if (sortTask) {
    ui.actionSortTask->setIcon(sortZAIcon);
    ui.buttonSortTask->setToolTip("Sort Descending");
    ui.tasksListWidget->setSortingEnabled(true);
    ui.tasksListWidget->sortItems(Qt::AscendingOrder);
  } else {
    ui.actionSortTask->setIcon(sortAZIcon);
    ui.buttonSortTask->setToolTip("Sort Ascending");
    ui.tasksListWidget->setSortingEnabled(true);
    ui.tasksListWidget->sortItems(Qt::DescendingOrder);
  }

  QObject::connect(
      ui.tasksListWidget, &QWidget::customContextMenuRequested, this,
      [=](const QPoint &pos) {
        setTasksButtons();
        auto items = ui.tasksListWidget->selectedItems();
        bool isMount = false;
        bool isRunning = false;
        bool isScheduled = false;
        int schedulersCount = ui.schedulers->count();

        if (items.count() > 0) {

          foreach (auto i, items) {
            JobOptionsListWidgetItem *item =
                static_cast<JobOptionsListWidgetItem *>(i);
            JobOptions *jo = item->GetData();

            if (jo->operation == JobOptions::Mount) {
              isMount = true;
            }

            int widgetsCount = ui.jobs->count();
            for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
              QWidget *widget = ui.jobs->itemAt(j)->widget();

              if (auto transfer = qobject_cast<JobWidget *>(widget)) {
                if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
                    (transfer->isRunning)) {
                  isRunning = true;
                }
              }

              if (auto mount = qobject_cast<MountWidget *>(widget)) {
                if ((mount->getUniqueID() == jo->uniqueId.toString()) &&
                    (mount->isRunning)) {
                  isRunning = true;
                }
              }

              if (!isScheduled) {
                for (int k = schedulersCount - 2; k >= 0; k = k - 2) {
                  QWidget *widget = ui.schedulers->itemAt(k)->widget();
                  if (auto scheduler =
                          qobject_cast<SchedulerWidget *>(widget)) {

                    if (scheduler->getSchedulerTaskId() ==
                        jo->uniqueId.toString()) {

                      isScheduled = true;
                      break;
                    }
                  }
                }
              }
            }
          }

          QMenu menu;
          if (!isMount) {
            menu.addAction(ui.actionAddToQueue);
            menu.addAction(ui.actionAddToScheduler);
          }
          menu.addSeparator();
          if (!isMount) {
            if (!isRunning) {
              menu.addAction(ui.actionDryRun);
            }
          }
          if (!isRunning) {
            menu.addAction(ui.actionRun);
          }
          if (isRunning) {
            menu.addAction(ui.actionStop);
          }
          menu.addSeparator();
          if (items.count() == 1) {
            menu.addAction(ui.actionEdit);
          }
          if (!isRunning && !isScheduled) {
            menu.addAction(ui.actionDelete);
          }
          menu.exec(ui.tasksListWidget->viewport()->mapToGlobal(pos));
        }
      });

  QObject::connect(ui.queueListWidget, &QWidget::customContextMenuRequested,
                   this, [=](const QPoint &pos) {
                     QMenu menu;
                     menu.addAction(ui.actionUpQueue);
                     menu.addAction(ui.actionDownQueue);
                     menu.addSeparator();
                     menu.addAction(ui.actionRemoveFromQueue);
                     menu.exec(
                         ui.queueListWidget->viewport()->mapToGlobal(pos));
                   });

  QObject::connect(ui.actionCleanNotRunning, &QAction::triggered, this, [=]() {
    int jobsCount = ((ui.jobs->count() - 2) / 2 - mJobCount);

    int button = QMessageBox::question(this, "Rclone Browser",
                                       QString("There are %1 inactive job(s).\n"
                                               "\nDo you want to clean them?")
                                           .arg(jobsCount),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (button == QMessageBox::Yes) {
      int widgetsCount = ui.jobs->count();
      for (int i = widgetsCount - 2; i >= 0; i = i - 2) {
        QWidget *widget = ui.jobs->itemAt(i)->widget();
        if (auto mount = qobject_cast<MountWidget *>(widget)) {
          if (!(mount->isRunning)) {
            emit mount->closed();
          }
        } else if (auto transfer = qobject_cast<JobWidget *>(widget)) {
          if (!(transfer->isRunning)) {
            emit transfer->closed();
          }
        } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
          if (!(stream->isRunning)) {
            emit stream->closed();
          }
        }
      }
    }
  });

  //!!!  QObject::connect(ui.actionStopAllTransfers
  QObject::connect(ui.actionStopAllTransfers, &QAction::triggered, this, [=]() {
    mDoNotSort = true;

    // we only stop transfer jobs - others are intact
    if (mTransferJobCount != 0) {

      int button = QMessageBox::question(
          this, "Rclone Browser",
          QString("There are %1 transfer job(s) running.\n"
                  "\nDo you want to stop  them?")
              .arg(mTransferJobCount),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

      if (button == QMessageBox::Yes) {

        // if queue is running and active stop it
        // to prevent race condition with new queue task auto starting
        // (triggered by stopping tasks)
        bool queueActive = false;

        if ((mQueueStatus == true) && mQueueTaskRunning &&
            (ui.queueListWidget->count() > 0)) {
          queueActive = true;
          mQueueStatus = false;
          /// remove top task from queue + save it
          ui.queueListWidget->takeItem(0);
          --mQueueCount;
          saveQueueFile();

          if (mQueueCount == 0) {
            ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
          } else {

            if (!mQueueTaskRunning) {
              ui.tabs->setTabText(3,
                                  QString("Queue (%1)>>(0)").arg(mQueueCount));
            } else {
              ui.tabs->setTabText(
                  3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
            }
          }
          setQueueButtons();
        }

        // now safely terminate all running transfers
        int widgetsCount = ui.jobs->count();
        for (int i = widgetsCount - 2; i >= 0; i = i - 2) {
          QWidget *widget = ui.jobs->itemAt(i)->widget();
          if (auto transfer = qobject_cast<JobWidget *>(widget)) {
            if ((transfer->isRunning)) {
              emit transfer->cancel();
            }
          }
        }

        // restart queue if it was active before
        if (queueActive == true) {
          mQueueStatus = true;

          if (ui.queueListWidget->count() == 0) {
            auto settings = GetSettings();
            if (settings->value("Settings/queueScriptRun", false).toBool()) {
              QString queueScript =
                  settings->value("Settings/queueScript").toString();
              if (!queueScript.isEmpty()) {
                runScript(queueScript);
              }
            }
          }

          if (ui.queueListWidget->count() > 0) {
            // start task
            // no transfer job is running as we just stopped all - we can start
            // without checking other running jobs
            JobOptionsListWidgetItem *item =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.queueListWidget->item(0));
            mQueueTaskRunning = true;
            runItem(item, "queue", item->GetRequestId());
            ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
            setQueueButtons();
          } else {
            mQueueTaskRunning = false;
          }
        }
      }
    }

    mDoNotSort = false;
    sortJobs();
  });

  QObject::connect(ui.actionSortTask, &QAction::triggered, this, [=]() {
    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    if (sortTask) {
      ui.actionSortTask->setIcon(sortAZIcon);
      ui.buttonSortTask->setToolTip("Sort Ascending");
      ui.tasksListWidget->setSortingEnabled(true);
      ui.tasksListWidget->sortItems(Qt::DescendingOrder);
      settings->setValue("Settings/sortTask", "false");
    } else {
      ui.actionSortTask->setIcon(sortZAIcon);
      settings->setValue("Settings/sortTask", "true");
      ui.tasksListWidget->setSortingEnabled(true);
      ui.tasksListWidget->sortItems(Qt::AscendingOrder);
      ui.buttonSortTask->setToolTip("Sort Descending");
    }
  });

  QObject::connect(ui.actionEdit, &QAction::triggered, this, [=]() {
    auto items = ui.tasksListWidget->selectedItems();

    if (items.count() != 1) {
      QMessageBox::information(this, tr("Edit task"),
                               tr("Please select one task to edit"));
    }

    else {
      editSelectedTask();
    }
  });

  QObject::connect(ui.actionDryRun, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, sortTask);

    QString itemsToRun;

    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToRun = itemsToRun + jo->description + "\n";
    }

    if (items.count() > 0) {
      int button = QMessageBox::question(
          this, "Dry run",
          QString(
              "Are you sure you want to dry run the following task(s)?\n\n" +
              itemsToRun),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (button == QMessageBox::Yes) {
        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);
          runItem(item, "task", "requestId_placeholder", true);
        }
      }
    }
    ui.tasksListWidget->setFocus();
  });

  QObject::connect(ui.actionStop, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, sortTask);

    QString itemsToStop;

    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToStop = itemsToStop + jo->description + "\n";
    }

    if (items.count() > 0) {
      int button = QMessageBox::question(
          this, "Stop tasks",
          QString("Are you sure you want to stop the following task(s)?\n\n" +
                  itemsToStop),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
      if (button == QMessageBox::Yes) {
        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);

          JobOptions *jo = item->GetData();

          int widgetsCount = ui.jobs->count();
          for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
            QWidget *widget = ui.jobs->itemAt(j)->widget();

            if (auto transfer = qobject_cast<JobWidget *>(widget)) {
              if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
                  (transfer->isRunning)) {
                transfer->cancel();
              }
            }
            if (auto mount = qobject_cast<MountWidget *>(widget)) {
              if ((mount->getUniqueID() == jo->uniqueId.toString()) &&
                  (mount->isRunning)) {
                mount->cancel();
              }
            }
          }
        }
      }
    }
    ui.tasksListWidget->setFocus();
  });

  QObject::connect(ui.actionRun, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, sortTask);

    QString itemsToRun;
    QString itemsAlreadyRunning;

    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToRun = itemsToRun + jo->description + "\n";

      // check if task already running
      int widgetsCount = ui.jobs->count();
      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *widget = ui.jobs->itemAt(j)->widget();

        if (auto transfer = qobject_cast<JobWidget *>(widget)) {
          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->isRunning)) {
            itemsAlreadyRunning = itemsAlreadyRunning + jo->description + "\n";
          }
        }

        if (auto mount = qobject_cast<MountWidget *>(widget)) {
          if ((mount->getUniqueID() == jo->uniqueId.toString()) &&
              (mount->isRunning)) {
            itemsAlreadyRunning = itemsAlreadyRunning + jo->description + "\n";
          }
        }
      }
    }

    if (!itemsAlreadyRunning.isEmpty()) {
      QMessageBox::critical(
          this, "Run",
          QString("Failed as the following task(s) are already running.\n\n" +
                  itemsAlreadyRunning),
          QMessageBox::Ok);
      return;
    }

    if (items.count() > 0) {
      int button = QMessageBox::question(
          this, "Run",
          QString("Are you sure you want to run the following task(s)?\n\n" +
                  itemsToRun),
          QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
      if (button == QMessageBox::Yes) {

        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);
          runItem(item, "task", "requestID_placeholder");
        }
      }
    }
    ui.tasksListWidget->setFocus();
    setTasksButtons();
  });

  QObject::connect(ui.actionDelete, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, sortTask);

    QString itemsToDelete;
    QString itemsAlreadyRunning;

    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToDelete = itemsToDelete + jo->description + "\n";

      // check if any tasks already running
      int widgetsCount = ui.jobs->count();
      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *widget = ui.jobs->itemAt(j)->widget();

        if (auto transfer = qobject_cast<JobWidget *>(widget)) {
          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->isRunning)) {
            itemsAlreadyRunning = itemsAlreadyRunning + jo->description + "\n";
          }
        }
        if (auto mount = qobject_cast<MountWidget *>(widget)) {
          if ((mount->getUniqueID() == jo->uniqueId.toString()) &&
              (mount->isRunning)) {
            itemsAlreadyRunning = itemsAlreadyRunning + jo->description + "\n";
          }
        }
      }
    }

    // don't delete running tasks
    if (!itemsAlreadyRunning.isEmpty()) {
      QMessageBox::critical(
          this, "Run",
          QString("Failed as the following task(s) are running:\n\n" +
                  itemsAlreadyRunning +
                  "\n\nYou can't delete running tasks.\nStop them first."),
          QMessageBox::Ok);
      ui.tasksListWidget->setFocus();
      setTasksButtons();
      return;
    }

    if (items.count() > 0) {
      // all clear - delete
      int button = QMessageBox::warning(
          this, "Delete",
          QString("Are you sure you want to delete the following task(s)?\n\n" +
                  itemsToDelete),
          QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

      if (button == QMessageBox::Yes) {

        QStringList itemsIDsToDelete;
        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);
          JobOptions *jo = item->GetData();
          itemsIDsToDelete << jo->uniqueId.toString();
        }
        for (const auto &i : itemsIDsToDelete) {
          int widgetsCount = ui.tasksListWidget->count();
          for (int j = widgetsCount - 1; j >= 0; j = j - 1) {
            JobOptionsListWidgetItem *item =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.tasksListWidget->item(j));
            JobOptions *jo = item->GetData();
            if (jo->uniqueId.toString() == i) {
              {
                ListOfJobOptions::getInstance()->Forget(jo);
              }
            }
          }
        }

      } else {
        ui.tasksListWidget->setFocus();
      }
    }

    // restore active tasks colours
    int widgetsCount = ui.jobs->count();
    for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));

      JobOptions *joTasks = item->GetData();

      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *widget = ui.jobs->itemAt(j)->widget();

        if (auto transfer = qobject_cast<JobWidget *>(widget)) {
          if ((transfer->getUniqueID() == joTasks->uniqueId.toString()) &&
              (transfer->isRunning)) {
            ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
          }
        }

        if (auto mount = qobject_cast<MountWidget *>(widget)) {
          if ((mount->getUniqueID() == joTasks->uniqueId.toString()) &&
              (mount->isRunning)) {
            ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
          }
        }
      }
    }
  });

  //!!! QObject::connect(ui.actionStartScheduler
  QObject::connect(ui.actionStartScheduler, &QAction::triggered, this, [=]() {
    mDoNotSort = true;
    auto settings = GetSettings();
    settings->setValue("Settings/schedulerStatus", "true");

    int schedulersCount = ui.schedulers->count();
    for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
      QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
      if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {

        scheduler->startScheduler();
      }
    }

    ui.buttonStartScheduler->setEnabled(false);
    ui.buttonStopScheduler->setEnabled(true);

    ui.labelSchedulerInfoStart->show();
    ui.labelSchedulerInfoStop->hide();

    ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                               .arg(mSchedulersCount)
                               .arg(mRunningSchedulersCount));
    mDoNotSort = false;
    sortJobs();
  });

  //!!!  QObject::connect(ui.actionStopScheduler
  QObject::connect(ui.actionStopScheduler, &QAction::triggered, this, [=]() {
    mDoNotSort = true;
    auto settings = GetSettings();
    settings->setValue("Settings/schedulerStatus", "false");

    int schedulersCount = ui.schedulers->count();
    for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
      QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
      if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {

        scheduler->stopScheduler();
      }
    }

    ui.buttonStartScheduler->setEnabled(true);
    ui.buttonStopScheduler->setEnabled(false);

    ui.labelSchedulerInfoStart->hide();
    ui.labelSchedulerInfoStop->show();

    ui.tabs->setTabText(4, QString("Scheduler (%1)").arg(mSchedulersCount));
    mDoNotSort = false;
    sortJobs();
  });

  //!!!  QObject::connect(ui.actionSortByStatus
  QObject::connect(ui.actionSortByStatus, &QAction::triggered, this, [=]() {
    ui.buttonSortByTime->setStyleSheet("QToolButton {border: 0;}");
    ui.buttonSortByStatus->setStyleSheet("QToolButton {}");

    if (mJobsSort != "byDate") {
      mJobsStatusSortOrder = !mJobsStatusSortOrder;
    }

    mJobsSort = "byStatus";

    if (mJobsStatusSortOrder) {
      ui.buttonSortByStatus->setIcon(sortAZIcon);
    } else {
      ui.buttonSortByStatus->setIcon(sortZAIcon);
    }

    sortJobs();
  });

  //!!!  QObject::connect(ui.actionSortByTime
  QObject::connect(ui.actionSortByTime, &QAction::triggered, this, [=]() {
    ui.buttonSortByStatus->setStyleSheet("QToolButton {border: 0;}");
    ui.buttonSortByTime->setStyleSheet("QToolButton {}");

    // flip sort order
    if (mJobsSort != "byStatus") {
      mJobsTimeSortOrder = !mJobsTimeSortOrder;
    }

    mJobsSort = "byDate";

    if (mJobsTimeSortOrder) {
      ui.buttonSortByTime->setIcon(sortTimeAZIcon);
    } else {
      ui.buttonSortByTime->setIcon(sortTimeZAIcon);
    }

    sortJobs();
  });

  //!!!  QObject::connect(ui.actionAddToScheduler
  QObject::connect(ui.actionAddToScheduler, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, !sortTask);

    QString itemsToAdd;

    // create list of selected tasks' names
    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToAdd = itemsToAdd + jo->description + "\n";

      if (jo->operation == JobOptions::Mount) {

        QMessageBox::information(
            this, tr("Mount tasks selected!"),
            tr("You selected mount tasks - they can't be added to the queue."));

        return;
      }
    }

    if (items.count() > 0) {
      int button = QMessageBox::question(
          this, "Add to the scheduler",
          QString("Are you sure you want to add the following "
                  "task(s) to the scheduler?\n\n" +
                  itemsToAdd),
          QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

      if (button == QMessageBox::Yes) {

        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);

          JobOptions *jo = item->GetData();

          jo->uniqueId.toString();
          mSchedulersCount++;
          QStringList args;
          args << "NewScheduler";
          addScheduler(jo->uniqueId.toString(), jo->description, args);

          if ((settings->value("Settings/schedulerStatus").toBool())) {

            ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                       .arg(mSchedulersCount)
                                       .arg(mRunningSchedulersCount));
          } else {
            ui.tabs->setTabText(
                4, QString("Scheduler (%1)").arg(mSchedulersCount));

            int schedulersCount = ui.schedulers->count();
            for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
              QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
              if (auto scheduler =
                      qobject_cast<SchedulerWidget *>(schedulerWidget)) {

                scheduler->stopScheduler();
              }
            }
          }
        }
        listTasks();
        //        saveSchedulerFile();
      } else {
        return;
      }
    }
  });

  //!!! QObject::connect(ui.actionAddToQueue
  QObject::connect(ui.actionAddToQueue, &QAction::triggered, this, [=]() {
    auto selection = ui.tasksListWidget->selectedItems();

    auto settings = GetSettings();
    bool sortTask = settings->value("Settings/sortTask").toBool();

    auto items = sortListWidget(selection, sortTask);

    QString itemsToAdd;

    // create list of selected tasks' names
    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();
      itemsToAdd = itemsToAdd + jo->description + "\n";

      if (jo->operation == JobOptions::Mount) {

        QMessageBox::information(
            this, tr("Mount tasks selected!"),
            tr("You selected mount tasks - they can't be added to the queue."));

        return;
      }
    }

    // if queue is empty we have to try to start first task
    bool isQueueEmpty = (ui.queueListWidget->count() == 0);

    if (items.count() > 0) {
      int button = QMessageBox::question(
          this, "Add to the queue",
          QString("Are you sure you want to add the following "
                  "task(s) to the queue?\n\n" +
                  itemsToAdd),
          QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

      if (button == QMessageBox::Yes) {

        mQueueCount = mQueueCount + items.count();

        foreach (auto i, items) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(i);

          JobOptions *jo = item->GetData();

          QIcon jobIcon;

          if (jo->jobType == JobOptions::JobType::Download) {
            if (jo->operation == JobOptions::Mount) {
              jobIcon = mMountIcon;
            } else {
              jobIcon = mDownloadIcon;
            }
          }
          if (jo->jobType == JobOptions::JobType::Upload) {
            jobIcon = mUploadIcon;
          }

          JobOptionsListWidgetItem *newitem = new JobOptionsListWidgetItem(
              jo, jobIcon, jo->description, QUuid::createUuid().toString());

          ui.queueListWidget->addItem(newitem);
        }

        if (ui.queueListWidget->count() > 0) {
          ui.buttonPurgeQueue->setEnabled(true);
        } else {
          ui.buttonPurgeQueue->setEnabled(false);
        }
        // save new queue to file
        saveQueueFile();
        ui.queueListWidget->setFocus();
      } else {
        // user pressed No
        return;
      }
    }

    // if queue was empty we start first task if queue is running and there is
    // no other transfer job running
    if (mQueueStatus && isQueueEmpty && (mTransferJobCount == 0)) {

      if (mQueueCount > 0) {

        JobOptionsListWidgetItem *item =
            static_cast<JobOptionsListWidgetItem *>(
                ui.queueListWidget->item(0));

        runItem(item, "queue", item->GetRequestId());
        ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
        mQueueTaskRunning = true;
        ui.tabs->setTabText(3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
      }

    } else {

      if (mQueueStatus) {

        if (mQueueTaskRunning) {
          ui.tabs->setTabText(3,
                              QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
        } else {

          ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
        }

      } else {
        ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
      }
    }
  });

  //!!!  QObject::connect(ui.actionStartQueue
  QObject::connect(ui.actionStartQueue, &QAction::triggered, this, [=]() {
    mQueueStatus = true;

    auto settings = GetSettings();
    settings->setValue("Settings/queueStatus", "true");

    ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));

    ui.buttonStopQueue->setEnabled(true);
    ui.buttonStartQueue->setEnabled(false);

    ui.buttonPurgeQueue->setEnabled(false);

    if (mQueueCount > 1) {
      ui.buttonPurgeQueue->setEnabled(true);
    } else {
      ui.buttonPurgeQueue->setEnabled(false);
    }

    if (ui.queueListWidget->currentRow() == 0 ||
        ui.queueListWidget->currentRow() == ui.queueListWidget->count() - 1) {
      ui.buttonDownQueue->setEnabled(false);
    } else {
      ui.buttonDownQueue->setEnabled(true);
    }

    if (ui.queueListWidget->currentRow() == 1 ||
        ui.queueListWidget->currentRow() == 0) {
      ui.buttonUpQueue->setEnabled(false);
    } else {
      ui.buttonUpQueue->setEnabled(true);
    }

    if (ui.queueListWidget->currentRow() == 0) {

      ui.buttonRemoveFromQueue->setEnabled(false);
      ui.buttonUpQueue->setEnabled(false);
      ui.buttonDownQueue->setEnabled(false);

    } else {
      ui.buttonRemoveFromQueue->setEnabled(true);
    }

    if (mQueueCount == 0) {
      ui.buttonRemoveFromQueue->setEnabled(false);
      ui.buttonPurgeQueue->setEnabled(false);
      ui.buttonUpQueue->setEnabled(false);
      ui.buttonDownQueue->setEnabled(false);
    }

    ui.labelQueueInfoStart->setText("Queue is running.");
    ui.labelQueueInfoStart->show();
    ui.labelQueueInfoStop->hide();

    // if not empty and nothing else running try to start first task
    if (mQueueCount > 0 && mTransferJobCount == 0) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.queueListWidget->item(0));

      // start only when not running already
      //      if (!isAlreadyRunning) {
      mQueueTaskRunning = true;
      runItem(item, "queue", item->GetRequestId());
      ui.tabs->setTabText(3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
      ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
      ui.queueListWidget->item(0)->setSelected(false);
      //      }
    }
  });

  QObject::connect(ui.actionStopQueue, &QAction::triggered, this, [=]() {
    mQueueStatus = false;

    auto settings = GetSettings();
    settings->setValue("Settings/queueStatus", "false");

    if (ui.queueListWidget->count() > 0) {
      ui.queueListWidget->item(0)->setBackground(QBrush());
    }

    ui.buttonStopQueue->setEnabled(false);
    ui.buttonStartQueue->setEnabled(true);
    ui.buttonPurgeQueue->setEnabled(true);
    ui.buttonRemoveFromQueue->setEnabled(true);

    if (mQueueCount == 0) {
      ui.buttonRemoveFromQueue->setEnabled(false);
      ui.buttonPurgeQueue->setEnabled(false);
      ui.buttonUpQueue->setEnabled(false);
      ui.buttonDownQueue->setEnabled(false);
      ui.tabs->setTabText(3, QString("Queue"));
    } else {
      ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
    }

    if (!(ui.queueListWidget->item(0) == nullptr)) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.queueListWidget->item(0));

      JobOptions *jo = item->GetData();

      int widgetsCount = ui.jobs->count();
      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *widget = ui.jobs->itemAt(j)->widget();
        if (auto transfer = qobject_cast<JobWidget *>(widget)) {

          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->isRunning)) {

            if (transfer->getRequestId() == item->GetRequestId()) {

              emit transfer->cancel();
            }
          }
        }
      }
    }

    ui.labelQueueInfoStop->setText("Queue is not running.");
    ui.labelQueueInfoStop->show();
    ui.labelQueueInfoStart->hide();
  });

  //!!! QObject::connect(ui.actionPurgeQueue
  QObject::connect(ui.actionPurgeQueue, &QAction::triggered, this, [=]() {
    if (ui.queueListWidget->count() > 0) {

      int button = QMessageBox::warning(
          this, "Clean the queue",
          QString("Are you sure you want to remove all\nnot running tasks from "
                  "the queue?"),
          QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

      if (button == QMessageBox::Yes) {

        int itemsCount = ui.queueListWidget->count();
        for (int i = 0; i < itemsCount; i++) {

          if (mQueueStatus && mQueueTaskRunning) {
            if (i != 0) {
              --mQueueCount;

              JobOptionsListWidgetItem *item_queue =
                  static_cast<JobOptionsListWidgetItem *>(
                      ui.queueListWidget->item(1));

              QString requestId = item_queue->GetRequestId();
              // notify schedulers
              int schedulersCount = ui.schedulers->count();
              for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
                QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
                if (auto scheduler =
                        qobject_cast<SchedulerWidget *>(schedulerWidget)) {
                  scheduler->updateTaskStatus(requestId,
                                              "removed from the queue");

                  if (scheduler->getSchedulerRequestId() ==
                      item_queue->GetRequestId()) {

                    mRunningSchedulersCount--;
                    ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                               .arg(mSchedulersCount)
                                               .arg(mRunningSchedulersCount));
                  }
                }
              }

              ui.queueListWidget->takeItem(1);
            }
          } else {
            --mQueueCount;

            JobOptionsListWidgetItem *item_queue =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.queueListWidget->item(0));

            QString requestId = item_queue->GetRequestId();
            // notify schedulers
            int schedulersCount = ui.schedulers->count();
            for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
              QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
              if (auto scheduler =
                      qobject_cast<SchedulerWidget *>(schedulerWidget)) {
                scheduler->updateTaskStatus(requestId,
                                            "removed from the queue");

                if (scheduler->getSchedulerRequestId() ==
                    item_queue->GetRequestId()) {
                  mRunningSchedulersCount--;
                  ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                             .arg(mSchedulersCount)
                                             .arg(mRunningSchedulersCount));
                }
              }
            }

            ui.queueListWidget->takeItem(0);
          }

          ui.buttonRemoveFromQueue->setEnabled(false);
          ui.buttonPurgeQueue->setEnabled(false);
          ui.buttonUpQueue->setEnabled(false);
          ui.buttonDownQueue->setEnabled(false);
        }
      } else {
        ui.queueListWidget->setFocus();
      }
    }

    if (mQueueStatus) {

      if (mQueueCount == 0) {
        ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
      } else {

        if (!mQueueTaskRunning) {
          ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
        } else {
          ui.tabs->setTabText(3,
                              QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
        }
      }

    } else {
      if (mQueueCount != 0) {
        ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
      } else {
        ui.tabs->setTabText(3, QString("Queue"));
      }
    }

    saveQueueFile();
  });

  QObject::connect(ui.actionRemoveFromQueue, &QAction::triggered, this, [=]() {
    if (mQueueStatus) {

      if (mQueueCount > 0) {

        JobOptionsListWidgetItem *item_queue =
            static_cast<JobOptionsListWidgetItem *>(
                ui.queueListWidget->item(ui.queueListWidget->currentRow()));

        QString requestId = item_queue->GetRequestId();

        // notify schedulers
        int schedulersCount = ui.schedulers->count();
        for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
          QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
          if (auto scheduler =
                  qobject_cast<SchedulerWidget *>(schedulerWidget)) {
            scheduler->updateTaskStatus(requestId, "removed from the queue");

            if (scheduler->getSchedulerRequestId() == requestId) {
              mRunningSchedulersCount--;
              ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                         .arg(mSchedulersCount)
                                         .arg(mRunningSchedulersCount));
            }
          }
        }

        --mQueueCount;
        ui.queueListWidget->takeItem(ui.queueListWidget->currentRow());

        if (ui.queueListWidget->currentRow() == 0 ||
            ui.queueListWidget->currentRow() ==
                ui.queueListWidget->count() - 1) {
          ui.buttonDownQueue->setEnabled(false);
        } else {
          ui.buttonDownQueue->setEnabled(true);
        }

        if (ui.queueListWidget->currentRow() == 0 ||
            ui.queueListWidget->currentRow() == 1) {
          ui.buttonUpQueue->setEnabled(false);
        } else {
          ui.buttonUpQueue->setEnabled(true);
        }
      }
      if (ui.queueListWidget->count() == 1) {
        ui.buttonRemoveFromQueue->setEnabled(false);
      }
      if (mQueueCount == 0) {
        ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
      } else {

        if (!mQueueTaskRunning) {
          ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
        } else {
          ui.tabs->setTabText(3,
                              QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
        }
      }

    } else {

      --mQueueCount;

      JobOptionsListWidgetItem *item_queue =
          static_cast<JobOptionsListWidgetItem *>(
              ui.queueListWidget->item(ui.queueListWidget->currentRow()));

      QString requestId = item_queue->GetRequestId();

      // notify schedulers
      int schedulersCount = ui.schedulers->count();
      for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
        QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
        if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {
          scheduler->updateTaskStatus(requestId, "removed from the queue");

          if (scheduler->getSchedulerRequestId() == requestId) {
            mRunningSchedulersCount--;
            ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                       .arg(mSchedulersCount)
                                       .arg(mRunningSchedulersCount));
          }
        }
      }

      ui.queueListWidget->takeItem(ui.queueListWidget->currentRow());

      if (ui.queueListWidget->currentRow() == ui.queueListWidget->count() - 1) {
        ui.buttonDownQueue->setEnabled(false);
      } else {
        ui.buttonDownQueue->setEnabled(true);
      }
      if (ui.queueListWidget->currentRow() == 0) {
        ui.buttonUpQueue->setEnabled(false);
      } else {
        ui.buttonUpQueue->setEnabled(true);
      }

      if (ui.queueListWidget->count() == 0) {

        ui.buttonRemoveFromQueue->setEnabled(false);
        ui.buttonPurgeQueue->setEnabled(false);
        ui.buttonUpQueue->setEnabled(false);
        ui.buttonDownQueue->setEnabled(false);
        ui.tabs->setTabText(3, QString("Queue"));

      } else {
        ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
      }
    }

    saveQueueFile();
  });

  QObject::connect(ui.actionDownQueue, &QAction::triggered, this, [=]() {
    auto item = ui.queueListWidget->takeItem(ui.queueListWidget->currentRow());
    ui.queueListWidget->insertItem(ui.queueListWidget->currentRow() + 1, item);
    ui.queueListWidget->setCurrentRow(ui.queueListWidget->row(item));

    saveQueueFile();
  });

  QObject::connect(ui.actionUpQueue, &QAction::triggered, this, [=]() {
    if (ui.queueListWidget->currentRow() == ui.queueListWidget->count() - 1) {
      // last row needs special treatment
      auto item =
          ui.queueListWidget->takeItem(ui.queueListWidget->currentRow());
      ui.queueListWidget->insertItem(ui.queueListWidget->currentRow(), item);
      ui.queueListWidget->setCurrentRow(ui.queueListWidget->row(item));
    } else {
      auto item =
          ui.queueListWidget->takeItem(ui.queueListWidget->currentRow());
      ui.queueListWidget->insertItem(ui.queueListWidget->currentRow() - 1,
                                     item);
      ui.queueListWidget->setCurrentRow(ui.queueListWidget->row(item));
    }

    saveQueueFile();
  });

  QObject::connect(ui.queueListWidget, &QListWidget::itemChanged, this,
                   [=]() { setQueueButtons(); });

  QObject::connect(ui.queueListWidget, &QListWidget::itemClicked, this,
                   [=]() { setQueueButtons(); });

  QObject::connect(ui.queueListWidget, &QListWidget::currentItemChanged, this,
                   [=]() { setQueueButtons(); });

  QObject::connect(ui.queueListWidget, &QListWidget::itemSelectionChanged, this,
                   [=]() { setQueueButtons(); });

  QObject::connect(ListOfJobOptions::getInstance(),
                   &ListOfJobOptions::tasksListUpdated, this,
                   &MainWindow::listTasks);

  mUploadIcon = arrowUpIcon;
  mDownloadIcon = arrowDownIcon;
  mMountIcon = mount1Icon;

  // remove close button from these tabs
  ui.tabs->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(0, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(1, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(1, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(2, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(2, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(3, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(3, QTabBar::LeftSide, nullptr);
  ui.tabs->tabBar()->setTabButton(4, QTabBar::RightSide, nullptr);
  ui.tabs->tabBar()->setTabButton(4, QTabBar::LeftSide, nullptr);

  ui.tabs->setCurrentIndex(0);

  {
    restoreSchedulersFromFile();
    listTasks();
    addTasksToQueue();
  }

  if (!(settings->value("Settings/schedulerStatus").toBool())) {

    int schedulersCount = ui.schedulers->count();
    for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
      QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
      if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {

        scheduler->stopScheduler();
      }
    }

    ui.buttonStartScheduler->setEnabled(true);
    ui.buttonStopScheduler->setEnabled(false);

    ui.labelSchedulerInfoStart->hide();
    ui.labelSchedulerInfoStop->show();

    ui.tabs->setTabText(4, QString("Scheduler (%1)").arg(mSchedulersCount));
  }

  if ((settings->value("Settings/queueStatus").toBool())) {
    ui.actionStartQueue->trigger();
  }

  QObject::connect(&mSystemTray, &QSystemTrayIcon::activated, this,
                   [=](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::DoubleClick ||
                         reason == QSystemTrayIcon::Trigger) {
                       showNormal();
                       mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
                       osxShowDockIcon();
#endif
                     }
                   });

  QObject::connect(&mSystemTray, &QSystemTrayIcon::messageClicked, this, [=]() {
    showNormal();
    mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
    osxShowDockIcon();
#endif

    ui.tabs->setCurrentIndex(1);
    if (mLastFinished) {
      mLastFinished->showDetails();
      ui.jobsArea->ensureWidgetVisible(mLastFinished);
    }
  });

  QMenu *trayMenu = new QMenu(this);
  QObject::connect(
      trayMenu->addAction("&Show"), &QAction::triggered, this, [=]() {
        MainWindow::setWindowState((windowState() & ~Qt::WindowMinimized) |
                                   Qt::WindowActive);
        MainWindow::show();  // bring window to top on macOS
        MainWindow::raise(); // bring window from minimized state on macOS
        MainWindow::activateWindow(); // bring window to front/unminimize on
                                      // windows
        mSystemTray.setVisible(mAlwaysShowInTray);
#ifdef Q_OS_MACOS
        osxShowDockIcon();
#endif
      });

  QObject::connect(trayMenu->addAction("&Quit"), &QAction::triggered, this,
                   [=]() {
                     if (canClose()) {
                       saveQueueFile();
                       saveSchedulerFile();
                       QApplication::quit();
                     }
                   });

  mSystemTray.setContextMenu(trayMenu);

  mStatusMessage = new QLabel();
  ui.statusBar->addWidget(mStatusMessage);
  ui.statusBar->setStyleSheet("QStatusBar::item { border: 0; }");

  QTimer::singleShot(0, ui.remotes, SLOT(setFocus()));

  QString rclone = GetRclone();
  if (rclone.isEmpty()) {
    rclone = QStandardPaths::findExecutable("rclone");
    if (rclone.isEmpty()) {
      QMessageBox::information(
          this, "Error",
          "Cannot check rclone version!\nPlease verify rclone location.");
      emit ui.preferences->trigger();
    } else {
      auto settings = GetSettings();
      settings->setValue("Settings/rclone", rclone);
      SetRclone(rclone);
      rcloneGetVersion();
    }
  } else {
    rcloneGetVersion();
  }

  // we start all auto mount tasks with 1s delay - so RB has chance to start
  // properly
  QTimer::singleShot(1000, Qt::CoarseTimer, this, SLOT(autoStartMounts()));

  // start minimised to tray
  if ((settings->value("Settings/startMinimisedToTray").toBool())) {
#ifdef Q_OS_MACOS
    osxHideDockIcon();
#endif
    mSystemTray.show();
    QTimer::singleShot(0, this, SLOT(hide()));
  }
}

MainWindow::~MainWindow() {
  auto settings = GetSettings();
  settings->setValue("MainWindow/geometry", saveGeometry());
}

QList<QListWidgetItem *>
MainWindow::sortListWidget(const QList<QListWidgetItem *> &list,
                           bool sortOrder) {

  QList<QListWidgetItem *> sortedList = list;

  int n = sortedList.size();
  int min_idx;

  // basic selection sort algorithm
  for (int i = 0; i < (n - 1); i++) {
    min_idx = i;
    for (int j = i + 1; j < n; j++) {

      JobOptionsListWidgetItem *item_j =
          static_cast<JobOptionsListWidgetItem *>(sortedList.at(j));
      JobOptions *jo_j = item_j->GetData();

      JobOptionsListWidgetItem *item_min_idx =
          static_cast<JobOptionsListWidgetItem *>(sortedList.at(min_idx));
      JobOptions *jo_min_idx = item_min_idx->GetData();

      if (sortOrder) {
        if ((jo_j->description).toUpper() <
            (jo_min_idx->description).toUpper()) {
          min_idx = j;
        }
      } else {
        if ((jo_j->description).toUpper() >
            (jo_min_idx->description).toUpper()) {
          min_idx = j;
        }
      }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
    sortedList.swapItemsAt(min_idx, i);
#else
    sortedList.swap(min_idx, i);
#endif
  }
  return sortedList;
}

void MainWindow::autoStartMounts(void) {

  // autostart all auto mounts from tasks list
  for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
    JobOptions *joTasks = item->GetData();
    if (joTasks->operation == JobOptions::Mount && joTasks->mountAutoStart) {
      runItem(item, "autostart", "requestID_placeholder");
    }
  }
}

void MainWindow::quitApp(void) {
  // wait for all processes to stop
  if (mQuitInfoDelay == 3) {

    QMessageBox *msgBox = new QMessageBox(
        QMessageBox::Warning, "Quitting",
        "Terminating all processes\nbefore quitting, please wait.",
        QMessageBox::NoButton, this);
    msgBox->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint |
                           Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    msgBox->setStandardButtons(QMessageBox::NoButton);
    msgBox->setCursor(Qt::WaitCursor);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);

    msgBox->show();

    mQuittingErrorMsgBox = msgBox;
  }

  bool processActive = false;
  bool unmountingFailed = false;
  int widgetsCount = ui.jobs->count();

  // loop over all jobs and clean them
  for (int i = widgetsCount - 2; i >= 0; i = i - 2) {
    QWidget *widget = ui.jobs->itemAt(i)->widget();
    if (auto mount = qobject_cast<MountWidget *>(widget)) {
      if (mount->isRunning) {
        processActive = true;
        if (mount->getUnmountingError() != "0") {
          // there is failed unmount - quitting fails
          // but loop continues closing what possible
          unmountingFailed = true;
        }
      } else {
        emit mount->closed();
      }
    } else if (auto transfer = qobject_cast<JobWidget *>(widget)) {
      if (transfer->isRunning) {
        processActive = true;
      } else {
        emit transfer->closed();
      }
    } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
      if (stream->isRunning) {
        processActive = true;
      } else {
        emit stream->closed();
      }
    }
  };

  if (unmountingFailed) {
    // quitting failed
    // reset to 3 attempts again
    mQuitInfoDelay = 3;

    if (mQuittingErrorMsgBox != NULL) {
      mQuittingErrorMsgBox->hide();
      mQuittingErrorMsgBox = NULL;
    }

    QMessageBox::critical(
        this, "Unmounting failed",
        QString("Some mounts can't be unmounted. Make sure that they are "
                "not used by other programs. You can also try to unmount "
                "them directly from your OS."));

    mAppQuittingStatus = false;
    return;
  }

  if (processActive == false) {
    // no running widget - bye bye - quitting at last
    saveQueueFile();
    saveSchedulerFile();
    QApplication::quit();
  } else {
    // something still running we check again a bit later then
    QTimer::singleShot(200, Qt::CoarseTimer, this, SLOT(quitApp()));
    ++mQuitInfoDelay;
  }
}

void MainWindow::setTasksButtons() {
  auto items = ui.tasksListWidget->selectedItems();
  if (items.isEmpty()) {
    ui.buttonDeleteTask->setEnabled(false);
    ui.buttonEditTask->setEnabled(false);
    ui.buttonRunTask->setEnabled(false);
    ui.buttonDryrunTask->setEnabled(false);
    ui.buttonAddToQueue->setEnabled(false);
    ui.buttonAddToScheduler->setEnabled(false);
  } else {

    ui.buttonDeleteTask->setEnabled(true);
    ui.buttonEditTask->setEnabled(true);
    ui.buttonRunTask->setEnabled(true);
    ui.buttonDryrunTask->setEnabled(true);
    ui.buttonAddToQueue->setEnabled(true);
    ui.buttonAddToScheduler->setEnabled(true);
  }

  if (ui.tasksListWidget->count() > 1) {
    ui.buttonSortTask->setEnabled(true);
  } else {
    ui.buttonSortTask->setEnabled(false);
  }

  if (items.count() > 1) {
    ui.buttonEditTask->setEnabled(false);
  }

  bool isMount = false;
  bool isRunning = false;
  int isNotRunning = items.count();
  bool isScheduled = false;
  int schedulersCount = ui.schedulers->count();

  if (items.count() > 0) {

    foreach (auto i, items) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(i);
      JobOptions *jo = item->GetData();

      if (!isScheduled) {
        for (int i = schedulersCount - 2; i >= 0; i = i - 2) {
          QWidget *widget = ui.schedulers->itemAt(i)->widget();
          if (auto scheduler = qobject_cast<SchedulerWidget *>(widget)) {

            if (scheduler->getSchedulerTaskId() == jo->uniqueId.toString()) {

              isScheduled = true;
              break;
            }
          }
        }
      }

      if (jo->operation == JobOptions::Mount) {
        isMount = true;
      }

      int widgetsCount = ui.jobs->count();
      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *widget = ui.jobs->itemAt(j)->widget();

        if (auto transfer = qobject_cast<JobWidget *>(widget)) {
          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->isRunning)) {
            isRunning = true;
            --isNotRunning;
          } else {

            if ((transfer->getUniqueID() == jo->uniqueId.toString())) {
            }
          }
        }

        if (auto mount = qobject_cast<MountWidget *>(widget)) {
          if ((mount->getUniqueID() == jo->uniqueId.toString()) &&
              (mount->isRunning)) {
            isRunning = true;
            --isNotRunning;
          } else {

            if ((mount->getUniqueID() == jo->uniqueId.toString())) {
            }
          }
        }
      }
    }
  }

  if (isRunning) {
    ui.buttonDeleteTask->setEnabled(false);
    ui.buttonRunTask->setEnabled(false);
    ui.buttonDryrunTask->setEnabled(false);
  }

  if (isNotRunning == 0 && items.count() != 0) {
    ui.buttonStop->setEnabled(true);
  } else {
    ui.buttonStop->setEnabled(false);
  }

  if (isMount) {
    ui.buttonDryrunTask->setEnabled(false);
    ui.buttonAddToQueue->setEnabled(false);
    ui.buttonAddToScheduler->setEnabled(false);
  }

  if (isScheduled) {
    ui.buttonDeleteTask->setEnabled(false);
  }

  return;
}

void MainWindow::setQueueButtons() {

  if (ui.queueListWidget->selectedItems().empty()) {

    ui.buttonDownQueue->setEnabled(false);
    ui.buttonUpQueue->setEnabled(false);
    ui.buttonRemoveFromQueue->setEnabled(false);
    ui.actionDownQueue->setEnabled(false);
    ui.actionUpQueue->setEnabled(false);
    ui.actionRemoveFromQueue->setEnabled(false);

    if (mQueueStatus) {

      if (ui.queueListWidget->count() > 1) {
        ui.buttonPurgeQueue->setEnabled(true);
      } else {
        ui.buttonPurgeQueue->setEnabled(false);
      }

      if (ui.queueListWidget->count() > 0) {
        ui.queueListWidget->item(0)->setSelected(false);
      }

    } else {

      if (ui.queueListWidget->count() > 0) {
        ui.buttonPurgeQueue->setEnabled(true);
      } else {
        ui.buttonPurgeQueue->setEnabled(false);
      }
    }
    return;
  }

  if (mQueueStatus) {

    if (ui.queueListWidget->count() > 1) {
      ui.buttonPurgeQueue->setEnabled(true);
    } else {
      ui.buttonPurgeQueue->setEnabled(false);
    }

    if (ui.queueListWidget->currentRow() == 0 ||
        ui.queueListWidget->currentRow() == ui.queueListWidget->count() - 1) {
      ui.buttonDownQueue->setEnabled(false);
      ui.actionDownQueue->setEnabled(false);
    } else {
      ui.buttonDownQueue->setEnabled(true);
      ui.actionDownQueue->setEnabled(true);
    }

    if (ui.queueListWidget->currentRow() == 1 ||
        ui.queueListWidget->currentRow() == 0) {
      ui.buttonUpQueue->setEnabled(false);
      ui.actionUpQueue->setEnabled(false);
    } else {
      ui.buttonUpQueue->setEnabled(true);
      ui.actionUpQueue->setEnabled(true);
    }

    if (ui.queueListWidget->currentRow() == 0) {
      ui.buttonRemoveFromQueue->setEnabled(false);
      ui.actionRemoveFromQueue->setEnabled(false);
    } else {
      ui.buttonRemoveFromQueue->setEnabled(true);
      ui.actionRemoveFromQueue->setEnabled(true);
    }

    if (ui.queueListWidget->count() > 0) {
      ui.queueListWidget->item(0)->setSelected(false);
    }

  } else {

    if (ui.queueListWidget->count() > 0) {
      ui.buttonPurgeQueue->setEnabled(true);
    } else {
      ui.buttonPurgeQueue->setEnabled(false);
    }

    if (ui.queueListWidget->currentRow() == 0) {
      ui.buttonUpQueue->setEnabled(false);
      ui.actionUpQueue->setEnabled(false);
    } else {
      ui.buttonUpQueue->setEnabled(true);
      ui.actionUpQueue->setEnabled(true);
    }

    if (ui.queueListWidget->currentRow() == ui.queueListWidget->count() - 1) {
      ui.buttonDownQueue->setEnabled(false);
      ui.actionDownQueue->setEnabled(false);
    } else {
      ui.buttonDownQueue->setEnabled(true);
      ui.actionDownQueue->setEnabled(true);
    }

    if (ui.queueListWidget->count() > 0) {
      ui.buttonPurgeQueue->setEnabled(true);
    } else {
      ui.buttonPurgeQueue->setEnabled(false);
    }

    ui.buttonRemoveFromQueue->setEnabled(true);
    ui.actionRemoveFromQueue->setEnabled(true);
  }
}

void MainWindow::rcloneGetVersion() {
  bool firstTime = mFirstTime;
  mFirstTime = false;

  QProcess *p = new QProcess();

  QObject::connect(
      p,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int code, QProcess::ExitStatus) {
        if (code == 0) {
          QString version = p->readAllStandardOutput().trimmed();

          // extract rclone version - numbers only
          QString rclone_info1 = version;
          QString rclone_version_no;
          int lineBreak = rclone_info1.indexOf('\n');
          if (lineBreak != -1) {
            rclone_info1.remove(lineBreak, rclone_info1.length() - lineBreak);
            rclone_version_no = rclone_info1;
            rclone_version_no.replace("rclone v", "");
            rclone_version_no.replace("-DEV", "");
          } else {
            // for very old rclone versions format was one line only
            rclone_version_no = rclone_info1.trimmed();
            rclone_version_no.replace("rclone v", "");
            rclone_version_no.replace("-DEV", "");
          }
          // save current version no in settings
          auto settings = GetSettings();
          settings->setValue("Settings/rcloneVersion", rclone_version_no);

#if defined(Q_OS_WIN32)
          // check if required version
          unsigned int result =
              compareVersion(rclone_version_no.toStdString(), "1.50");

          if (result == 2) {
            QMessageBox::warning(
                this, "",
                "For mount functionality to work you need "
                "rclone version at least v1.50 "
                "and your current version is v" +
                    rclone_version_no +
                    ". Mount will be disabled. \n\nPlease consider upgrading.");
          };
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
          QStringList lines = version.split("\n", Qt::SkipEmptyParts);
#else
          QStringList lines = version.split("\n", QString::SkipEmptyParts);
#endif

          QString rclone_info2;
          QString rclone_info3;

          int counter = 0;
          foreach (QString line, lines) {
            line = line.trimmed();
            if (counter == 1)
              rclone_info2 = line.replace("- ", "");
            if (counter == 2)
              rclone_info3 = line.replace("- ", "");
            counter++;
          };

          QFileInfo appBundlePath;
#ifdef Q_OS_MACOS
          if (IsPortableMode()) {

            QFileInfo applicationPath = qApp->applicationFilePath();
            QFileInfo MacOSPath = applicationPath.dir().path();
            QFileInfo ContentsPath = MacOSPath.dir().path();
            appBundlePath = ContentsPath.dir().path();

            mStatusMessage->setText(rclone_info1 + ", " + rclone_info2 + ", " +
                                    rclone_info3);

            mStatusMessage->setToolTip(
                rclone_info1 + " in " +
                QDir::toNativeSeparators(GetRclone().replace(
                    appBundlePath.fileName() + "/Contents/MacOS/../../../",
                    "")) +
                ", " + rclone_info2 + ", " + rclone_info3);

          } else {

            mStatusMessage->setText(rclone_info1 + ", " + rclone_info2 + ", " +
                                    rclone_info3);

            mStatusMessage->setToolTip(
                rclone_info1 + " in " + QDir::toNativeSeparators(GetRclone()) +
                ", " + rclone_info2 + ", " + rclone_info3);
          }
#else
#ifdef Q_OS_WIN
          mStatusMessage->setText(rclone_info1 + ", " +
                                  rclone_info2 + ", " + rclone_info3);


          mStatusMessage->setToolTip(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone()) + ", " +
                                  rclone_info2 + ", " + rclone_info3);

#else
          if (IsPortableMode()) {
            QString xdg_config_home = qgetenv("XDG_CONFIG_HOME");
            QString appImageConfigFolder = xdg_config_home.right(xdg_config_home.length()-xdg_config_home.lastIndexOf("/"));

            mStatusMessage->setText(rclone_info1 + ", " +
                                  rclone_info2 + ", " + rclone_info3);

            mStatusMessage->setToolTip(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone().replace(appImageConfigFolder + "/..",  "")) + ", " +
                                  rclone_info2 + ", " + rclone_info3);


          } else {
            mStatusMessage->setText(rclone_info1 + ", " +
                                  rclone_info2 + ", " + rclone_info3);

            mStatusMessage->setToolTip(rclone_info1 + " in " +
                                  QDir::toNativeSeparators(GetRclone()) + ", " +
                                  rclone_info2 + ", " + rclone_info3);


         }
#endif
#endif

          rcloneListRemotes();
        } else {
          if (p->error() != QProcess::FailedToStart) {
            if (getConfigPassword(p)) {
              rcloneGetVersion();
            } else {
              close();
            }
            p->deleteLater();
            return;
          }

          if (firstTime) {
            if (p->error() == QProcess::FailedToStart) {
              QMessageBox::information(
                  this, "Error",
                  "Wrong rclone executable or rclone not found!\nPlease select "
                  "its location in next dialog.");
            } else {
              QMessageBox::information(this, "Error",
                                       "Cannot check rclone version!\nPlease "
                                       "verify rclone location.");
            }
            emit ui.preferences->trigger();
          }
        }

        auto settings = GetSettings();

        /// check rclone version

        // get already stored rclone version no
        QString rclone_version_no =
            settings->value("Settings/rcloneVersion").toString();

        // during first run the key might not exist yet
        if (!(settings->contains("Settings/checkRcloneUpdates"))) {
          // if checkRcloneUpdates does not exist create new key
          settings->setValue("Settings/checkRcloneUpdates", true);
        };

        bool checkRcloneUpdates =
            settings->value("Settings/checkRcloneUpdates").toBool();

        // if check updates enabled in settings
        if (checkRcloneUpdates) {
          QString last_check;
          QString current_date = QDate::currentDate().toString();

          if (!(settings->contains("Settings/lastRcloneUpdateCheck"))) {
            // if lastRcloneUpdateCheck does not exist create new key
            settings->setValue("Settings/lastRcloneUpdateCheck", current_date);
          } else { // read last check date
            last_check =
                settings->value("Settings/lastRcloneUpdateCheck").toString();
          };

          // dont check if already checked today (once per day only)
          if (!(last_check == current_date)) {
            // remmber when last checked
            settings->setValue("Settings/lastRcloneUpdateCheck", current_date);

            QString url =
                "https://api.github.com/repos/rclone/rclone/releases/latest";
            QNetworkAccessManager manager;
            QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
            QEventLoop event;
            connect(response, SIGNAL(finished()), &event, SLOT(quit()));
            event.exec();
            QByteArray content = response->readAll();
            QJsonParseError jsonError;

            QJsonDocument document = QJsonDocument::fromJson(
                content, &jsonError); // parse and capture the error flag

            if (jsonError.error == QJsonParseError::NoError) {

              if (document.object().contains("tag_name")) {

                QJsonValue tag_name = document.object().value("tag_name");

                QString rclone_latest_version_no = tag_name.toString(QString());

                rclone_latest_version_no.replace("v", "");
                rclone_latest_version_no.replace("-DEV", "");
                rclone_latest_version_no = rclone_latest_version_no.trimmed();

                // check if new version available and if yes display information
                unsigned int result =
                    compareVersion(rclone_latest_version_no.toStdString(),
                                   rclone_version_no.toStdString());
                // latest version is greater than current
                if (result == 1) {

                  QMessageBox::information(
                      this, "",
                      QString(
                          R"(<p>New rclone version is available</p>)"
                          R"(<p>You have: v)" +
                          rclone_version_no +
                          "<br />"
                          R"(New version: v)" +
                          rclone_latest_version_no +
                          "</p>"
                          R"(<p>Visit rclone <a href="https://rclone.org/downloads/">downloads</a> page to upgrade</p>)"));
                };
              };
            };
          };
        };

        /// check rclone browser version

        // during first run the key might not exist yet
        if (!(settings->contains("Settings/checkRcloneBrowserUpdates"))) {
          // if checkRcloneBrowserUpdates does not exist create new key
          settings->setValue("Settings/checkRcloneBrowserUpdates", true);
        };

        bool checkRcloneBrowserUpdates =
            settings->value("Settings/checkRcloneBrowserUpdates").toBool();

        // if check updates enabled in settings
        if (checkRcloneBrowserUpdates) {
          QString last_check;
          QString current_date = QDate::currentDate().toString();

          if (!(settings->contains("Settings/lastRcloneBrowserUpdateCheck"))) {
            // if lastRcloneBrowserUpdateCheck does not exist create new key
            settings->setValue("Settings/lastRcloneBrowserUpdateCheck",
                               current_date);
          } else { // read last check date
            last_check =
                settings->value("Settings/lastRcloneBrowserUpdateCheck")
                    .toString();
          };

          // dont check if already checked today (once per day only)
          if (!(last_check == current_date)) {
            // remmber when last checked
            settings->setValue("Settings/lastRcloneBrowserUpdateCheck",
                               current_date);

            // get latest version available
            QString url = "https://api.github.com/repos/kapitainsky/"
                          "rclonebrowser/releases/latest";
            QNetworkAccessManager manager;
            QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
            QEventLoop event;
            connect(response, SIGNAL(finished()), &event, SLOT(quit()));
            event.exec();
            QByteArray content = response->readAll();

            QJsonParseError jsonError;
            QJsonDocument document = QJsonDocument::fromJson(
                content, &jsonError); // parse and capture the error flag

            if (jsonError.error == QJsonParseError::NoError) {
              if (document.object().contains("tag_name")) {
                QJsonValue tag_name = document.object().value("tag_name");
                QString rclone_browser_latest_version_no =
                    tag_name.toString(QString());
                rclone_browser_latest_version_no =
                    rclone_browser_latest_version_no.trimmed();

                // check if new version available and if yes display information
                unsigned int result = compareVersion(
                    rclone_browser_latest_version_no.toStdString(),
                    RCLONE_BROWSER_VERSION);
                // latest version is greater than current
                if (result == 1) {
                  QMessageBox::information(
                      this, "",
                      QString(
                          R"(<p>New Rclone Browser version is available</p>)"
                          R"(<p>You have: v)" RCLONE_BROWSER_VERSION "<br />"
                          R"(New version: v)" +
                          rclone_browser_latest_version_no +
                          "</p>"
                          R"(<p>Visit <a href="https://github.com/kapitainsky/RcloneBrowser/releases/latest">releases</a> page to download</p>)"));
                };
              };
            };
          };
        };

        p->deleteLater();
      });

  QObject::connect(
      p, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        QString errorString =
            QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error);

        QMessageBox::information(
            this, "Error",
            "Cannot start rclone\n\n Error: " + errorString +
                "\n\nPlease verify rclone excecutable location.");
        emit ui.preferences->trigger();
      });

  UseRclonePassword(p);
  p->start(GetRclone(),
           QStringList() << "version"
                         << "--ask-password=false",
           QIODevice::ReadOnly);
}

void MainWindow::rcloneConfig() {

  // for macOS and Linux we have to take care of possible spaces in rclone and
  // rclone.conf paths by using "" around them
  QString terminalRcloneCmd;
  if (!GetRcloneConf().isEmpty()) {
    terminalRcloneCmd = "\"" + GetRclone() + "\"" + " config" + " --config " +
                        "\"" + GetRcloneConf().at(1) + "\"";
  } else {
    terminalRcloneCmd = "\"" + GetRclone() + "\"" + " config";
  }

#if defined(Q_OS_WIN32) && (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
  QProcess::startDetached(GetRclone(), QStringList()
                                           << "config" << GetRcloneConf());
  return;
#else

  QProcess *p = new QProcess(this);

  QObject::connect(p,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int code, QProcess::ExitStatus) {
                     if (code == 0) {
                       emit rcloneListRemotes();
                     }
                     p->deleteLater();
                   });
#endif

#if defined(Q_OS_WIN32)
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  p->setCreateProcessArgumentsModifier(
      [](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_NEW_CONSOLE;
        args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
      });
  p->setProgram(GetRclone());
  p->setArguments(QStringList() << "config" << GetRcloneConf());
#endif

#elif defined(Q_OS_MACOS)
  auto tmp = new QFile("/tmp/rclone_config.command");
  tmp->open(QIODevice::WriteOnly);
  QTextStream(tmp) << "#!/bin/sh\n" << terminalRcloneCmd << "\n";
  tmp->close();
  tmp->setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser |
                      QFileDevice::ExeUser | QFileDevice::ReadGroup |
                      QFileDevice::ExeGroup | QFileDevice::ReadOther |
                      QFileDevice::ExeOther);
  p->setProgram("open");
  p->setArguments(QStringList() << tmp->fileName());
#else
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString terminal = env.value("TERMINAL");
  if (terminal.isEmpty()) {
    terminal = QStandardPaths::findExecutable("gnome-terminal");
    if (terminal.isEmpty()) {
      terminal = QStandardPaths::findExecutable("xfce4-terminal");
      if (terminal.isEmpty()) {
        terminal = QStandardPaths::findExecutable("xterm");
        if (terminal.isEmpty()) {
          terminal = QStandardPaths::findExecutable("x-terminal-emulator");
          if (terminal.isEmpty()) {
            terminal = QStandardPaths::findExecutable("konsole");
            if (terminal.isEmpty()) {
              QMessageBox::critical(this, "Error",
                                    "Not sure how to launch terminal!\n"
                                    "Please set path to terminal executable in "
                                    "$TERMINAL environment variable.",
                                    QMessageBox::Ok);
              return;
            }
          }
        }
      }
    }
  }

  p->setArguments(QStringList() << "-e" << terminalRcloneCmd);
  p->setProgram(terminal);
#endif

#if !defined(Q_OS_WIN32) || (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
  UseRclonePassword(p);
  p->start(QIODevice::NotOpen);
#endif
}

void MainWindow::rcloneListRemotes() {

  ui.remotes->clear();

  auto settings = GetSettings();
  QString mIconsLayout = settings->value("Settings/iconsLayout").toString();

  QProcess *p = new QProcess();

  QObject::connect(
      p,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int code, QProcess::ExitStatus) {
        if (code == 0) {
          QStyle *style = qApp->style();

          QString bytes = p->readAllStandardOutput().trimmed();
          QStringList items = bytes.split('\n');

          auto settings = GetSettings();
          bool darkModeIni = settings->value("Settings/darkModeIni").toBool();
          QString iconSize = settings->value("Settings/iconSize").toString();
          QString iconsColour =
              settings->value("Settings/iconsColour").toString();

          for (const QString &line : items) {
            if (line.isEmpty()) {
              continue;
            }

            QStringList parts = line.split(':');
            if (parts.count() != 2) {
              continue;
            }

            QString name = parts[0].trimmed();
            QString type = parts[1].trimmed();
            QString tooltip = "type: " + type + "\n\nname: " + name;

            QString img_add = "";
            int size;

            // medium scale by default
            double darkModeIconScale = 1.333;
            double lightModeiconScale = 2;
            // to avoid "variable not used" compiler error
            if (darkModeIconScale == lightModeiconScale) {
            };

            // set icons scale based on iconSize value
            if (iconSize == "S") {
              lightModeiconScale = 3;
              darkModeIconScale = 2;
            }

            if (iconSize == "M") {
              lightModeiconScale = 4;
              darkModeIconScale = 2.666;
            }

            if (iconSize == "L") {
              lightModeiconScale = 6;
              darkModeIconScale = 4;
            }

            if (iconSize == "XL") {
              lightModeiconScale = 8;
              darkModeIconScale = 5.333;
            }

            if (iconSize == "XXL") {
              lightModeiconScale = 15;
              darkModeIconScale = 10;
            }

            // disable scaling - all is fusion now
            // let's leave scaling logic for now
            darkModeIconScale = lightModeiconScale;

#if !defined(Q_OS_MACOS)
            // _inv only for dark mode
            // we use darkModeIni to apply mode active at startup
            if (darkModeIni) {
              if (iconsColour == "white") {
                img_add = "_inv";
              } else {
                img_add = "";
              }
            } else {
              img_add = "";
            }
#if defined(Q_OS_WIN)
            // on Windows dark theme changes PM_ListViewIconSize size
            // so we have to adjust
            if (darkModeIni) {
              size = darkModeIconScale *
                     style->pixelMetric(QStyle::PM_ListViewIconSize);
            } else {
              size = lightModeiconScale *
                     style->pixelMetric(QStyle::PM_ListViewIconSize);
            }
#else
             // for Linux/BSD PM_ListViewIconSize stays the same
             size = lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
#endif
#else
             QString sysInfo = QSysInfo::productVersion();
             // dark mode on older macOS
             if (sysInfo == "10.9" ||
                 sysInfo == "10.10" ||
                 sysInfo == "10.11" ||
                 sysInfo == "10.12" ||
                 sysInfo == "10.13") {

               // on older macOS we also have to adjust icon size per mode
               if (darkModeIni) {
                 size = darkModeIconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
                 if (iconsColour == "white") {
                   img_add = "_inv";
                 } else {
                   img_add = "";
                 }
               } else {
                 size = lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
                 img_add = "";
               }

             } else {
               // for macOS > 10.13 native dark mode does not change IconSize base
               size = 1.5 * lightModeiconScale * style->pixelMetric(QStyle::PM_ListViewIconSize);
               if (iconsColour == "white") {
                  img_add = "_inv";
               } else {
                   img_add = "";
               }
             }
#endif
            ui.remotes->setIconSize(QSize(size, size));

            if (mIconsLayout == "tiles") {
              ui.remotes->setViewMode(QListWidget::IconMode);
              // disable drag and drop
              ui.remotes->setMovement(QListView::Static);
              // always adjust icons after the window is resized
              ui.remotes->setResizeMode(QListView::Adjust);
              ui.remotes->setWrapping(true);
              ui.remotes->setGridSize(QSize(size + 20, size + 40));
              ui.remotes->setSpacing(10);
              ui.remotes->setTextElideMode(Qt::ElideMiddle);
            }
            if (mIconsLayout == "longlist") {
              ui.remotes->setViewMode(QListWidget::ListMode);
              ui.remotes->setResizeMode(QListView::Adjust);
              ui.remotes->setWrapping(false);
              ui.remotes->setGridSize(QSize(size + 800, size + 20));
            }
            if (mIconsLayout == "list") {
              ui.remotes->setViewMode(QListWidget::ListMode);
              ui.remotes->setResizeMode(QListView::Adjust);
              ui.remotes->setWrapping(true);
              ui.remotes->setGridSize(QSize(size + 100, size + 20));
              ui.remotes->setSpacing(10);
            }

            QString path = ":media/images/remotes_icons/" +
                           type.replace(' ', '_') + img_add + ".png";
            QIcon icon(QFile(path).exists()
                           ? path
                           : ":media/images/remotes_icons/unknown" + img_add +
                                 ".png");

            QListWidgetItem *item = new QListWidgetItem(icon, name);
            item->setData(Qt::UserRole, type);
            item->setToolTip(tooltip);
            ui.remotes->addItem(item);
          }
        } else {
          if (p->error() != QProcess::FailedToStart) {
            if (getConfigPassword(p)) {
              rcloneListRemotes();
            }
          }
        }
        p->deleteLater();
        ui.open->setEnabled(false);
      });

  QObject::connect(
      p, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        QString errorString =
            QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error);

        QMessageBox::information(
            this, "Error",
            "Cannot start rclone\n\n Error: " + errorString +
                "\n\nPlease verify rclone excecutable location.");
        emit ui.preferences->trigger();
      });

  UseRclonePassword(p);
  p->start(GetRclone(),
           QStringList() << "listremotes" << GetRcloneConf()
                         << GetDefaultOptionsList("defaultRcloneOptions")
                         << "--long"
                         << "--ask-password=false",
           QIODevice::ReadOnly);
}

bool MainWindow::getConfigPassword(QProcess *p) {
  QString output = p->readAllStandardError().trimmed();
  if (output.indexOf("RCLONE_CONFIG_PASS") > 0) {
    bool ok;
    QString password = QInputDialog::getText(
        this, qApp->applicationDisplayName(),
        "Enter password for .rclone.conf configuration file:",
        QLineEdit::Password, QString(), &ok);
    if (ok) {
      SetRclonePassword(password);
      return true;
    }
  } else if (output.indexOf("unknown command \"listremotes\"") > 0) {
    QMessageBox::critical(this, qApp->applicationDisplayName(),
                          "It seems rclone version you are using is too "
                          "old.\nPlease upgrade to the latest version");
    return false;
  }
  return false;
}

bool MainWindow::canClose() {
  if (mJobCount == 0) {
    return true;
  }

  bool wasVisible = isVisible();

  ui.tabs->setCurrentIndex(1);
  showNormal();

  int button = QMessageBox::question(
      this, "Rclone Browser",
      QString("There are %1 job(s) running.\n"
              "\nDo you want to stop them and quit?")
          .arg(mJobCount),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (!wasVisible) {
    hide();
  }

  if (button == QMessageBox::Yes) {
    // make sure terminated job is not removed from the queue
    // we make close process aware that it is quitting
    mAppQuittingStatus = true;

    int widgetsCount = ui.jobs->count();
    for (int i = widgetsCount - 2; i >= 0; i = i - 2) {
      QWidget *widget = ui.jobs->itemAt(i)->widget();
      if (auto mount = qobject_cast<MountWidget *>(widget)) {
        if (mount->isRunning) {
          emit mount->cancel();
        } else {
          emit mount->closed();
        }

      } else if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        if (transfer->isRunning) {
          emit transfer->cancel();
        } else {
          emit transfer->closed();
        }
      } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
        if (stream->isRunning) {
          emit stream->cancel();
        } else {
          emit stream->closed();
        }
      }
    }
    return true;
  }
  return false;
}

void MainWindow::closeEvent(QCloseEvent *ev) {
  if (mCloseToTray && isVisible()) {
#ifdef Q_OS_MACOS
    osxHideDockIcon();
#endif
    mSystemTray.show();
    hide();
    ev->ignore();
    return;
  }

  if (canClose()) {
    // quitApp() will wait for everything to finish
    quitApp();
    ev->ignore();
  } else {
    ev->ignore();
  }
}

void MainWindow::restoreSchedulersFromFile() {
  // make sure that tasks are already listed so we can cross check

  QString filePath = GetConfigDir().absoluteFilePath("scheduler.conf");
  QFile file(filePath);
  QTextStream in(&file);

  if (!file.open(QIODevice::ReadOnly)) {
    return;
  } else {

    while (!in.atEnd()) {

      QString line = in.readLine();

      // check if corresponding task exist?
      // get scheduler taskId
      QStringList args = line.split(",");
      QString schedulerTaskID = args.at(args.indexOf("mTaskId") + 1);

      ListOfJobOptions *ljo = ListOfJobOptions::getInstance();

      for (JobOptions *jo : ljo->getTasks()) {
        if (jo->uniqueId.toString() == schedulerTaskID) {
          mSchedulersCount++;
          addScheduler("", "", args);
        }
      }
    }

    file.close();
  }

  ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                             .arg(mSchedulersCount)
                             .arg(mRunningSchedulersCount));
}

void MainWindow::addTasksToQueue() {
  // restore queue from file
  // ignore no more existing

  ui.queueListWidget->clear();

  auto items = ui.tasksListWidget->selectedItems();

  ListOfJobOptions *ljo = ListOfJobOptions::getInstance();

  QString filePath = GetConfigDir().absoluteFilePath("queue.conf");
  QFile file(filePath);
  QTextStream in(&file);

  QString fileTaskId;
  QString fileRequestId;

  if (!file.open(QIODevice::ReadOnly)) {
    return;
  } else {

    QString taskNameDisplay;

    while (!in.atEnd()) {

      QString line = in.readLine();

      if (line.indexOf(",") == -1) {
        // old task file
        fileTaskId = line;
        fileRequestId = QUuid::createUuid().toString();

      } else {

        fileTaskId = line.left(line.indexOf(","));
        fileRequestId = line.right(line.length() - (line.indexOf(",") + 1));
      }

      for (JobOptions *jo : ljo->getTasks()) {

        QIcon jobIcon = mDownloadIcon;

        if (jo->jobType == JobOptions::JobType::Download) {
          if (jo->operation == JobOptions::Mount) {
            jobIcon = mMountIcon;
          } else {
            jobIcon = mDownloadIcon;
          }
        }
        if (jo->jobType == JobOptions::JobType::Upload) {
          jobIcon = mUploadIcon;
        }

        if (jo->operation == JobOptions::Mount) {
          if (jo->mountAutoStart) {
            taskNameDisplay = jo->description + "(autostart)";
          } else {
            taskNameDisplay = jo->description;
          }
        } else {
          taskNameDisplay = jo->description;
        }

        // check if task is from scheduler
        bool transferModeSch = false;
        int schedulersCount = ui.schedulers->count();
        for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
          QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
          if (auto scheduler =
                  qobject_cast<SchedulerWidget *>(schedulerWidget)) {
            if (fileRequestId == scheduler->getSchedulerRequestId()) {
              transferModeSch = true;
              scheduler->updateTaskStatus(fileRequestId, "in the queue");
            }
          }
        }

        if (jo->uniqueId.toString() == fileTaskId) {
          if (transferModeSch) {
            taskNameDisplay = taskNameDisplay + " (*Sch)";
            mRunningSchedulersCount++;
            ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                       .arg(mSchedulersCount)
                                       .arg(mRunningSchedulersCount));
          }

          JobOptionsListWidgetItem *item = new JobOptionsListWidgetItem(
              jo, jobIcon, taskNameDisplay, fileRequestId);

          ++mQueueCount;
          ui.queueListWidget->addItem(item);
        }
      }
    }

    file.close();

    if (mQueueCount == 0) {
      ui.tabs->setTabText(3, QString("Queue"));
    } else {
      ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
    }

    ui.labelQueueInfoStop->setText("Queue is not running.");
    ui.labelQueueInfoStop->show();
    ui.labelQueueInfoStart->hide();
  }

  ui.buttonStopQueue->setEnabled(false);
  ui.buttonStartQueue->setEnabled(true);
  ui.buttonDownQueue->setEnabled(false);
  ui.buttonUpQueue->setEnabled(false);
  ui.buttonRemoveFromQueue->setEnabled(false);

  if (ui.queueListWidget->count() > 0) {
    ui.buttonPurgeQueue->setEnabled(true);
  } else {
    ui.buttonPurgeQueue->setEnabled(false);
  }
}

void MainWindow::listTasks() {

  ui.tasksListWidget->clear();

  // make tasks list flow (wrap) in its window
  ui.tasksListWidget->setViewMode(QListWidget::ListMode);
  ui.tasksListWidget->setResizeMode(QListView::Adjust);
  ui.tasksListWidget->setSpacing(5);
  ui.tasksListWidget->setWrapping(true);

  ui.queueListWidget->setViewMode(QListWidget::ListMode);
  ui.queueListWidget->setResizeMode(QListView::Adjust);
  ui.queueListWidget->setSpacing(5);
  ui.queueListWidget->setWrapping(true);

  // enable drag and drop reordering (there is no persistence of order
  // implemented yet)
  // ui.tasksListWidget->setDragDropMode(QAbstractItemView::InternalMove);

  auto settings = GetSettings();
  bool sortTask = settings->value("Settings/sortTask").toBool();
  if (sortTask) {
    //    ui.actionSortTask->setIcon(sortZAIcon);
    //    ui.buttonSortTask->setToolTip("Sort Descending");
    ui.tasksListWidget->setSortingEnabled(true);
    ui.tasksListWidget->sortItems(Qt::AscendingOrder);
  } else {
    //    ui.actionSortTask->setIcon(sortAZIcon);
    //    ui.buttonSortTask->setToolTip("Sort Ascending");
    ui.tasksListWidget->setSortingEnabled(true);
    ui.tasksListWidget->sortItems(Qt::DescendingOrder);
  }

  ListOfJobOptions *ljo = ListOfJobOptions::getInstance();

  for (JobOptions *jo : ljo->getTasks()) {

    QIcon jobIcon;
    QString taskNameDisplay;

    if (jo->jobType == JobOptions::JobType::Download) {
      if (jo->operation == JobOptions::Mount) {
        jobIcon = mMountIcon;
      } else {
        jobIcon = mDownloadIcon;
      }
    }
    if (jo->jobType == JobOptions::JobType::Upload) {
      jobIcon = mUploadIcon;
    }

    if (jo->operation == JobOptions::Mount) {
      if (jo->mountAutoStart) {
        taskNameDisplay = jo->description + " (*auto)";
      } else {
        taskNameDisplay = jo->description;
      }
    } else {
      taskNameDisplay = jo->description;
    }

    JobOptionsListWidgetItem *item = new JobOptionsListWidgetItem(
        jo, jobIcon, taskNameDisplay, "uniqueID_placeholder_task");
    ui.tasksListWidget->addItem(item);
  }

  ui.buttonDeleteTask->setEnabled(false);
  ui.buttonEditTask->setEnabled(false);
  ui.buttonRunTask->setEnabled(false);
  ui.buttonStop->setEnabled(false);
  ui.buttonDryrunTask->setEnabled(false);
  ui.buttonAddToQueue->setEnabled(false);
  ui.buttonAddToScheduler->setEnabled(false);

  if (ui.tasksListWidget->count() > 1) {
    ui.buttonSortTask->setEnabled(true);
  } else {
    ui.buttonSortTask->setEnabled(false);
  }

  // update ui.queueListWidget when task changed
  QString uniqueId_queue;
  QString uniqueId_task;
  bool itemFound = false;

  // for every item in queue
  if (ui.queueListWidget->count() > 0) {
    for (int i = 0; i < ui.queueListWidget->count(); i++) {

      ui.queueListWidget->item(i);

      JobOptionsListWidgetItem *item_queue =
          static_cast<JobOptionsListWidgetItem *>(ui.queueListWidget->item(i));
      JobOptions *jo_queue = item_queue->GetData();
      uniqueId_queue = jo_queue->uniqueId.toString();
      // preserve requestId
      QString requestId = item_queue->GetRequestId();

      // if no corresponding item found in the queue means task has been deleted
      // and have to be removed from the queue as well
      itemFound = false;

      if (ui.tasksListWidget->count() > 0) {

        // check if corresponding item in the task list
        for (int j = 0; j < ui.tasksListWidget->count(); j++) {

          JobOptionsListWidgetItem *item_task =
              static_cast<JobOptionsListWidgetItem *>(
                  ui.tasksListWidget->item(j));
          JobOptions *jo_task = item_task->GetData();

          uniqueId_task = jo_task->uniqueId.toString();

          // uniqueId never changes, name can be edited so we check Id
          if (uniqueId_queue == uniqueId_task) {
            itemFound = true;
            // update ui.queueListWidget

            ui.queueListWidget->takeItem(i);

            QIcon jobIcon = mDownloadIcon;

            if (jo_task->jobType == JobOptions::JobType::Download) {
              if (jo_task->operation == JobOptions::Mount) {
                jobIcon = mMountIcon;
              } else {
                jobIcon = mDownloadIcon;
              }
            }
            if (jo_task->jobType == JobOptions::JobType::Upload) {
              jobIcon = mUploadIcon;
            }

            // check if task is from scheduler
            bool transferModeSch = false;
            int schedulersCount = ui.schedulers->count();
            for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
              QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
              if (auto scheduler =
                      qobject_cast<SchedulerWidget *>(schedulerWidget)) {
                if (requestId == scheduler->getSchedulerRequestId()) {
                  transferModeSch = true;
                }
              }
            }

            QString taskNameDisplay = jo_task->description;
            if (transferModeSch) {

              taskNameDisplay = taskNameDisplay + " (*Sch)";
            }

            JobOptionsListWidgetItem *item_insert =
                new JobOptionsListWidgetItem(jo_task, jobIcon, taskNameDisplay,
                                             requestId);

            ui.queueListWidget->insertItem(i, item_insert);

            if (i == 0 && mQueueStatus) {
              ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
            }
          }
        } // for j
      }

      // remove item from ui.queueListWidget if removed from tasks
      if (mQueueStatus) {
        // running queue
        if (!itemFound) {
          // only if already running leave it
          // never should happen - as not possible to delete already running
          if (i != 0) {
            --mQueueCount;
            ui.queueListWidget->takeItem(i);
            if (mQueueCount == 0) {
              ui.tabs->setTabText(3,
                                  QString("Queue (%1)>>(0)").arg(mQueueCount));
            } else {
              ui.tabs->setTabText(
                  3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
            }
          }
        }

      } else {
        // stopped queue
        if (!itemFound) {
          --mQueueCount;
          ui.queueListWidget->takeItem(i);
          if (mQueueCount == 0) {
            ui.tabs->setTabText(3, QString("Queue"));
          } else {
            ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
          }
        }

      } // mQueueStatus
    }   // for i

    saveQueueFile();
  }

  // update schedulers tasks names and add "(*Sch)" to tasks' names

  // loop over all tasks
  for (int i = 0; i < ui.tasksListWidget->count(); i = i + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(i));
    JobOptions *joTasks = item->GetData();

    // loop over all schedulers
    int schedulersCount = ui.schedulers->count();
    for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
      QWidget *widget = ui.schedulers->itemAt(j)->widget();

      if (auto scheduler = qobject_cast<SchedulerWidget *>(widget)) {

        if (joTasks->uniqueId.toString() == scheduler->getSchedulerTaskId()) {

          // update task name in scheduler

          scheduler->updateTaskName(joTasks->description);

          // update task name in tasks list + " (scheduled)"
          ui.tasksListWidget->takeItem(i);

          QIcon jobIcon = mDownloadIcon;

          if (joTasks->jobType == JobOptions::JobType::Download) {
            if (joTasks->operation == JobOptions::Mount) {
              jobIcon = mMountIcon;
            } else {
              jobIcon = mDownloadIcon;
            }
          }
          if (joTasks->jobType == JobOptions::JobType::Upload) {
            jobIcon = mUploadIcon;
          }

          JobOptionsListWidgetItem *item_insert = new JobOptionsListWidgetItem(
              joTasks, jobIcon, joTasks->description + " (*Sch)",
              "placeholder");

          ui.tasksListWidget->insertItem(i, item_insert);
        }
      }
    }
  }

  // restore active tasks colours
  int widgetsCount = ui.jobs->count();
  for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));

    JobOptions *joTasks = item->GetData();

    for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
      QWidget *widget = ui.jobs->itemAt(j)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        if ((transfer->getUniqueID() == joTasks->uniqueId.toString()) &&
            (transfer->isRunning)) {
          ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
        }
      }

      if (auto mount = qobject_cast<MountWidget *>(widget)) {
        if ((mount->getUniqueID() == joTasks->uniqueId.toString()) &&
            (mount->isRunning)) {
          ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
        }
      }
    }
  }

  ui.queueListWidget->setFocus();
  setQueueButtons();
  ui.tasksListWidget->setFocus();
  setTasksButtons();
  saveSchedulerFile();

} // MainWindow::listTasks()

void MainWindow::runItem(JobOptionsListWidgetItem *item,
                         const QString &transferMode, const QString &requestId,
                         bool dryrun) {

  QMutexLocker locker(&mRunItemMutex);

  if (item == nullptr)
    return;

  // if more than n jobs let's delete some old inactive one

  int widgetsCount = ui.jobs->count();

  if ((widgetsCount - 2) / 2 > 75) {

    // find the oldest inactive job

    QDateTime dt = QDateTime::currentDateTime();
    QDateTime widgetStartDateTime = dt;
    int remove = 0;
    bool foundOldest = false;

    for (int i = widgetsCount - 2; i >= 0; i = i - 2) {

      QWidget *widget = ui.jobs->itemAt(i)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {

        if (!(transfer->isRunning)) {
          widgetStartDateTime = transfer->getStartDateTime();
          foundOldest = true;
        }

      } else if (auto mount = qobject_cast<MountWidget *>(widget)) {
        if (!(mount->isRunning)) {
          widgetStartDateTime = mount->getStartDateTime();
          foundOldest = true;
        }
      } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
        if (!(stream->isRunning)) {
          widgetStartDateTime = stream->getStartDateTime();
          foundOldest = true;
        }
      }

      if (dt > widgetStartDateTime) {
        dt = widgetStartDateTime;
        remove = i;
      }

    } //  for (int i

    if (foundOldest) {
      // remove the oldest from jobs
      QWidget *widget = ui.jobs->itemAt(remove)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        emit transfer->closed();
      } else if (auto mount = qobject_cast<MountWidget *>(widget)) {
        emit mount->closed();
      } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
        emit stream->closed();
      } else {
      }
    }
  }

  JobOptions *jo = item->GetData();

  // running items have darkGreen background
  for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
    JobOptions *joTasks = item->GetData();
    if (joTasks->uniqueId.toString() == jo->uniqueId.toString()) {
      ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
    }
  }

  if (jo->operation != JobOptions::Mount) {
    // transfer
    jo->dryRun = dryrun;
    QStringList args = jo->getOptions();

    QString info;
    QString operation;

    if (int(jo->operation) == 1)
      operation = "Copy";
    if (int(jo->operation) == 2)
      operation = "Move";
    if (int(jo->operation) == 3)
      operation = "Sync";

    if (dryrun) {
      info = QString("Dry run, task: \"%1\", %2 from %3")
                 .arg(jo->description)
                 .arg(operation.toLower())
                 .arg(jo->source);
    } else {
      if (transferMode == "queue") {
        info = QString("Queue task: \"%1\", %2 from %3")
                   .arg(jo->description)
                   .arg(operation)
                   .arg(jo->source);
      } else {

        info = QString("Task: \"%1\", %2 from %3")
                   .arg(jo->description)
                   .arg(operation)
                   .arg(jo->source);
      }
    }

    // check if run item is from scheduler

    bool transferModeSch = false;
    int schedulersCount = ui.schedulers->count();
    for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
      QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
      if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {
        if (requestId == scheduler->getSchedulerRequestId()) {
          transferModeSch = true;
        }
      }
    }

    if (transferMode == "scheduler" || transferModeSch) {

      // check if not already running
      int widgetsCount = ui.jobs->count();
      bool alreadyRunning = false;
      for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
        QWidget *transferWidget = ui.jobs->itemAt(j)->widget();
        if (auto transfer = qobject_cast<JobWidget *>(transferWidget)) {
          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->isRunning)) {
            alreadyRunning = true;
            break;
          }
        }
      }

      QString schedulerTaskStatus;
      if (alreadyRunning) {
        schedulerTaskStatus = "already running";
        mRunningSchedulersCount--;
        ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                   .arg(mSchedulersCount)
                                   .arg(mRunningSchedulersCount));

      } else {
        info = QString("Scheduled task: \"%1\", %2 from %3")
                   .arg(jo->description)
                   .arg(operation)
                   .arg(jo->source);
        addTransfer(info, jo->source, jo->dest, args, jo->uniqueId.toString(),
                    transferMode, requestId);
        schedulerTaskStatus = "running";
      }

      // notify schedulers
      int schedulersCount = ui.schedulers->count();
      for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
        QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
        if (auto scheduler = qobject_cast<SchedulerWidget *>(schedulerWidget)) {
          scheduler->updateTaskStatus(requestId, schedulerTaskStatus);
        }
      }

    } else {

      addTransfer(info, jo->source, jo->dest, args, jo->uniqueId.toString(),
                  transferMode, requestId);
    }

  } else {
    // mount

    QStringList args;

    args << "mount";

    args << jo->source;

    args << jo->dest;

    if (!jo->mountRcPort.isEmpty()) {

      args << "--rc";
      args << "--rc-addr";
      args << "localhost:" + jo->mountRcPort;

      // generate random username and password
      const QString possibleCharacters(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

      QString rcUser;
      for (int i = 0; i < 10; ++i) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
        int index = QRandomGenerator::global()->generate() %
                    possibleCharacters.length();
#else
        int index = qrand() % possibleCharacters.length();
#endif
        QChar nextChar = possibleCharacters.at(index);
        rcUser.append(nextChar);
      }

      QString rcPass;
      for (int i = 0; i < 22; ++i) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
        int index = QRandomGenerator::global()->generate() %
                    possibleCharacters.length();
#else
        int index = qrand() % possibleCharacters.length();
#endif

        QChar nextChar = possibleCharacters.at(index);
        rcPass.append(nextChar);
      }

      args << "--rc-user=" + rcUser;
      args << "--rc-pass=" + rcPass;
    }
    if (jo->remoteType == "drive") {
      if (jo->remoteMode == "shared") {
        args << "--drive-shared-with-me";
        if (!jo->mountReadOnly) {
          args << "--read-only";
        }
      }
      if (jo->remoteMode == "trash") {
        args << "--drive-trashed-only";
      }
    }

    if (jo->mountReadOnly) {
      args << "--read-only";
    }

    if (!(jo->mountVolume.trimmed().isEmpty())) {
      args << "--volname";
      args << jo->mountVolume;
    }

    switch (jo->mountCacheLevel) {
    case JobOptions::MountCacheLevel::Off:
      break;
    case JobOptions::MountCacheLevel::Minimal:
      args << "--vfs-cache-mode";
      args << "minimal";
      break;
    case JobOptions::MountCacheLevel::Writes:
      args << "--vfs-cache-mode";
      args << "writes";
      break;
    case JobOptions::MountCacheLevel::Full:
      args << "--vfs-cache-mode";
      args << "full";
      break;
    case JobOptions::MountCacheLevel::UnknownCacheLevel:
      break;
    }

    if (!jo->extra.trimmed().isEmpty()) {
      for (auto line : jo->extra.trimmed().split('\n')) {
        if (!line.isEmpty()) {
          for (QString arg :
               line.split(QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
            if (!arg.isEmpty()) {
              args << arg.replace("\"", "");
            }
          }
        }
      }
    }

    addNewMount(jo->source, jo->dest, jo->remoteType, args, jo->mountScript,
                jo->uniqueId.toString(), "Mounting task: " + jo->description);
  }
}

void MainWindow::editSelectedTask() {

  auto items = ui.tasksListWidget->selectedItems();

  // remember which item we edit
  int selection = (ui.tasksListWidget->selectionModel()->currentIndex()).row();

  foreach (auto i, items) {

    JobOptionsListWidgetItem *item = static_cast<JobOptionsListWidgetItem *>(i);

    JobOptions *jo = item->GetData();
    bool isDownload = (jo->jobType == JobOptions::Download);
    QString remoteType = (jo->remoteType);
    QString remoteMode = (jo->remoteMode);

    QString jobType = "";

    if (jo->jobType == JobOptions::Download) {
      if (jo->operation == JobOptions::Mount) {
        jobType = "Mount";
      } else {
        jobType = "Download";
      }
    }
    if (jo->jobType == JobOptions::Upload) {
      jobType = "Upload";
    }

    QString remote = isDownload ? jo->source : jo->dest;
    QString path = isDownload ? jo->dest : jo->source;
    // qDebug() << "remote:" + remote;
    // qDebug() << "path:" + path;

    if (jobType == "Download" || jobType == "Upload") {

      QStringList empty;
      TransferDialog td(isDownload, false, remote, path, jo->isFolder,
                        remoteType, remoteMode, false, empty, this, jo, true);
      td.exec();

    } else {
      if (jobType == "Mount") {
        MountDialog md(remote, path, remoteType, remoteMode, this, jo, true);
        md.exec();
      }
    }
  }

  // restore the selection to help user keep track of what s/he was doing
  ui.tasksListWidget->setCurrentItem(ui.tasksListWidget->item(selection));
  ui.tasksListWidget->setFocus();
  setTasksButtons();

  // restore active tasks colours
  int widgetsCount = ui.jobs->count();
  for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));

    JobOptions *joTasks = item->GetData();

    for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
      QWidget *widget = ui.jobs->itemAt(j)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        if ((transfer->getUniqueID() == joTasks->uniqueId.toString()) &&
            (transfer->isRunning)) {
          ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
        }
      }

      if (auto mount = qobject_cast<MountWidget *>(widget)) {
        if ((mount->getUniqueID() == joTasks->uniqueId.toString()) &&
            (mount->isRunning)) {
          ui.tasksListWidget->item(k)->setBackground(Qt::darkGreen);
        }
      }
    }
  }

  // edit mode on the TransferDialog suppresses the usual Accept buttons
  // and the Save Task button closes it... so there is nothing more to do here
}

bool MainWindow::saveQueueFile(void) {

  QMutexLocker locker(&mSaveQueueFileMutex);
  QString filePath = GetConfigDir().absoluteFilePath("queue.conf");

#if QT_VERSION < QT_VERSION_CHECK(5, 1, 0)
  QFile file(filePath);
#else
  QSaveFile file(filePath);
#endif

  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  QTextStream out(&file);

  // loop over ui.queueListWidget
  for (int i = 0; i < ui.queueListWidget->count(); ++i) {
    QListWidgetItem *item = ui.queueListWidget->item(i);

    JobOptionsListWidgetItem *jobItem =
        static_cast<JobOptionsListWidgetItem *>(item);

    JobOptions *jo = jobItem->GetData();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
    out << jo->uniqueId.toString() << "," << jobItem->GetRequestId()
        << Qt::endl;
#else
    out << jo->uniqueId.toString() << "," << jobItem->GetRequestId() << endl;
#endif
  }

  out.flush();

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
  return out.status() == QTextStream::Ok && file.commit();
#else
  return out.status() == QTextStream::Ok;
#endif
}

bool MainWindow::saveSchedulerFile(void) {

  QMutexLocker locker(&mSaveSchedulerFileMutex);
  QString filePath = GetConfigDir().absoluteFilePath("scheduler.conf");

#if QT_VERSION < QT_VERSION_CHECK(5, 1, 0)
  QFile file(filePath);
#else
  QSaveFile file(filePath);
#endif

  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  };

  QTextStream out(&file);
  int schedulersCount = ui.schedulers->count();

  for (int i = schedulersCount - 2; i >= 0; i = i - 2) {
    QWidget *widget = ui.schedulers->itemAt(i)->widget();
    if (auto scheduler = qobject_cast<SchedulerWidget *>(widget)) {
      QStringList args = scheduler->getSchedulerParameters();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 1)
      out << args.join(",") << Qt::endl;
#else
      out << args.join(",") << endl;
#endif
    }
  }

  out.flush();

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
  return out.status() == QTextStream::Ok && file.commit();
#else
  return out.status() == QTextStream::Ok;
#endif
}

void MainWindow::addSavedTransfer(const QString &uniqueId, bool dryRun,
                                  bool addToQueue) {

  QMutexLocker locker(&mMutex);

  // keep for future use
  if (dryRun) {
  }

  // find task based on taskID
  for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
    JobOptionsListWidgetItem *item =
        static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
    JobOptions *joTask = item->GetData();

    if (uniqueId == joTask->uniqueId.toString()) {

      if (!addToQueue) {

        // run immediately
        runItem(item, "task", QUuid::createUuid().toString(), false);
        break;
      } else {
        // add to queue

        bool isQueueEmpty = (ui.queueListWidget->count() == 0);

        QIcon jobIcon;

        if (joTask->jobType == JobOptions::JobType::Download) {
          if (joTask->operation == JobOptions::Mount) {
            jobIcon = mMountIcon;
          } else {
            jobIcon = mDownloadIcon;
          }
        }
        if (joTask->jobType == JobOptions::JobType::Upload) {
          jobIcon = mUploadIcon;
        }

        JobOptionsListWidgetItem *newitem =
            new JobOptionsListWidgetItem(joTask, jobIcon, joTask->description,
                                         QUuid::createUuid().toString());

        ui.queueListWidget->addItem(newitem);
        mQueueCount = mQueueCount + 1;

        // if queue was empty we start first taks if queue is running and
        // there is no other transfer job running
        if (mQueueStatus && isQueueEmpty && (mTransferJobCount == 0)) {

          if (mQueueCount > 0) {

            JobOptionsListWidgetItem *item =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.queueListWidget->item(0));

            runItem(item, "scheduler", item->GetRequestId());
            ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
            mQueueTaskRunning = true;
            ui.tabs->setTabText(
                3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
          }

        } else {

          if (mQueueStatus) {

            if (mQueueCount == 0) {
              ui.tabs->setTabText(3,
                                  QString("Queue (%1)>>(0)").arg(mQueueCount));
            } else {
              if (!mQueueTaskRunning) {
                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(0)").arg(mQueueCount));
              } else {
                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
              }
            }
          } else {
            if (mQueueCount == 0) {
              ui.tabs->setTabText(3, QString("Queue"));
            } else {
              ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
            }
          }
        }
        saveQueueFile();
        break;
      }
    }
  }
}

void MainWindow::addTransfer(const QString &message, const QString &source,
                             const QString &dest, const QStringList &args,
                             const QString &uniqueId,
                             const QString &transferMode,
                             const QString &requestId) {

  QProcess *transfer = new QProcess(this);
  transfer->setProcessChannelMode(QProcess::MergedChannels);

  auto widget = new JobWidget(transfer, message, args, source, dest, uniqueId,
                              transferMode, requestId);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  //!!!  QObject::connect(  widget, &JobWidget::finished
  QObject::connect(
      widget, &JobWidget::finished, this,
      [=](const QString &info, const QString &jobFinalStatus) {
        QMutexLocker locker(&mMutex);

        if (mNotifyFinishedTransfers) {
          mLastFinished = widget;
#if defined(Q_OS_WIN)
          mSystemTray.showMessage(
              "Rclone Browser - transfer " + jobFinalStatus, info,
              QIcon(":media/images/program_icons/rclone-browser512.png"));
#else
#if defined(Q_OS_MACOS)
          MacOsNotification::Display(
              "Rclone Browser - transfer " + jobFinalStatus, info);
#else
          mSystemTray.showMessage("Rclone Browser - transfer " + jobFinalStatus,
                                  info, QSystemTrayIcon::Information);
#endif
#endif
        }

        if (mSoundNotif) {
          // play notification sound
          QSound::play(":media/sounds/notification-sound.wav");
        }

        --mTransferJobCount;

        if (mTransferJobCount == 0) {

      // allow entering sleep
#if defined(Q_OS_WIN)
          SetThreadExecutionState(ES_CONTINUOUS);
#endif
#if defined(Q_OS_MACOS)
          mMacOsPowerSaving->resumePowerSaving();
#endif
          // run custom script
          auto settings = GetSettings();
          QString transferOffScript =
              settings->value("Settings/transferOffScript").toString();

          bool jobLastFinishedScriptRun =
              settings->value("Settings/jobLastFinishedScriptRun", false)
                  .toBool();
          if (jobLastFinishedScriptRun) {

            if (!transferOffScript.isEmpty()) {
              runScript(transferOffScript);
            }
          }
        }

        if (--mJobCount == 0) {
          ui.tabs->setTabText(1, "Jobs");
        } else {
          ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
        }

        ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
        ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                             (ui.jobs->count() - 2) / 2);

        // transfer finished - remove darkGreen from tasks list
        auto transfer = qobject_cast<JobWidget *>(widget);
        for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(
                  ui.tasksListWidget->item(k));
          JobOptions *jo = item->GetData();
          if (transfer->getUniqueID() == jo->uniqueId.toString()) {

            ui.tasksListWidget->item(k)->setBackground(QBrush());
            break;
          }
        }

        // if job finished try to run next one from the queue
        // only when not quitting, queue is active and there are tasks in the
        // queue and there is no running other task

        if (!mAppQuittingStatus && mQueueStatus &&
            ui.queueListWidget->count() > 0) {
          //            ui.queueListWidget->count() > 0 && mTransferJobCount ==
          //            0) {

          auto QueueRunningTask = ui.queueListWidget->item(0);

          JobOptionsListWidgetItem *item =
              static_cast<JobOptionsListWidgetItem *>(QueueRunningTask);
          JobOptions *jo = item->GetData();

          // check if finished task the same as running from the queue
          auto transfer = qobject_cast<JobWidget *>(widget);

          // we also have to check requestId - to distinguish between the same
          // task triggered by queue/scheduler and by user directly if yes we
          // try to run next one
          if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
              (transfer->getRequestId() == item->GetRequestId())) {
            ui.queueListWidget->takeItem(0);
            --mQueueCount;

            // queue is still running, even if mQueueCount is 0
            if (mQueueCount == 0) {
              mQueueTaskRunning = false;
              ui.tabs->setTabText(3,
                                  QString("Queue (%1)>>(0)").arg(mQueueCount));
              // run queueScript
              auto settings = GetSettings();
              bool queueScriptRun =
                  settings->value("Settings/queueScriptRun", false).toBool();

              if (queueScriptRun) {
                QString queueScript =
                    settings->value("Settings/queueScript").toString();
                if (!queueScript.isEmpty()) {
                  runScript(queueScript);
                }
              }
            }

            // if there is still task to run
            if ((ui.queueListWidget->count()) > 0) {

              auto nextTask = ui.queueListWidget->item(0);
              // check if task is not already running (user could run it
              // manually)

              bool isAlreadyRunning = false;

              JobOptionsListWidgetItem *item =
                  static_cast<JobOptionsListWidgetItem *>(nextTask);
              JobOptions *jo = item->GetData();

              int widgetsCount = ui.jobs->count();
              for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
                QWidget *widget = ui.jobs->itemAt(j)->widget();
                if (auto transfer = qobject_cast<JobWidget *>(widget)) {
                  if ((transfer->getUniqueID() == jo->uniqueId.toString()) &&
                      (transfer->isRunning)) {
                    isAlreadyRunning = true;
                    break;
                  }
                }
              }

              // if the same task not running and nothing else running
              if (!isAlreadyRunning && mTransferJobCount == 0) {
                mQueueTaskRunning = true;
                runItem(item, "queue", item->GetRequestId());

                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));

                ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
              } else {

                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(0)").arg(mQueueCount));

                mQueueTaskRunning = false;
              }
            }
            saveQueueFile();
          } else {
            // finished task was not one from the queue
            // queue is active and have some tasks and if there is nothing else
            // running we start top task from the queue
            if (!mQueueTaskRunning) {
              auto nextTask = ui.queueListWidget->item(0);

              JobOptionsListWidgetItem *item =
                  static_cast<JobOptionsListWidgetItem *>(nextTask);

              if (mTransferJobCount == 0) {

                mQueueTaskRunning = true;
                runItem(item, "queue", item->GetRequestId());
                ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
              }

            } else {

              // mQueueTaskRunning = false;
              ui.tabs->setTabText(
                  3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
            }
          }
        }

        setTasksButtons();

        // notify schedulers about finished task
        int schedulersCount = ui.schedulers->count();
        for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
          QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();
          if (auto scheduler =
                  qobject_cast<SchedulerWidget *>(schedulerWidget)) {
            // jobFinalStatus = stopped, error, finished

            if (transfer->getRequestId() ==
                scheduler->getSchedulerRequestId()) {
              mRunningSchedulersCount--;
              ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                         .arg(mSchedulersCount)
                                         .arg(mRunningSchedulersCount));

              scheduler->updateTaskStatus(transfer->getRequestId(),
                                          jobFinalStatus);
            }
          }
        }

        // if queue was stopped task terminated but still in the queue
        // then scheduled task is "in the queue" not "stopped"
        // we notify schedulers what is still in the queue

        if (mQueueStatus == false) {
          int itemsCount = ui.queueListWidget->count();
          if (itemsCount > 0) {
            JobOptionsListWidgetItem *item_queue =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.queueListWidget->item(0));

            int schedulersCount = ui.schedulers->count();
            for (int j = schedulersCount - 2; j >= 0; j = j - 2) {
              QWidget *schedulerWidget = ui.schedulers->itemAt(j)->widget();

              if (auto scheduler =
                      qobject_cast<SchedulerWidget *>(schedulerWidget)) {
                // jobFinalStatus = stopped, error, finished
                scheduler->updateTaskStatus(item_queue->GetRequestId(),
                                            "in the queue");

                if (transfer->getRequestId() ==
                        scheduler->getSchedulerRequestId() &&
                    transfer->getRequestId() == item_queue->GetRequestId()) {
                  mRunningSchedulersCount++;
                  ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                             .arg(mSchedulersCount)
                                             .arg(mRunningSchedulersCount));
                }
              }
            }
          }
        }

        // job status changed so we have to sort jobs list

        sortJobs();
      });

  QObject::connect(widget, &JobWidget::closed, this, [=]() {
    if (widget == mLastFinished) {
      mLastFinished = nullptr;
    }

    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;

    int _jobsCount = (ui.jobs->count() - 2) / 2;
    ui.buttonSortByTime->setEnabled(_jobsCount > 1);
    ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

    //    sortJobs();

    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }
    ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
    ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                         (ui.jobs->count() - 2) / 2);
  });

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);
  ++mTransferJobCount;

  int _jobsCount = (ui.jobs->count() - 2) / 2;
  ui.buttonSortByTime->setEnabled(_jobsCount > 1);
  ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

  sortJobs();

  // prevent OS sleep when transfer running
#if defined(Q_OS_WIN)
  SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED |
                          ES_AWAYMODE_REQUIRED);
#endif
#if defined(Q_OS_MACOS)
  mMacOsPowerSaving->suspendPowerSaving();
#endif

  // run custom script
  auto settings = GetSettings();
  QString transferOnScript =
      settings->value("Settings/transferOnScript").toString();

  bool jobStartScriptRun =
      settings->value("Settings/jobStartScriptRun", false).toBool();

  if (jobStartScriptRun) {

    if (!transferOnScript.isEmpty()) {
      runScript(transferOnScript);
    }
  }

  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
  ui.buttonCleanNotRunning->setEnabled(mJobCount != (ui.jobs->count() - 2) / 2);

  UseRclonePassword(transfer);
  transfer->start(GetRclone(), args + GetRcloneConf(), QIODevice::ReadOnly);

  ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
  ui.buttonCleanNotRunning->setEnabled(mJobCount != (ui.jobs->count() - 2) / 2);

  // job status changed so we have to sort jobs list
  sortJobs();
}

//  runs Script
void MainWindow::runScript(const QString &script) {

  QProcess *p = new QProcess();

  QObject::connect(p,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int code, QProcess::ExitStatus) {
                     if (code == 0) {
                     }
                     p->deleteLater();
                   });

  QStringList scriptList;

  for (QString arg : script.split(QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
    if (!arg.isEmpty()) {
      scriptList << arg.replace("\"", "");
    }
  }

  QString scriptCmd = scriptList.takeAt(0);
  QStringList scriptArgs = scriptList;

  p->start(scriptCmd, scriptArgs, QIODevice::ReadOnly);
}

void MainWindow::addNewMount(const QString &remote, const QString &folder,
                             const QString &remoteType, const QStringList &args,
                             const QString &script, const QString &uniqueId,
                             const QString &info) {

  if (remoteType == "") {
  }

  QStringList argsFinal = args;

  QProcess *mount = new QProcess(this);
  mount->setProcessChannelMode(QProcess::MergedChannels);

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  // get default mount options
  auto settings = GetSettings();
  QString opt = settings->value("Settings/mount").toString();

  if (!opt.isEmpty()) {
    // split on spaces but not if inside quotes e.g. --option-1 --option-2="arg1
    // arg2" --option-3 arg3 should generate "--option-1" "--option-2=\"arg1
    // arg2\"" "--option-3" "arg3"
    for (QString arg : opt.split(QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
      if (!arg.isEmpty()) {
        argsFinal << arg.replace("\"", "");
      }
    }
  }

  argsFinal << GetRcloneConf();

  auto widget =
      new MountWidget(mount, remote, folder, argsFinal, script, uniqueId, info);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);

  int _jobsCount = (ui.jobs->count() - 2) / 2;
  ui.buttonSortByTime->setEnabled(_jobsCount > 1);
  ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

  QObject::connect(widget, &MountWidget::finished, this, [=]() {
    if (--mJobCount == 0) {
      ui.tabs->setTabText(1, "Jobs");
    } else {
      ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
    }

    ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
    ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                         (ui.jobs->count() - 2) / 2);

    // mount finished - we remove darkGreen from task list
    auto mount = qobject_cast<MountWidget *>(widget);
    for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
      JobOptions *jo = item->GetData();
      if (mount->getUniqueID() == jo->uniqueId.toString()) {

        ui.tasksListWidget->item(k)->setBackground(QBrush());
      }
    }
    setTasksButtons();
    sortJobs();
  });

  QObject::connect(widget, &MountWidget::closed, this, [=]() {
    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;

    int _jobsCount = (ui.jobs->count() - 2) / 2;
    ui.buttonSortByTime->setEnabled(_jobsCount > 1);
    ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }
    ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
    ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                         (ui.jobs->count() - 2) / 2);

    setTasksButtons();
  });

  UseRclonePassword(mount);
  mount->start(GetRclone(), argsFinal, QIODevice::ReadOnly);

  ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
  ui.buttonCleanNotRunning->setEnabled(mJobCount != (ui.jobs->count() - 2) / 2);
  sortJobs();
}

void MainWindow::addScheduler(const QString &taskId, const QString &taskName,
                              const QStringList &args) {

  if (ui.schedulers->count() == 2) {
    ui.noSchedulesAvailable->hide();
  }

  auto widget = new SchedulerWidget(taskId, taskName, args);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QObject::connect(widget, &SchedulerWidget::closed, this, [=]() {
    ui.schedulers->removeWidget(widget);
    ui.schedulers->removeWidget(line);
    widget->deleteLater();
    delete line;

    mSchedulersCount--;

    auto settings = GetSettings();
    if ((settings->value("Settings/schedulerStatus").toBool())) {

      ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                 .arg(mSchedulersCount)
                                 .arg(mRunningSchedulersCount));
    } else {

      ui.tabs->setTabText(4, QString("Scheduler (%1)").arg(mSchedulersCount));
    }

    saveSchedulerFile();
    if (ui.schedulers->count() == 2) {
      ui.noSchedulesAvailable->show();
    }
    QTimer::singleShot(0, this, SLOT(listTasks()));
  });

  QObject::connect(widget, &SchedulerWidget::save, this,
                   [=]() { saveSchedulerFile(); });

  QObject::connect(widget, &SchedulerWidget::stopTask, this, [=]() {
    QMutexLocker locker(&mStopTaskMutex);
    mDoNotSort = true;
    QString requestID = widget->getSchedulerRequestId();

    // stop if running
    int widgetsCount = ui.jobs->count();
    for (int j = widgetsCount - 2; j >= 0; j = j - 2) {
      QWidget *widget = ui.jobs->itemAt(j)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {

        if (requestID == transfer->getRequestId()) {

          if (transfer->isRunning) {
            emit transfer->cancel();

            //               mRunningSchedulersCount--;
            ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                       .arg(mSchedulersCount)
                                       .arg(mRunningSchedulersCount));

            if (ui.queueListWidget->count() > 0) {
              for (int i = 0; i < ui.queueListWidget->count(); i++) {

                JobOptionsListWidgetItem *item_queue =
                    static_cast<JobOptionsListWidgetItem *>(
                        ui.queueListWidget->item(i));

                if (item_queue->GetRequestId() == requestID) {

                  ui.queueListWidget->takeItem(i);

                  break;
                }
              }
            }
          }
        }
      }
    }

    // remove from the queue
    // for every item in queue
    if (ui.queueListWidget->count() > 0) {
      for (int i = 0; i < ui.queueListWidget->count(); i++) {

        JobOptionsListWidgetItem *item_queue =
            static_cast<JobOptionsListWidgetItem *>(
                ui.queueListWidget->item(i));

        if (item_queue->GetRequestId() == requestID) {

          --mQueueCount;
          ui.queueListWidget->takeItem(i);
          widget->updateTaskStatus(requestID, "removed from the queue");
          mRunningSchedulersCount--;
          ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                     .arg(mSchedulersCount)
                                     .arg(mRunningSchedulersCount));

          break;
        }
      }

      if (mQueueStatus) {

        if (mQueueCount == 0) {
          ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
        } else {

          if (!mQueueTaskRunning) {
            ui.tabs->setTabText(3, QString("Queue (%1)>>(0)").arg(mQueueCount));
          } else {
            ui.tabs->setTabText(
                3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
          }
        }
      } else {
        if (mQueueCount == 0) {
          ui.tabs->setTabText(3, QString("Queue"));
        } else {
          ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
        }
      }
    }
    mDoNotSort = false;
    sortJobs();
  });

  QObject::connect(widget, &SchedulerWidget::editTask, this, [=]() {
    QString taskID = widget->getSchedulerTaskId();

    for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
      JobOptions *joTasks = item->GetData();

      if (taskID == joTasks->uniqueId.toString()) {

        bool isDownload = (joTasks->jobType == JobOptions::Download);
        QString remoteType = (joTasks->remoteType);
        QString remoteMode = (joTasks->remoteMode);

        QString remote = isDownload ? joTasks->source : joTasks->dest;
        QString path = isDownload ? joTasks->dest : joTasks->source;

        QStringList empty;
        TransferDialog td(isDownload, false, remote, path, joTasks->isFolder,
                          remoteType, remoteMode, false, empty, this, joTasks,
                          true);
        td.exec();
        break;
      }
    }
  });

  QObject::connect(widget, &SchedulerWidget::runTask, this, [=]() {
    QMutexLocker locker(&mMutex);
    mDoNotSort = true;
    // when quitting (waiting for unmount) don't start new tasks
    if (mAppQuittingStatus) {
      mDoNotSort = false;
      return;
    }

    QString taskID = widget->getSchedulerTaskId();
    QString requestID = widget->getSchedulerRequestId();
    int executionMode = widget->getExecutionMode();

    // find task based on taskID
    for (int k = 0; k < ui.tasksListWidget->count(); k = k + 1) {
      JobOptionsListWidgetItem *item =
          static_cast<JobOptionsListWidgetItem *>(ui.tasksListWidget->item(k));
      JobOptions *joTask = item->GetData();

      if (taskID == joTask->uniqueId.toString()) {

        if (executionMode == 0) {
          // run immediately
          mRunningSchedulersCount++;
          ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                     .arg(mSchedulersCount)
                                     .arg(mRunningSchedulersCount));

          runItem(item, "scheduler", requestID);
        }

        if (executionMode == 1) {
          // add to the queue

          // if the same task is already in the queue we skip adding another one
          int queueCount = ui.queueListWidget->count();
          for (int i = 0; i < queueCount; i++) {
            JobOptionsListWidgetItem *item_queue =
                static_cast<JobOptionsListWidgetItem *>(
                    ui.queueListWidget->item(i));

            JobOptions *jo_queue = item_queue->GetData();
            QString uniqueId_queue = jo_queue->uniqueId.toString();
            if (taskID == uniqueId_queue) {
              widget->updateTaskStatus(requestID, "task already in the queue");
              mDoNotSort = false;
              return;
            }
          }

          bool isQueueEmpty = (ui.queueListWidget->count() == 0);

          QIcon jobIcon;

          if (joTask->jobType == JobOptions::JobType::Download) {
            if (joTask->operation == JobOptions::Mount) {
              jobIcon = mMountIcon;
            } else {
              jobIcon = mDownloadIcon;
            }
          }
          if (joTask->jobType == JobOptions::JobType::Upload) {
            jobIcon = mUploadIcon;
          }

          JobOptionsListWidgetItem *newitem = new JobOptionsListWidgetItem(
              joTask, jobIcon, joTask->description + " (*Sch)", requestID);

          ui.queueListWidget->addItem(newitem);
          mQueueCount = mQueueCount + 1;

          widget->updateTaskStatus(requestID, "in the queue");
          mRunningSchedulersCount++;
          ui.tabs->setTabText(4, QString("Scheduler (%1)>>(%2)")
                                     .arg(mSchedulersCount)
                                     .arg(mRunningSchedulersCount));

          // if queue was empty we start first taks if queue is running and
          // there is no other transfer job running
          if (mQueueStatus && isQueueEmpty && (mTransferJobCount == 0)) {

            if (mQueueCount > 0) {

              JobOptionsListWidgetItem *item =
                  static_cast<JobOptionsListWidgetItem *>(
                      ui.queueListWidget->item(0));

              runItem(item, "scheduler", item->GetRequestId());
              ui.queueListWidget->item(0)->setBackground(Qt::darkGreen);
              mQueueTaskRunning = true;
              ui.tabs->setTabText(
                  3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
            }

          } else {

            if (mQueueStatus) {

              if (mQueueCount == 0) {
                ui.tabs->setTabText(
                    3, QString("Queue (%1)>>(0)").arg(mQueueCount));
              } else {
                if (!mQueueTaskRunning) {
                  ui.tabs->setTabText(
                      3, QString("Queue (%1)>>(0)").arg(mQueueCount));
                } else {
                  ui.tabs->setTabText(
                      3, QString("Queue (%1)>>(1)").arg(mQueueCount - 1));
                }
              }
            } else {
              if (mQueueCount == 0) {
                ui.tabs->setTabText(3, QString("Queue"));
              } else {
                ui.tabs->setTabText(3, QString("Queue (%1)").arg(mQueueCount));
              }
            }
          }
          saveQueueFile();
        }
      }
    }
    mDoNotSort = false;
    sortJobs();
  });

  ui.schedulers->insertWidget(0, widget);
  ui.schedulers->insertWidget(1, line);
}

void MainWindow::addStream(const QString &remote, const QString &stream,
                           const QString &remoteType) {
  auto player = new QProcess();
  auto rclone = new QProcess();
  rclone->setStandardOutputProcess(player);

  QObject::connect(
      player,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      this, [=](int status, QProcess::ExitStatus) {
        player->deleteLater();
        if (status != 0 && player->error() == QProcess::FailedToStart) {
          QMessageBox::critical(
              this, "Error",
              QString("Failed to start '%1' player process").arg(stream));
          auto settings = GetSettings();
          settings->remove("Settings/streamConfirmed");
        }
      });

  ui.tabs->setTabText(1, QString("Jobs (%1)").arg(++mJobCount));

  ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
  ui.buttonCleanNotRunning->setEnabled(mJobCount != (ui.jobs->count() - 2) / 2);

  auto settings = GetSettings();
  QString opt = settings->value("Settings/mount").toString();
  QString driveSharedMode = settings->value("Settings/remoteMode").toString();

  QStringList args;

  args << "cat";

  args << remote;

  if (remoteType == "drive") {

    if (driveSharedMode == "shared") {
      args << "--drive-shared-with-me";
    }

    if (driveSharedMode == "trash") {
      args << "--drive-trashed-only";
    }
  };

  args << GetDefaultOptionsList("defaultRcloneOptions");

  args << GetRcloneConf();

  auto widget = new StreamWidget(rclone, player, remote, stream, args);

  auto line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);

  QObject::connect(widget, &StreamWidget::finished, this, [=]() {
    if (--mJobCount == 0) {
      ui.tabs->setTabText(1, "Jobs");
    } else {
      ui.tabs->setTabText(1, QString("Jobs (%1)").arg(mJobCount));
    }

    ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
    ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                         (ui.jobs->count() - 2) / 2);
    sortJobs();
  });

  QObject::connect(widget, &StreamWidget::closed, this, [=]() {
    ui.jobs->removeWidget(widget);
    ui.jobs->removeWidget(line);
    widget->deleteLater();
    delete line;
    if (ui.jobs->count() == 2) {
      ui.noJobsAvailable->show();
    }

    int _jobsCount = (ui.jobs->count() - 2) / 2;
    ui.buttonSortByTime->setEnabled(_jobsCount > 1);
    ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

    ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
    ui.buttonCleanNotRunning->setEnabled(mJobCount !=
                                         (ui.jobs->count() - 2) / 2);
  });

  if (ui.jobs->count() == 2) {
    ui.noJobsAvailable->hide();
  }

  ui.jobs->insertWidget(0, widget);
  ui.jobs->insertWidget(1, line);

  int _jobsCount = (ui.jobs->count() - 2) / 2;
  ui.buttonSortByTime->setEnabled(_jobsCount > 1);
  ui.buttonSortByStatus->setEnabled(_jobsCount > 1);

  QStringList streamPrefsList;

  for (QString arg : stream.split(QRegExp(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
    if (!arg.isEmpty()) {
      streamPrefsList << arg.replace("\"", "");
    }
  }

  QString streamCmd = streamPrefsList.takeAt(0);
  QStringList streamArgs = streamPrefsList;

  player->start(streamCmd, streamArgs, QProcess::ReadOnly);

  UseRclonePassword(rclone);
  rclone->start(GetRclone(), QStringList() << args, QProcess::WriteOnly);

  ui.buttonStopAllJobs->setEnabled(mTransferJobCount != 0);
  ui.buttonCleanNotRunning->setEnabled(mJobCount != (ui.jobs->count() - 2) / 2);

  sortJobs();
}

void MainWindow::slotCloseTab(int index) {

  // only when last remote tab is closed return to remote list tab
  // only when closing current tab
  if (ui.tabs->currentIndex() == index) {

    if (ui.tabs->count() == 6) {
      if (index == 5) {
        ui.tabs->setCurrentIndex(0);
      }
    } else {

      if (ui.tabs->count() == 7) {
        ui.tabs->setCurrentIndex(5);
      }
    }
  }
}

void MainWindow::sortJobs() {

  //  bool mJobsTimeSortOrder = false;
  //  bool mJobsStatusSortOrder = false;
  //  QString mJobsSort = "byDate";

  //  QMutexLocker locker(&mMutex);

  QMutexLocker locker(&mJobsSortMutex);

  // don't sort when quitting or stopping all transfers
  if (mAppQuittingStatus) {
    return;
  }
  if (mDoNotSort) {
    return;
  }

  int widgetsCount = ui.jobs->count();
  int move = widgetsCount > 2 ? widgetsCount - 4 : 0;
  QDateTime dt;
  QDateTime widgetStartDateTime;
  QString widgetStatus = 0;
  QString ws;

  for (int i = 0; i < (widgetsCount - 2) / 2 - 1; i = i + 1) {

    for (int j = widgetsCount - 4; j >= i * 2; j = j - 2) {

      QWidget *widget = ui.jobs->itemAt(j)->widget();

      if (auto transfer = qobject_cast<JobWidget *>(widget)) {
        widgetStartDateTime = transfer->getStartDateTime();
        widgetStatus = transfer->getStatus();

      } else if (auto mount = qobject_cast<MountWidget *>(widget)) {
        widgetStartDateTime = mount->getStartDateTime();
        widgetStatus = mount->getStatus();

      } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
        widgetStartDateTime = stream->getStartDateTime();
        widgetStatus = stream->getStatus();
      }

      if (mJobsSort == "byDate") {
        if (j == widgetsCount - 4) {
          move = j;
          dt = widgetStartDateTime;
        } else {
          if (mJobsTimeSortOrder) {
            if (dt > widgetStartDateTime) {
              move = j;
              dt = widgetStartDateTime;
            }
          } else {
            if (dt < widgetStartDateTime) {
              move = j;
              dt = widgetStartDateTime;
            }
          }
        }
      }

      if (mJobsSort == "byStatus") {

        if (j == widgetsCount - 4) {
          move = j;
          ws = widgetStatus;
        } else {
          if (mJobsStatusSortOrder) {
            if (ws > widgetStatus) {
              move = j;
              ws = widgetStatus;
            }
          } else {
            if (ws < widgetStatus) {
              move = j;
              ws = widgetStatus;
            }
          }
        }
      }

    } // for (int j

    // move to top
    QWidget *widget = ui.jobs->itemAt(move)->widget();
    QWidget *widget_line = ui.jobs->itemAt(move + 1)->widget();
    auto line = qobject_cast<QFrame *>(widget_line);

    if (auto transfer = qobject_cast<JobWidget *>(widget)) {
      ui.jobs->removeWidget(transfer);
      ui.jobs->removeWidget(line);
      ui.jobs->insertWidget(i * 2, transfer);
      ui.jobs->insertWidget(i * 2 + 1, line);
    } else if (auto mount = qobject_cast<MountWidget *>(widget)) {
      ui.jobs->removeWidget(mount);
      ui.jobs->removeWidget(line);
      ui.jobs->insertWidget(i * 2, mount);
      ui.jobs->insertWidget(i * 2 + 1, line);
    } else if (auto stream = qobject_cast<StreamWidget *>(widget)) {
      ui.jobs->removeWidget(stream);
      ui.jobs->removeWidget(line);
      ui.jobs->insertWidget(i * 2, stream);
      ui.jobs->insertWidget(i * 2 + 1, line);
    } else {
      //
      break;
    }
  } //  for (int i

  return;
}
