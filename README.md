## dw - Diceware manager

dw is a simple Diceware software created for learning purposes, but with 
the intention of also being useful for creating and handling passphrases 
using a diceware method.

Given the fact that computers are not bound to six sides, a character set is introduced instead for the ability to keep roughly the same entropy when shortening the key length by extending the set of possible characters in a "dice roll", or vice versa.
### Features
#### List file creation
_dw_ is able to create list files given a source of words to use, currently only supports files but later on perhaps stdin as well.
The amount of words needed depends on configuration parameters such as character set and key length.

#### List file importing
In the case of wanting to use a diceware list already on hand, but not in the format _dw_ uses, _dw_ is able to import the list into the format _dw_ prefers.
It will ask for formatting in the file in order to be able to separate keys from words, this format is written as "k-w", k meaning keys and w meaning words. The current implementation makes "k-w" equivalent to "k-:a w" as it breaks as soon as it finds the first character after either k or w, whichever comes first.

#### Passphrase generation
_dw_ is of course able to generate passphrases by using a existing diceware list.

The amount of words can either be given in command-line by _dw -g#_ or later given when asked by the software itself.

#### Passphrase lookup
Given the diceware list used for the generation of the passphrase, _dw_ is able to look up the passphrase given word ID input from stdin.

Either use some utility writing to stdout and redirect to _dw_, or look up individual keys and exit by sending EOF (Ctrl-d)

### Configuration
Upon execution with no configuration file found, _dw_ will offer to generate one for you with default values.

#### Parameters
_NOTE: Amount of words needed are decided by |character-set|^key-size, and by implication also describes in idea of the memory needed for the list representation. While keeping this value high is preferable, keep in mind that allocation errors will occur upon insufficient available memory._

__character-set__: The set of characters possible in a "dice roll", can be defined in groups by [09] for 0-9. This feature uses the character position in the ascii table, so [0z] may give more characters than you may intend. _(default: 0123456789)_

__key-size__: The amount of characters from the character set needed to identify one word. _(default: 5)_

__default-list__: What list in $DW\_HOME that should be used when _dw_ is executed without a list file supplied. _(default: default)_

__min-word-length__: The minimum length a word needs to be in order to be added to the list, it is recommended to have a length of at least 14 characters, including spaces, for a passphrase of 5 words and 17 characters for 6 words, keeping _min-word-length_ at 2 will make that condition be fulfilled always, setting it to 1 will accept more words, but it is recommended to ensure the passphrase length is good. _(default: 2)_

__unique__: If true duplicate words found will be discarded, _HIGHLY RECOMMENDED TO BE SET TO TRUE_ as duplicate words in a list decreases entropy. _(default: true)_

### Dependencies
- libconfig (>= 1.5-3, probably lower is possible)
