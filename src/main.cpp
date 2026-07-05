#include "main_window.h"
#include "utils.h"

#ifdef Q_OS_WIN
// required by messageHandler
#include "stdio.h"
#endif

#ifdef Q_OS_MACOS
extern void qt_set_sequence_auto_mnemonic(bool b);
#endif

#ifdef Q_OS_WIN
// redirect debug to stderr so we can run "app.exe > log.txt 2>&1" on Windows
void messageHandler(QtMsgType type, const QMessageLogContext &context,
                    const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  const char *file = context.file ? context.file : "";
  const char *function = context.function ? context.function : "";
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file,
            context.line, function);
    break;
  case QtInfoMsg:
    fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file,
            context.line, function);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file,
            context.line, function);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file,
            context.line, function);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file,
            context.line, function);
    break;
  }
}
#endif

int main(int argc, char *argv[]) {

#ifdef Q_OS_WIN
  // redirect debug to stderr so we can run "app.exe > log.txt 2>&1" on Windows
  QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
  qInstallMessageHandler(messageHandler);
#endif

  // set locale to UK english
  // would be great to let Qt manage it but it leads to issue like this one:
  // https://github.com/kapitainsky/RcloneBrowser/issues/96
  // maybe one day somebody looks into localizing RB and solves this better
  // From now on chaps - we speak english only
  QLocale l(QLocale::English, QLocale::UnitedKingdom);
  QLocale::setDefault(l);

#ifdef Q_OS_MACOS
  qt_set_sequence_auto_mnemonic(true);
#endif

// Qt 6 always enables high-DPI scaling, so these attributes are only needed
// (and only exist) on Qt 5.
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) &&                                  \
    QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
  if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO) &&
      !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") &&
      !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") &&
      !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  }
#endif

  QApplication app(argc, argv);

  //  app.setApplicationDisplayName("Rclone Browser");
  app.setApplicationName("rclone-browser");
  app.setOrganizationName("rclone-browser");
  app.setWindowIcon(QIcon(":/icons/icon.png"));

// initialize SSL libraries
// see: https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues/57
#if defined(Q_OS_LINUX)
  QString currentDir = QDir::currentPath();
  QDir::setCurrent(QCoreApplication::applicationDirPath());
  QSslSocket::supportsSsl();
  QDir::setCurrent(currentDir);
#endif

  auto settings = GetSettings();

  // initialize proxy settings
  if (!(settings->contains("Settings/useProxy"))) {
    settings->setValue("Settings/useProxy", "false");
  };
  if (!(settings->contains("Settings/http_proxy"))) {
    settings->setValue("Settings/http_proxy", "");
  };
  if (!(settings->contains("Settings/https_proxy"))) {
    settings->setValue("Settings/https_proxy", "");
  };
  if (!(settings->contains("Settings/no_proxy"))) {
    settings->setValue("Settings/no_proxy", "");
  };

  if (settings->value("Settings/useProxy").toBool()) {
    qputenv("HTTP_PROXY", settings->value("Settings/http_proxy").toByteArray());
    qputenv("http_proxy", settings->value("Settings/http_proxy").toByteArray());
    qputenv("HTTPS_PROXY",
            settings->value("Settings/https_proxy").toByteArray());
    qputenv("https_proxy",
            settings->value("Settings/https_proxy").toByteArray());
    qputenv("NO_PROXY", settings->value("Settings/no_proxy").toByteArray());
    qputenv("no_proxy", settings->value("Settings/no_proxy").toByteArray());
  }

  // remmber darkMode state on app startup
  // during first run the darkModeIni key might not exist
  if (!(settings->contains("Settings/darkModeIni"))) {
    // if darkModeIni does not exist create new key
    settings->setValue("Settings/darkModeIni", "true");
  };

  // during first run the darkMode key might not exist
  if (!(settings->contains("Settings/darkMode"))) {
    // if darkMode does not exist create new key
    settings->setValue("Settings/darkMode", "true");
  };

  bool darkMode = settings->value("Settings/darkMode").toBool();

  settings->setValue("Settings/darkModeIni", darkMode);

  // during first run the iconSize key might not exist
  if (!(settings->contains("Settings/iconSize"))) {
    // if iconSize does not exist create new key
    settings->setValue("Settings/iconSize", "M");
  };

  // during first run the iconsLayout key might not exist
  if (!(settings->contains("Settings/iconsLayout"))) {
    // if iconsLayout does not exist create new key
    settings->setValue("Settings/iconsLayout", "tiles");
  };

  // during first run the iconsColour key might not exist
  if (!(settings->contains("Settings/iconsColour"))) {
    // if iconsColour does not exist create new key
    settings->setValue("Settings/iconsColour", "black");
  };

  // during first run the buttonStyle key might not exist
  if (!(settings->contains("Settings/buttonStyle"))) {
    // if buttonstyle does not exist create new key
    settings->setValue("Settings/buttonStyle", "icononly");
  };

  // during first run the fontSize key might not exist
  if (!(settings->contains("Settings/fontSize"))) {
    // if fontSize does not exist create new key
#ifdef Q_OS_WIN
    // on Windows Fussion mode uses too small fonts
    settings->setValue("Settings/fontSize", "1");
#else
    settings->setValue("Settings/fontSize", "0");
#endif
  };

  // during first run the buttonSize key might not exist
  if (!(settings->contains("Settings/buttonSize"))) {
    // if buttonSize does not exist create new key
    settings->setValue("Settings/buttonSize", "0");
  };

  // during first run the sortTask key might not exist
  if (!(settings->contains("Settings/sortTask"))) {
    // if sortTask does not exist create new key
    settings->setValue("Settings/sortTask", "false");
  };

  // during first run the remoteMode key might not exist
  if (!(settings->contains("Settings/remoteMode"))) {
    // if remoteMode does not exist create new key
    settings->setValue("Settings/remoteMode", "main");
  };

  // during first run the remoteType key might not exist
  if (!(settings->contains("Settings/remoteType"))) {
    // if remoteType does not exist create new key
    settings->setValue("Settings/remoteType", "main");
  };

  if (!(settings->contains("Settings/soundNotif"))) {
    settings->setValue("Settings/soundNotif", "false");
  };

  // true - scheduler runing, false - scheduler not runing
  if (!(settings->contains("Settings/schedulerStatus"))) {
    settings->setValue("Settings/schedulerStatus", "true");
  };

  // true - queue runing, false - queue not runing
  if (!(settings->contains("Settings/queueStatus"))) {
    settings->setValue("Settings/queueStatus", "true");
  };

  if (!(settings->contains("Settings/startMinimisedToTray"))) {
    settings->setValue("Settings/startMinimisedToTray", "false");
  };

  if (!(settings->contains("Settings/transferAutoName"))) {
    settings->setValue("Settings/transferAutoName", "false");
  };

  if (!(settings->contains("Settings/transferAddToQueue"))) {
    settings->setValue("Settings/transferAddToQueue", "false");
  };

  // preemptive content loading on/off
  if (!(settings->contains("Settings/preemptiveLoading"))) {
    settings->setValue("Settings/preemptiveLoading", "true");
  };

  // preemptive content loading level (0,1,2)
  if (!(settings->contains("Settings/preemptiveLoadingLevel"))) {
    settings->setValue("Settings/preemptiveLoadingLevel", "0");
  }

  // during first run the queueScript key might not exist
  if (!(settings->contains("Settings/queueScript"))) {
    settings->setValue("Settings/queueScript", "");
  };

  // script to run when transfer jobs start
  if (!(settings->contains("Settings/transferOnScript"))) {
    settings->setValue("Settings/transferOnScript", "");
  };

  // script to run when last transfer jobs finished
  if (!(settings->contains("Settings/transferOffScript"))) {
    settings->setValue("Settings/transferOffScript", "");
  };

  // during first run the queueScriptRun key might not exist
  if (!(settings->contains("Settings/queueScriptRun"))) {
    settings->setValue("Settings/queueScriptRun", "false");
  };

  if (!(settings->contains("Settings/jobStartScriptRun"))) {
    settings->setValue("Settings/jobStartScriptRun", "false");
  };

  if (!(settings->contains("Settings/jobLastFinishedScriptRun"))) {
    settings->setValue("Settings/jobLastFinishedScriptRun", "false");
  };

  // remember and re-use last transfer options
  if (!(settings->contains("Settings/rememberLastOptions"))) {
    settings->setValue("Settings/rememberLastOptions", "true");
  };

  // use ports (49152-65535) -
  // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
  // during first run the rcPortStartWin key might not exist
  if (!(settings->contains("Settings/rcPortStartWin"))) {
    // if rcPortStartWin does not exist create new key
    settings->setValue("Settings/rcPortStartWin", "49700");
  };

  // set application font size
  int fontsize = 0;
  fontsize = (settings->value("Settings/fontSize").toInt());

  QFont defaultFont = QApplication::font();
  defaultFont.setPointSize(defaultFont.pointSize() + fontsize);
  qApp->setFont(defaultFont);

  // enforce one instance of Rclone Browser per user
  QString tmpDir;
  QString applicationNameBase;
  QFileInfo applicationPath;
  QFileInfo applicationUserPath;

  // QString xdg_config_home = qgetenv("XDG_CONFIG_HOME");
  // qDebug() << QString("main.cpp $XDG_CONFIG_HOME: " + xdg_config_home);

  // QString APPIMAGE = qgetenv("APPIMAGE");
  // qDebug() << QString("main.cpp $APPIMAGE: " + APPIMAGE);

  QFileInfo appBundlePath;

  if (IsPortableMode()) {

    //  qDebug() << QString("isPortable is true");
    //  applicationPath = qApp->applicationFilePath();
#ifdef Q_OS_MACOS
    // on macOS excecutable file is located in
    // ./rclone-browser.app/Contents/MasOS/
    // to get actual bundle folder we have
    // to traverse three levels up
    applicationPath = qApp->applicationFilePath();
    tmpDir = applicationPath.absolutePath() + "/../../..";

    // get bundle name
    QFileInfo MacOSPath = applicationPath.dir().path();
    QFileInfo ContentsPath = MacOSPath.dir().path();
    appBundlePath = ContentsPath.dir().path();

#else
    // not macOS
#ifdef Q_OS_WIN
    applicationPath = qApp->applicationFilePath();
    tmpDir = applicationPath.absolutePath();
#else
    QString xdg_config_home = qgetenv("XDG_CONFIG_HOME");
    tmpDir = xdg_config_home + "/rclone-browser";
    // create ./rclone-browser folder
    if (!QDir(tmpDir).exists()) {
      QDir().mkdir(tmpDir);
    }
#endif
#endif
  } else {
    // not portable mode
    // get tmp folder from Qt  - OS dependend
    tmpDir = QDir::tempPath();
  }

  // check if tmpDir writable
  // as isWritable does weird things on Windows
  // we do this old fashioned way by creating temp file
  QTemporaryFile tempfile(tmpDir + "/rclone-browserXXXXXX.test");

  if (tempfile.open()) {
    tempfile.close();
    tempfile.remove();
  } else {
    // folder has no write access
    if (IsPortableMode()) {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText("You need write "
                     "access to this folder:\n\n"
#ifdef Q_OS_MACOS
                     + appBundlePath.absolutePath() +
#else
#ifdef Q_OS_WIN
                     + tmpDir +
#else
                     + tmpDir.left(tmpDir.length() - 15) +
#endif
#endif
                     "\n\n"
#ifdef Q_OS_MACOS
                     "Or remove file:\n\n" +
                     appBundlePath.baseName() +
                     ".ini \n\nfrom the above folder "
#else
#ifdef Q_OS_WIN
                     "Or remove file:\n\n" +
                     applicationPath.baseName() +
                     ".ini \n\nfrom the above folder "
#else
                     "Or remove folder:\n\n" +
                     tmpDir.left(tmpDir.length() - 15) +
                     "\n\n"
#endif
#endif
                     "to disable portable mode.");
      msgBox.exec();
    } else {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText("You need write "
                     "access to this folder: \n\n"
#ifdef Q_OS_MACOS
                     + tmpDir
#else

#ifdef Q_OS_WIN
                     + tmpDir
#else

                     + tmpDir.left(tmpDir.length() - 15)
#endif
#endif
      );
      msgBox.exec();
    }
    return static_cast<int>(
        0x80004004); // exit immediately if folder not writable
  }

  // qDebug() << QString("main.cpp tmpDir:  " + tmpDir);

  // not most elegant as fixed name but in reality not big deal
  QLockFile lockFile(tmpDir + "/.RcloneBrowser_4q6RgLs2RpbJA.lock");

  if (!lockFile.tryLock(100)) {
    // if already running display warning and quit
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Rclone Browser is already running."
                   "\r\n\nOnly one instance is allowed.");
    msgBox.exec();
    return static_cast<int>(
        0x80004004); // exit immediately if another instance is running
  }

  MainWindow w;
  w.show();

  return app.exec();
}
