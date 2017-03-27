Write-Host "Creating Scintillua++ Release"

Remove-Item .\ScintilluaPlusPlus.zip -ErrorAction SilentlyContinue
Remove-Item .\release\* -Force -Recurse -ErrorAction SilentlyContinue

New-Item -ItemType directory -Path .\release\config\Scintillua++

Copy-Item .\bin\Release_Win32\Scintillua++.dll .\release
Copy-Item .\bin\Release_Win32\LexLPeg.dll .\release\config\Scintillua++

Copy-Item .\extra\Scintillua++.ini .\release\config
Copy-Item .\ext\Scintillua\lexers\* .\release\config\Scintillua++ -recurse
Copy-Item .\extra\npp.lua .\release\config\Scintillua++\themes

Compress-Archive -Path .\release\* -DestinationPath .\ScintilluaPlusPlus.zip

Remove-Item .\release\ -Force -Recurse
