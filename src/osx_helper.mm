#include "osx_helper.h"
#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

// Qt 6 removed QtMacExtras (and QtMac::fromCGImageRef), so convert the
// CGImageRef into a QImage manually by rendering it into a bitmap context.
static QImage qt_mac_toQImage(CGImageRef image) {
  const size_t width = CGImageGetWidth(image);
  const size_t height = CGImageGetHeight(image);
  if (width == 0 || height == 0) {
    return QImage();
  }

  QImage result(static_cast<int>(width), static_cast<int>(height),
                QImage::Format_ARGB32_Premultiplied);
  result.fill(Qt::transparent);

  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(
      result.bits(), width, height, 8, result.bytesPerLine(), colorSpace,
      kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
  if (context) {
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    CGContextRelease(context);
  }
  CGColorSpaceRelease(colorSpace);

  return result;
}

QIcon osxGetIcon(const QString &extension) {
  QIcon icon;
  @autoreleasepool {
    UTType *contentType =
        [UTType typeWithFilenameExtension:extension.toNSString()];
    if (!contentType) {
      contentType = UTTypeData;
    }

    NSImage *image =
        [[NSWorkspace sharedWorkspace] iconForContentType:contentType];
    if (!image) {
      return icon;
    }
    NSRect rect = NSMakeRect(0, 0, image.size.width, image.size.height);
    CGImageRef imageRef = [image CGImageForProposedRect:&rect
                                                context:NULL
                                                  hints:nil];
    if (imageRef) {
      QImage converted = qt_mac_toQImage(imageRef);
      if (!converted.isNull()) {
        icon = QIcon(QPixmap::fromImage(converted));
      }
    }
  }
  return icon;
}

void osxShowDockIcon() {
  ProcessSerialNumber psn = {0, kCurrentProcess};
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);
}

void osxHideDockIcon() {
  ProcessSerialNumber psn = {0, kCurrentProcess};
  TransformProcessType(&psn, kProcessTransformToUIElementApplication);
}
