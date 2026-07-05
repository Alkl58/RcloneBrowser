#include "mac_os_notifications.h"
#import <Foundation/NSUUID.h>
#import <UserNotifications/UserNotifications.h>

@interface RcloneBrowserNotificationDelegate
    : NSObject <UNUserNotificationCenterDelegate>
@end

@implementation RcloneBrowserNotificationDelegate
- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:
             (void (^)(UNNotificationPresentationOptions options))
                 completionHandler {
  Q_UNUSED(center);
  Q_UNUSED(notification);
  completionHandler(UNNotificationPresentationOptionBanner |
                    UNNotificationPresentationOptionSound);
}
@end

static void deliverNotification(UNUserNotificationCenter *center,
                                NSString *title, NSString *text) {
  UNMutableNotificationContent *content =
      [[UNMutableNotificationContent alloc] init];
  content.title = title;
  content.body = text;
  content.sound = [UNNotificationSound defaultSound];

  NSString *identifier = [[NSUUID UUID] UUIDString];
  UNNotificationRequest *request =
      [UNNotificationRequest requestWithIdentifier:identifier
                                           content:content
                                           trigger:nil];
  [center addNotificationRequest:request withCompletionHandler:nil];
  [content release];
}

void MacOsNotification::Display(QString title, QString text) {
  static RcloneBrowserNotificationDelegate *notificationDelegate =
      [[RcloneBrowserNotificationDelegate alloc] init];

  NSString *notificationTitle = [title.toNSString() copy];
  NSString *notificationText = [text.toNSString() copy];
  UNUserNotificationCenter *center =
      [UNUserNotificationCenter currentNotificationCenter];
  center.delegate = notificationDelegate;

  [center getNotificationSettingsWithCompletionHandler:^(
              UNNotificationSettings *settings) {
    UNAuthorizationStatus status = settings.authorizationStatus;
    if (status == UNAuthorizationStatusAuthorized ||
        status == UNAuthorizationStatusProvisional) {
      deliverNotification(center, notificationTitle, notificationText);
      [notificationTitle release];
      [notificationText release];
      return;
    }

    if (status == UNAuthorizationStatusNotDetermined) {
      [center
          requestAuthorizationWithOptions:UNAuthorizationOptionAlert |
                                          UNAuthorizationOptionSound
                       completionHandler:^(BOOL granted, NSError *error) {
                         Q_UNUSED(error);
                         if (granted) {
                           deliverNotification(center, notificationTitle,
                                               notificationText);
                         }
                         [notificationTitle release];
                         [notificationText release];
                       }];
      return;
    }

    [notificationTitle release];
    [notificationText release];
  }];
}
