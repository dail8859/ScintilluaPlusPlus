Write-Host "Creating Scintillua++ Releases"

function make_release($build, $postfix)
{
	Remove-Item .\ScintilluaPlusPlus$postfix.zip -ErrorAction SilentlyContinue
	Remove-Item .\release\* -Force -Recurse -ErrorAction SilentlyContinue

	New-Item -ItemType directory -Path .\release\config\Scintillua++

	Copy-Item .\bin\Release_$build\Scintillua++$postfix.dll .\release
	#Copy-Item .\bin\Release_$build\LexLPeg$postfix.dll .\release\config\Scintillua++
	Copy-Item .\bin\Release_$build\LexLPeg.dll .\release\config\Scintillua++

	Copy-Item .\extra\Scintillua++.ini .\release\config
	Copy-Item .\ext\Scintillua\lexers\* .\release\config\Scintillua++ -recurse
	Copy-Item .\extra\npp.lua .\release\config\Scintillua++\themes

	Compress-Archive -Path .\release\* -DestinationPath .\ScintilluaPlusPlus$postfix.zip

	Remove-Item .\release\ -Force -Recurse
}

make_release "Win32" ""
make_release "x64" "_64"
