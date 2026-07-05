#include "job_options.h"
#include "utils.h"
#include <qexception.h>
#include <qlogging.h>
#ifdef _WIN32
#pragma warning(disable : 4505)
#endif
JobOptions::JobOptions(bool isDownload) : JobOptions() {
  setJobType(isDownload);
  uniqueId = QUuid::createUuid();
}

JobOptions::JobOptions()
    : jobType(UnknownJobType), operation(UnknownOp), dryRun(false), sync(false),
      syncTiming(UnknownTiming), skipNewer(false), skipExisting(false),
      compare(false), compareOption(), verbose(false), sameFilesystem(false),
      dontUpdateModified(false), maxDepth(0), deleteExcluded(false),
      isFolder(false), DriveSharedWithMe(false), mountReadOnly(false),
      mountCacheLevel(UnknownCacheLevel), mountAutoStart(false),
      mountWinDriveMode(false), noTraverse(false), createEmptySrcDirs(false),
      deleteEmptySrcDirs(false) {}

const qint32 JobOptions::classVersion = 8;

JobOptions::~JobOptions() {}

/*
 * Turn the options held here into a string list for
 * use in the rclone command.
 *
 * This logic was originally in transfer_dialog.cpp.
 *
 * This needs to change whenever e.g. new options are
 * added to the dialog.
 */
QStringList JobOptions::getOptions() const {
  QStringList list;

  if (operation == Copy) {
    list << "copy";
  } else if (operation == Move) {
    list << "move";
  } else if (operation == Sync) {
    list << "sync";
  }

  list << source;
  list << dest;

  if (!GetDefaultOptionsList("defaultRcloneOptions").isEmpty()) {
    list << GetDefaultOptionsList("defaultRcloneOptions");
  }

  if (jobType == JobOptions::JobType::Download) {
    if (!GetDefaultOptionsList("defaultDownloadOptions").isEmpty()) {
      list << GetDefaultOptionsList("defaultDownloadOptions");
    }
  }

  if (jobType == JobOptions::JobType::Upload) {
    if (!GetDefaultOptionsList("defaultUploadOptions").isEmpty()) {
      list << GetDefaultOptionsList("defaultUploadOptions");
    }
  }

  if (sync) {
    switch (syncTiming) {
    case During:
      list << "--delete-during";
      break;
    case After:
      list << "--delete-after";
      break;
    case Before:
      list << "--delete-before";
      break;
    default:
      break;
      ;
    }
  }

  if (skipNewer) {
    list << "--update";
  }
  if (skipExisting) {
    list << "--ignore-existing";
  }

  if (compare) {
    switch (compareOption) {
    case Checksum:
      list << "--checksum";
      break;
    case IgnoreSize:
      list << "--ignore-size";
      break;
    case SizeOnly:
      list << "--size-only";
      break;
    case ChecksumIgnoreSize:
      list << "--checksum"
           << "--ignore-size";
      break;
    default:
      break;
    }
  }

  if (sameFilesystem) {
    list << "--one-file-system";
  }

  if (dontUpdateModified) {
    list << "--no-update-modtime";
  }

  if (noTraverse) {
    list << "--no-traverse";
  }

  if (createEmptySrcDirs) {
    list << "--create-empty-src-dirs";
  }

  if (deleteEmptySrcDirs) {
    list << "--delete-empty-src-dirs";
  }

  list << "--transfers" << transfers;
  list << "--checkers" << checkers;

  if (!bandwidth.isEmpty()) {
    list << "--bwlimit" << bandwidth;
  }
  if (!minSize.isEmpty()) {
    list << "--min-size" << minSize;
  }
  if (!minAge.isEmpty()) {
    list << "--min-age" << minAge;
  }
  if (!maxAge.isEmpty()) {
    list << "--max-age" << maxAge;
  }

  if (maxDepth != 0) {
    list << "--max-depth" << QString::number(maxDepth);
  }

  list << "--contimeout" << (connectTimeout + "s");
  list << "--timeout" << (idleTimeout + "s");
  list << "--retries" << retries;
  list << "--low-level-retries" << lowLevelRetries;

  if (deleteExcluded) {
    list << "--delete-excluded";
  }

  if (!extra.isEmpty()) {

    for (auto line : extra.split('\n')) {
      // split on spaces but not if inside quotes e.g. --option-1
      // --option-2="arg1 arg2" --option-3 arg3 should generate "--option-1"
      // "--option-2=\"arg1 arg2\"" "--option-3" "arg3"
      for (QString arg :
           line.split(QRegularExpression(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
        if (!arg.isEmpty()) {
          list << arg.replace("\"", "");
        }
      }
    }
  }

  if (!included.isEmpty()) {
    for (auto line : included.split('\n')) {
      list << "--include" << line;
    }
  }

  // excluded after included and extra options as they can also contain included
  if (!excluded.isEmpty()) {
    for (auto line : excluded.split('\n')) {
      list << "--exclude" << line;
    }
  }

  if (!filtered.isEmpty()) {
    for (auto line : filtered.split('\n')) {
      list << "--filter" << line;
    }
  }

  // get Google Drive mode option
  if (remoteType == "drive") {
    if (remoteMode == "shared") {
      list << "--drive-shared-with-me";
    } else {
      if (remoteMode == "trash") {
        list << "--drive-trashed-only";
      } else {
        if (remoteMode == "main") {
        } else {
          // older tasks dont't have googleDriveMode
          // and value from DriveSharedWithMe has to be used
          if (DriveSharedWithMe) {
            list << "--drive-shared-with-me";
          }
        }
      }
    }
  }

  // always verbose
  list << "--verbose";

  list << "--stats"
       << "1s";

  list << "--stats-file-name-length"
       << "0";

  if (dryRun) {
    list << "--dry-run";
  }

  return list;
}

SerializationException::SerializationException(QString msg)
    : QException(), Message(msg) {}
