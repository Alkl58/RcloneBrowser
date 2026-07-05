#include "check_dialog.h"
#include "utils.h"

CheckDialog::CheckDialog(const QString &remote, const QDir &path,
                         const QString &remoteType, QWidget *parent)
    : QDialog(parent) {

  ui.setupUi(this);
  //  resize(0, 0);

  adjustSize();
  setMaximumHeight(this->height());
  setMinimumHeight(this->height());

  setWindowTitle("Check integrity");

  mTarget = remote + ":" + path.path();

  ui.remoteToCheck->setText(mTarget);
  ui.remoteToCheck->setCursorPosition(0);

  if (remoteType == "crypt") {
    ui.check->setDisabled(true);
    ui.cryptcheck->setChecked(true);

    ui.download->setDisabled(true);
    ui.sizeOnly->setDisabled(true);

  } else {
    ui.cryptcheck->setDisabled(true);
    ui.check->setChecked(true);
  }

  QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, this,
                   &QDialog::accept);

  QObject::connect(ui.buttonBox, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);

  QObject::connect(ui.download, &QCheckBox::clicked, this, [=]() {
    if (ui.download->isChecked()) {
      ui.sizeOnly->setChecked(false);
      ui.sizeOnly->setDisabled(true);
    } else {
    }
    ui.sizeOnly->setDisabled(false);
  });

  QObject::connect(ui.sizeOnly, &QCheckBox::clicked, this, [=]() {
    if (ui.sizeOnly->isChecked()) {
      ui.download->setChecked(false);
      ui.download->setDisabled(true);
    } else {
    }
    ui.download->setDisabled(false);
  });

  QObject::connect(ui.sourceDirBrowse, &QToolButton::clicked, this, [=]() {
    QString sourceDir =
        QFileDialog::getExistingDirectory(this, "Choose source directory");
    if (!sourceDir.isEmpty()) {
      ui.sourceDir->setText(QDir::toNativeSeparators(sourceDir));
      ui.sourceDir->setCursorPosition(0);
    }
  });
}

CheckDialog::~CheckDialog() {}

QString CheckDialog::getSource() const { return ui.sourceDir->text(); }

bool CheckDialog::isCheck() const { return ui.check->isChecked(); }

QStringList CheckDialog::getOptions() const {

  QStringList list;

  if (ui.cryptcheck->isChecked()) {
    list << "cryptcheck";
  }

  if (ui.check->isChecked()) {
    list << "check";
  }

  if (ui.oneWay->isChecked()) {
    list << "--one-way";
  }

  if (ui.sizeOnly->isChecked()) {
    list << "--size-only";
  }

  if (ui.download->isChecked()) {
    list << "--download";
  }

  QString extra = ui.textExtra->text().trimmed();
  if (!extra.isEmpty()) {
    for (QString arg :
         extra.split(QRegularExpression(" (?=[^\"]*(\"[^\"]*\"[^\"]*)*$)"))) {
      if (!arg.isEmpty()) {
        list << arg.replace("\"", "");
      }
    }
  }

  list << getSource();

  list << mTarget;

  return list;
}

void CheckDialog::done(int r) {
  if (r == QDialog::Accepted) {
    if (ui.sourceDir->text().isEmpty()) {
      QMessageBox::warning(this, "Warning", "Please enter source directory!");
      return;
    }
  }
  QDialog::done(r);
}
