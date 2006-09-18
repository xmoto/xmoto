Name "X-Moto 0.2.2"
OutFile "xmoto-0.2.2-win32-setup.exe"
InstallDir $PROGRAMFILES\XMoto
InstallDirRegKey HKLM "Software\XMoto" "Install_Dir"

BGGradient 000000 800000 FFFFFF
InstallColors FF8080 000030
XPStyle on

ChangeUI all "${NSISDIR}\Contrib\UIs\default.exe"

LicenseData "..\COPYING.txt"

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instFiles

Section "Game (required)"

	SectionIn RO
	SetOutPath $INSTDIR
	
	File "msvcr71.dll"
	File "msvcp71.dll"
	File "jpeg6b.dll"
	File "libpng13.dll"
	File "zlib1.dll"
	File "lua5.1.dll"
	File "libcurl.dll"
	File "SDL.dll"
	File "SDL_mixer.dll"
	File "ogg.dll"
	File "vorbis.dll"
	File "vorbisfile.dll"
	File "xmoto.bin"
	File "xmoto.exe"
	File "xmoto.ogg"
	File "fonts.dat"
	
	File "/oname=README.txt" "..\README.txt"
	File "/oname=COPYING.txt" "..\COPYING.txt"
	File "/oname=ChangeLog.txt" "..\ChangeLog.txt"
	
	CreateDirectory "$INSTDIR\Replays"
	CreateDirectory "$INSTDIR\Levels"
	CreateDirectory "$INSTDIR\Textures"
	CreateDirectory "$INSTDIR\LCache"

	WriteRegStr HKLM SOFTWARE\XMoto "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto" "DisplayName" "X-Moto"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto" "NoRepair" 1
	WriteUninstaller "uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts"

	CreateDirectory "$SMPROGRAMS\X-Moto"
	CreateShortCut "$SMPROGRAMS\X-Moto\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
	CreateShortCut "$SMPROGRAMS\X-Moto\X-Moto.lnk" "$INSTDIR\xmoto.exe" "" "$INSTDIR\xmoto.exe" 0
	CreateShortCut "$SMPROGRAMS\X-Moto\X-Moto (Windowed).lnk" "$INSTDIR\xmoto.exe" "-win" "$INSTDIR\xmoto.exe" 0
 
SectionEnd

Section "Level Editor"

	SetOutPath $INSTDIR
	
	File "editor.dat"
	File "xmoto-edit.exe"

SectionEnd

Section "Uninstall"
  
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\XMoto"
	DeleteRegKey HKLM SOFTWARE\XMoto

	RMDir /r "$SMPROGRAMS\X-Moto"
  	RMDir /r "$INSTDIR"

SectionEnd
