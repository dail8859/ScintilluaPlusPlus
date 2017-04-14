# Scintillua++
Notepad++ plugin for supporting [Scintillua's](https://foicica.com/scintillua/) LPeg lexers. This adds nearly 70 new languages to Notepad++.

**Note:** This is still in early development. This is not meant to replace or replicate current Notepad++ behavior. This does not function as a normal "external lexer" as most other do, such as being configurable through the Style Configurator.

## Installation
Download a release from the [Release](https://github.com/dail8859/ScintilluaPlusPlus/releases) page. Extract the zip file then:

1. Copy the `Scintillua++.dll` (or `Scintillua++_64.dll` for the 64bit version) to Notepad++'s plugin directory
1. Copy the `Scintillua++` directory into Notepad++'s plugin config directory - most likely located under `%APPDATA%`

## Usage
This auto detects files based on file names or file extensions. By default this will only parse files that Notepad++ does not recognize. Setting the `override` flag to `true` in the settings file will parse files even if Notepad++ already supports the file type.

This comes with a default theme that is very similar to Notepad++'s default theme. You are also able to create your own as long as they are located in the `themes` directory and selected in the settings.

You can also create your own custom lexers using the [LPeg](http://www.inf.puc-rio.br/~roberto/lpeg/) Lua library.

## Development
The code has been developed using MSVC 2015. Building the code will generate the DLL which can be used by Notepad++. For convenience, MSVC automatically copies the DLL and files into Notepad++'s directories.

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

Other parts of this are covered under their respective licenses.
