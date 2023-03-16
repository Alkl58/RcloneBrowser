#include "progress_dialog.h"
#include "utils.h"

ProgressDialog::ProgressDialog(const QString &title, const QString &operation,
                               const QString &message, QProcess *process,
                               QWidget *parent, bool close, bool trim,
                               QString toolTip)
    : QDialog(parent) {

  ui.setupUi(this);

  // remove window close button
  setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint |
                 Qt::WindowMinMaxButtonsHint);

  resize(0, 0);

  setWindowTitle("Rclone Browser - " + title);

  // manually control window size to ensure resizing take into account output
  // field which can be hidden
  mMinimumWidth = minimumWidth();
  mWidth = this->width();
  mHeight = this->height();
  ui.output->setVisible(true);
  adjustSize();
  mMinimumHeight = this->height();
  ui.output->setVisible(false);
  adjustSize();

  if (!toolTip.isEmpty()) {
    QString toolTipPlacer =
        QString("<html><head/><body><p>%1</p></body></html>").arg(toolTip);
    ui.frame->setToolTip(tr(toolTipPlacer.toLocal8Bit().constData()));
  }

  ui.labelOperation->setText(operation);

  ui.labelOperation->setStyleSheet(
      "QLabel { color: green; font-weight: bold; }");

  ui.info->setText(message);
  ui.info->setCursorPosition(0);

  // ui.output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

  // apply font size preferences
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

  // icons style
  QString iconsColour = settings->value("Settings/iconsColour").toString();

  QString img_add = "";

  if (iconsColour == "white") {
    img_add = "_inv";
  }

  ui.output->setVisible(false);

  // set default arrow
  ui.buttonShowOutput->setIcon(
      QIcon(":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));
  ui.buttonShowOutput->setIconSize(QSize(24, 24));

  QPushButton *cancelButton =
      ui.buttonBox->addButton("&Cancel", QDialogButtonBox::RejectRole);

  QObject::connect(cancelButton, &QPushButton::clicked, this, [=]() {
    if (mIsRunning) {
      process->kill();
    }
  });

  QObject::connect(ui.buttonBox, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);

  QObject::connect(
      ui.buttonShowOutput, &QPushButton::toggled, this, [=](bool checked) {
        ui.output->setVisible(checked);

        if (checked) {
          ui.buttonShowOutput->setIcon(QIcon(
              ":media/images/qbutton_icons/vdownarrow" + img_add + ".png"));
          ui.buttonShowOutput->setIconSize(QSize(24, 24));

          mWidth = this->width();
          setMinimumWidth(mWidth);
          setMaximumWidth(mWidth);
          adjustSize();
          setMinimumHeight(mHeight);
          setMaximumHeight(mHeight);
          setMinimumHeight(mMinimumHeight);
          setMaximumHeight(16777215);
          setMinimumWidth(mMinimumWidth);
          setMaximumWidth(16777215);
        } else {
          ui.buttonShowOutput->setIcon(QIcon(
              ":media/images/qbutton_icons/vrightarrow" + img_add + ".png"));
          ui.buttonShowOutput->setIconSize(QSize(24, 24));

          mWidth = this->width();
          mHeight = this->height();
          setMinimumHeight(0);
          setMinimumWidth(mWidth);
          adjustSize();
          setMinimumWidth(mMinimumWidth);
          //  when without output dont allow resize vertical
          int height = this->height();
          if (height != minimumHeight() || height != maximumHeight()) {
            setMinimumHeight(height);
            setMaximumHeight(height);
          }
        }
      });

  QObject::connect(process,
                   static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
                       &QProcess::finished),
                   this, [=](int code, QProcess::ExitStatus status) {
                     mIsRunning = false;
                     cancelButton->setText("&Close");

                     if (status == QProcess::NormalExit && code == 0) {

                       if (iconsColour == "white") {
                         ui.labelOperation->setStyleSheet(
                             "QLabel { font-weight: bold; }");
                       } else {
                         ui.labelOperation->setStyleSheet(
                             "QLabel { color: black; font-weight: bold; }");
                       }

                       ui.labelOperation->setText("Finished ");

                       if (close) {
                         emit accept();
                       }
                     } else {

                       ui.labelOperation->setStyleSheet(
                           "QLabel { color: red; font-weight: bold; }");
                       ui.labelOperation->setText("Error ");

                       ui.buttonShowOutput->setChecked(true);
                       ui.buttonBox->setEnabled(true);
                     }
                   });

  QObject::connect(process, &QProcess::readyRead, this, [=]() {
    QString output = process->readAll();
    if (trim) {
      output = output.trimmed();
    }
    ui.output->appendPlainText(output);

    emit outputAvailable(output);
  });

  process->setProcessChannelMode(QProcess::MergedChannels);
  process->start(QIODevice::ReadOnly);
}

ProgressDialog::~ProgressDialog() {}

void ProgressDialog::expand() { ui.buttonShowOutput->setChecked(true); }

void ProgressDialog::allowToClose() { ui.buttonBox->setEnabled(true); }
//
// QString ProgressDialog::getOutput() const
//{
//    return ui.output->toPlainText();
//}

void ProgressDialog::closeEvent(QCloseEvent *ev) {
  ev->ignore();
  return;
}
