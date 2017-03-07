-- Notepad++ lexer theme for Scintillua.

local property = require('lexer').property

property['color.black'] = '#000000'
property['color.green'] = '#008000'
property['color.maroon'] = '#95004A'
property['color.brown'] = '#804000'
property['color.blue'] = '#0000FF'
property['color.yellow'] = '#FF8000'
property['color.grey'] = '#808080'
property['color.darkblue'] = '#000080'
property['color.lightblue'] = '#0080C0'
property['color.purple'] = '#8000FF'
property['color.darkerblue'] = '#0000A0'
property['color.red'] = '#7F0000'
property['color.teal'] = '#007F7F'
property['color.white'] = '#FFFFFF'
property['color.orange'] = '#FF8000'

-- Default styles.
local font = 'Courier New'
local size = 10

property['style.default'] = 'font:'..font..',size:'..size..
                            ',fore:$(color.black),back:$(color.white)'

-- Token styles.
property['style.nothing'] = ''
property['style.class'] = 'fore:$(color.black),bold'
property['style.comment'] = 'fore:$(color.green)'
property['style.constant'] = 'fore:$(color.teal),bold' -- Change to lightblue?
property['style.definition'] = 'fore:$(color.black),bold'
property['style.error'] = 'fore:$(color.red)'
property['style.function'] = 'fore:$(color.lightblue),bold'
property['style.keyword'] = 'fore:$(color.blue),bold'
property['style.label'] = 'fore:$(color.teal),bold'
property['style.number'] = 'fore:$(color.orange)'
property['style.operator'] = 'fore:$(color.darkblue),bold'
property['style.regex'] = 'fore:$(color.purple)'
property['style.string'] = 'fore:$(color.grey)'
property['style.preprocessor'] = 'fore:$(color.brown)'
property['style.tag'] = 'fore:$(color.blue)'
property['style.type'] = 'fore:$(color.purple)'
property['style.variable'] = 'fore:$(color.black)'
property['style.whitespace'] = ''
property['style.embedded'] = 'fore:$(color.blue)'
property['style.identifier'] = '$(style.nothing)'

-- Predefined styles.
property['style.linenumber'] = 'fore:$(color.grey),back:#E4E4E4'
property['style.bracelight'] = 'fore:#0000FF,bold'
property['style.bracebad'] = 'fore:#FF0000,bold'
property['style.controlchar'] = '$(style.nothing)'
property['style.indentguide'] = 'fore:#C0C0C0,back:$(color.white)'
property['style.calltip'] = 'fore:$(color.white),back:#444444'
