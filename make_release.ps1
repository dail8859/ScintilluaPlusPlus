Write-Host "Creating Scintillua++ Releases"

function make_release($build, $postfix)
{
	Remove-Item .\ScintilluaPlusPlus$postfix.zip -ErrorAction SilentlyContinue
	Remove-Item .\release\* -Force -Recurse -ErrorAction SilentlyContinue

	New-Item -ItemType directory -Path .\release\Scintillua++

	Copy-Item .\bin\Release_$build\Scintillua++$postfix.dll .\release
	Copy-Item .\bin\Release_$build\LexLPeg$postfix.dll .\release\Scintillua++

	Copy-Item .\extra\default.ini .\release\Scintillua++
	Copy-Item .\ext\Scintillua\lexers\* .\release\Scintillua++ -recurse
	Copy-Item .\extra\npp.lua .\release\Scintillua++\themes

	Compress-Archive -Path .\release\* -DestinationPath .\ScintilluaPlusPlus$postfix.zip

	Remove-Item .\release\ -Force -Recurse
}

make_release "Win32" ""
make_release "x64" "_64"
