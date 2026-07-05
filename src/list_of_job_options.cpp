#include "list_of_job_options.h"
#include <QDataStream>
#include <qdir.h>
#include <qlogging.h>
#include <qstandardpaths.h>
#include <utils.h>

static QDataStream &operator>>(QDataStream &dataStream, JobOptions &jo);
static QDataStream &operator<<(QDataStream &dataStream, JobOptions &jo);
static QDataStream &operator>>(QDataStream &in, JobOptions::Operation &e);
static QDataStream &operator>>(QDataStream &in, JobOptions::SyncTiming &e);
static QDataStream &operator>>(QDataStream &in, JobOptions::CompareOption &e);
static QDataStream &operator>>(QDataStream &in, JobOptions::JobType &e);
static QDataStream &operator>>(QDataStream &in, JobOptions::MountCacheLevel &e);

ListOfJobOptions *ListOfJobOptions::SavedJobOptions = nullptr;
const QString ListOfJobOptions::persistenceFileName = "tasks.bin";

ListOfJobOptions::ListOfJobOptions() {}

ListOfJobOptions *ListOfJobOptions::getInstance() {
  if (SavedJobOptions == nullptr) {
    SavedJobOptions = new ListOfJobOptions();
    RestoreFromUserData(*SavedJobOptions);
  }
  return SavedJobOptions;
}

bool ListOfJobOptions::Persist(JobOptions *jo) {
  bool isNew = !this->tasks.contains(jo);
  if (isNew)
    this->tasks.append(jo);
  else {
    //    int ix = tasks.indexOf(jo);
    //    JobOptions *old = tasks[ix];
    //    qDebug() << QString("old [%1] New [%2]")
    //                    .arg(old->description)
    //                    .arg(jo->description);
  }
  PersistToUserData();
  return isNew;
}

bool ListOfJobOptions::Forget(JobOptions *jo) {
  bool isKnown = this->tasks.contains(jo);
  if (!isKnown)
    return false;
  int ix = tasks.indexOf(jo);
  tasks.removeAt(ix);
  //  qDebug() << QString("removed [%1]").arg(jo->description);
  PersistToUserData();
  return isKnown;
}

QString ListOfJobOptions::GetPersistenceFilePath() {

  QDir outputDir;

  if (IsPortableMode()) {
    // in portable mode tasks' file will be saved in the same folder as
    // excecutable
#ifdef Q_OS_MACOS
    // on macOS excecutable file is located in
    // ./rclone-browser.app/Contents/MasOS/
    // to get actual bundle folder we have
    // to traverse three levels up
    outputDir = QDir(qApp->applicationDirPath() + "/../../..");
#else
#ifdef Q_OS_WIN
    // not macOS
    outputDir = QDir(qApp->applicationDirPath());
#else
    QString xdg_config_home = qgetenv("XDG_CONFIG_HOME");
    outputDir = QDir(xdg_config_home + "/rclone-browser");
#endif
#endif

  } else {

    // get data location folder from Qt  - OS dependend
    outputDir = QDir(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  }

  // make sure the destination folder exists, otherwise saving would fail
  if (!outputDir.exists()) {
    if (!outputDir.mkpath(".")) {
      qWarning() << "Could not create tasks folder:" << outputDir.absolutePath();
      return QString();
    }
  }

  return outputDir.absoluteFilePath(persistenceFileName);
}

QFile *ListOfJobOptions::GetPersistenceFile(QIODevice::OpenModeFlag mode) {

  QString filePath = GetPersistenceFilePath();
  if (filePath.isEmpty())
    return nullptr;

  QFile *file = new QFile(filePath);

  if (!file->open(mode)) {
    //    qDebug() << QString("Could not open ") << file->fileName();
    delete file;
    file = nullptr;
  }
  return file;
}

bool ListOfJobOptions::RestoreFromUserData(ListOfJobOptions &dataIn) {
  QFile *file = GetPersistenceFile(QIODevice::ReadOnly);
  if (file == nullptr)
    return false;
  QDataStream instream(file);
  instream.setVersion(QDataStream::Qt_5_2);

  while (!instream.atEnd()) {
    JobOptions *jo = new JobOptions();
    try {
      instream >> *jo;
      dataIn.tasks.append(jo);
    } catch (SerializationException &ex) {
      //      qDebug() << QString("failed to restore tasks: ") << ex.Message;
      // avoid leaking the partially-read object that failed to deserialize
      delete jo;
      file->close();
      delete file;
      return false;
    }
  }

  file->close();
  delete file;

  return true;
}

bool ListOfJobOptions::PersistToUserData() {
  // Determine the destination path directly (creating the folder if needed)
  // instead of opening the existing file. Previously this opened the file in
  // ReadOnly mode just to read its path, which failed when the file did not
  // exist yet - meaning tasks/queues were never saved on a fresh install.
  QString filePath = GetPersistenceFilePath();

  if (filePath.isEmpty()) {
    qWarning() << "Could not determine tasks file location; tasks not saved.";
    return false;
  }

  // QSaveFile writes to a temporary file and atomically replaces the target on
  // commit(), creating it if it does not exist yet.
  QSaveFile fileToSave(filePath);

  // note this mode implies Truncate also
  if (!fileToSave.open(QIODevice::WriteOnly)) {
    qWarning() << "Could not open tasks file for writing:" << filePath << "-"
               << fileToSave.errorString();
    return false;
  }

  QDataStream outstream(&fileToSave);

  outstream.setVersion(QDataStream::Qt_5_2);
  for (JobOptions *it : tasks) {
    outstream << *it;
  }

  if (!fileToSave.commit()) {
    qWarning() << "Could not save tasks file:" << filePath << "-"
               << fileToSave.errorString();
    return false;
  }

  emit tasksListUpdated();

  return true;
}

QDataStream &operator<<(QDataStream &stream, JobOptions &jo) {
  stream << jo.myName() << JobOptions::classVersion << jo.description
         << jo.jobType << jo.operation << /* jo.dryRun <<*/ jo.sync
         << jo.syncTiming << jo.skipNewer << jo.skipExisting << jo.compare
         << jo.compareOption << jo.verbose << jo.sameFilesystem
         << jo.dontUpdateModified << jo.transfers << jo.checkers << jo.bandwidth
         << jo.minSize << jo.minAge << jo.maxAge << jo.maxDepth
         << jo.connectTimeout << jo.idleTimeout << jo.retries
         << jo.lowLevelRetries << jo.deleteExcluded << jo.excluded << jo.extra
         << jo.DriveSharedWithMe << jo.source << jo.dest << jo.isFolder
         << jo.uniqueId << jo.remoteMode << jo.remoteType << jo.mountReadOnly
         << jo.mountCacheLevel << jo.mountVolume << jo.mountAutoStart
         << jo.mountRcPort << jo.mountScript << jo.mountWinDriveMode
         << jo.included << jo.noTraverse << jo.createEmptySrcDirs << jo.filtered
         << jo.deleteEmptySrcDirs;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, JobOptions &jo) {
  QString actualName;
  qint32 actualVersion;

  stream >> actualName;
  if (QString::compare(actualName, jo.myName()) != 0)
    throw SerializationException("incorrect class");

  stream >> actualVersion;
  if (actualVersion > JobOptions::classVersion)
    throw SerializationException("stored version is newer");

  stream >> jo.description >> jo.jobType >> jo.operation >>
      /* jo.dryRun >> */ jo.sync >> jo.syncTiming >> jo.skipNewer >>
      jo.skipExisting >> jo.compare >> jo.compareOption >> jo.verbose >>
      jo.sameFilesystem >> jo.dontUpdateModified >> jo.transfers >>
      jo.checkers >> jo.bandwidth >> jo.minSize >> jo.minAge >> jo.maxAge >>
      jo.maxDepth >> jo.connectTimeout >> jo.idleTimeout >> jo.retries >>
      jo.lowLevelRetries >> jo.deleteExcluded >> jo.excluded >> jo.extra >>
      jo.DriveSharedWithMe >> jo.source >> jo.dest;

  // as fields are added in later revisions, check actualVersion here and
  // conditionally extract any new fields iff they are expected based on the
  // stream value
  if (actualVersion >= 2) {
    stream >> jo.isFolder;
  }

  if (actualVersion >= 3) {
    stream >> jo.uniqueId;
  }
  if (actualVersion >= 4) {
    stream >> jo.remoteMode;
    stream >> jo.remoteType;
  }

  if (actualVersion >= 5) {
    stream >> jo.mountReadOnly;
    stream >> jo.mountCacheLevel;
    stream >> jo.mountVolume;
    stream >> jo.mountAutoStart;
    stream >> jo.mountRcPort;
    stream >> jo.mountScript;
    stream >> jo.mountWinDriveMode;
  }

  if (actualVersion >= 6) {
    stream >> jo.included;
  }

  if (actualVersion >= 7) {
    stream >> jo.noTraverse;
    stream >> jo.createEmptySrcDirs;
  }

  if (actualVersion >= 8) {
    stream >> jo.filtered;
    stream >> jo.deleteEmptySrcDirs;
  }

  return stream;
}

QDataStream &operator>>(QDataStream &in, JobOptions::Operation &e) {
  in >> (quint32 &)e;
  return in;
}

QDataStream &operator>>(QDataStream &in, JobOptions::SyncTiming &e) {
  in >> (quint32 &)e;
  return in;
}

QDataStream &operator>>(QDataStream &in, JobOptions::CompareOption &e) {
  in >> (quint32 &)e;
  return in;
}

QDataStream &operator>>(QDataStream &in, JobOptions::JobType &e) {
  in >> (quint32 &)e;
  return in;
}

QDataStream &operator>>(QDataStream &in, JobOptions::MountCacheLevel &e) {
  in >> (quint32 &)e;
  return in;
}
