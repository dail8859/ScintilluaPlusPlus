# Scintillua++
Notepad++ plugin for supporting [Scintillua's](https://foicica.com/scintillua/) LPeg lexers. This adds nearly 70 new languages to Notepad++.

**Note:** This is still in early development.

## Usage
By default this will only parse files that Notepad++ does not recognize. Setting the `override` flag to `true` in the settings file will parse files even if Notepad++ already supports the file type.

This comes with a default theme that is very similar to Notepad++'s default theme. You are also able to create your own as long as they are located in the appropriate directory.

You can also create your own custom lexers using the [LPeg](http://www.inf.puc-rio.br/~roberto/lpeg/) Lua library.

## Installation
No public release is available at this time.

## Development
The code has been developed using MSVC 2013. Building the code will generate the DLL which can be used by Notepad++. For convenience, MSVC automatically copies the DLL and files into Notepad++'s directories.

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

Other parts of this are covered under their respective licenses.
