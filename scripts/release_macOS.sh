#!/bin/sh

set -e

# Homebrew installs Qt 6 under /opt/homebrew on Apple Silicon and
# /usr/local on Intel. Override QTDIR to point at another Qt 6 kit if needed.
if [ -d /opt/homebrew/opt/qt ]; then
  QTDIR=/opt/homebrew/opt/qt
else
  QTDIR=/usr/local/opt/qt
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"/..
VERSION=$(cat "$ROOT"/VERSION)-$(git rev-parse --short HEAD)
BUILD="$ROOT"/build
TARGET=rclone-browser-$VERSION-macos
DMG=rclone-browser-$VERSION-macos
APP="$TARGET"/"Rclone Browser.app"

# clean from previous builds (if for the same version in releases)
if [ -d "$BUILD" ]; then
  rm -rf "$BUILD"
fi
if [ -d "$ROOT"/release/"$TARGET" ]; then
  rm -rf "$ROOT"/release/"$TARGET"*
fi
if [ -f "$ROOT"/release/"$DMG".dmg ]; then
  rm "$ROOT"/release/"$DMG".dmg
fi
if [ -d "$ROOT"/release/"Rclone Browser.app" ]; then
  rm -rf "$ROOT"/release/"Rclone Browser.app"
fi



mkdir -p "$BUILD"
cd "$BUILD"
# brew install cmake qt
cmake .. -DCMAKE_PREFIX_PATH="$QTDIR" -DCMAKE_BUILD_TYPE=Release
# brew install coreutils
make --jobs=$(nproc --all)
cd build
"$QTDIR"/bin/macdeployqt rclone-browser.app -executable="rclone-browser.app/Contents/MacOS/rclone-browser"
cd ../..


mkdir -p release
cd release
mkdir "$TARGET"
cp -R "$BUILD"/build/rclone-browser.app "$APP"
cp "$ROOT"/README.md "$APP"/Readme.md
cp "$ROOT"/CHANGELOG.md "$APP"/Changelog.md
cp "$ROOT"/LICENSE "$APP"/License.txt
mv "$APP"/Contents/MacOS/rclone-browser "$APP"/Contents/MacOS/"Rclone Browser"

sed -i .bak 's/rclone-browser/Rclone Browser/g' "$APP"/Contents/Info.plist
rm "$APP"/Contents/*.bak

cat >"$APP"/Contents/MacOS/qt.conf <<EOF
[Paths]
Plugins = Plugins
EOF

echo
echo "Preparing zip file"
# brew install p7zip
7za a -mx=9 -r -tzip "$TARGET".zip "$TARGET"

## gpg --detach-sign "$TARGET".zip.sig "$TARGET".zip

echo
echo "Preparing dmg file"
# brew install node && npm install -g appdmg
# https://github.com/LinusU/node-appdmg
cp -R "$TARGET"/"Rclone Browser.app" .
cd ../scripts
appdmg ../assets/appdmg.json ../release/"$DMG".dmg
cd ../release
rm -rf "Rclone Browser.app"
