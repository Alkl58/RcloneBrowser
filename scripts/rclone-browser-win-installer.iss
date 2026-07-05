
#define MyAppName "Rclone Browser"
#define MyAppPublisher "kapitainsky"
#define MyAppURL "https://github.com/kapitainsky/RcloneBrowser"
#define MyAppExeName "RcloneBrowser.exe"

[Setup]
; We will use two different IDs - for 64bit and 32bit
;64bit: AppId={{0AF9BF43-8D44-4AFF-AE60-6CECF1BF0D31}
;32bit: AppId={{5644ED3A-6028-47C0-9796-29548EF7CEA3}
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
; The [Icons] "quicklaunchicon" entry uses {userappdata} but its [Tasks] entry has a proper IsAdminInstallMode Check.
UsedUserAreasWarning=no
LicenseFile=..\release\{#MyAppDir}\License.txt
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
SetupIconFile=..\src\icon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma
SolidCompression=yes
WizardStyle=modern

#if MyAppArch=="x64"
; "ArchitecturesAllowed=x64compatible" allows Setup to run on x64 (and, under
; emulation, ARM64) machines and install in native 64-bit mode.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#endif

#if MyAppArch=="arm64"
; ARM64 build: only run and install natively on ARM64 Windows.
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#endif

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
Source: "..\release\{#MyAppDir}\RcloneBrowser.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\release\{#MyAppDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

