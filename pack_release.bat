:: 1. 一時作業フォルダの作成
mkdir _release_temp
mkdir _release_temp\circuit\KiCad_pcba\production_files
mkdir _release_temp\circuit\ver155-GHA-7-3-9
mkdir _release_temp\circuit\ver155-LC115H-N
mkdir _release_temp\firmware\osr-cdi15X.X
mkdir _release_temp\genuine-map-files
mkdir _release_temp\wincdi\InstallerProject\Release

:: 2. 指定ファイルのコピー
copy circuit\KiCad_pcba\jlcpcb\production_files\*.* _release_temp\circuit\KiCad_pcba\production_files\
copy circuit\ver155-GHA-7-3-9\*.* _release_temp\circuit\ver155-GHA-7-3-9\
copy circuit\ver155-LC115H-N\*.* _release_temp\circuit\ver155-LC115H-N\
copy firmware\osr-cdi15X.X\osr-cdi15X.X.production.hex _release_temp\firmware\osr-cdi15X.X\
copy genuine-map-files\*.* _release_temp\genuine-map-files\
copy wincdi\InstallerProject\Release\*.* _release_temp\wincdi\InstallerProject\Release\

:: 3. Windows標準コマンド(PowerShell)でZIP圧縮
powershell -Command "Compress-Archive -Path '_release_temp\*' -DestinationPath 'OSR-CDI_v1.5.6-beta_Package.zip' -Force"

:: 4. 一時フォルダの削除
rmdir /s /q _release_temp

echo ZIP package created: OSR-CDI_v1.5.6-beta_Package.zip