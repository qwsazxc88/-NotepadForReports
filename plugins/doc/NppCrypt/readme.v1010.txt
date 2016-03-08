------------------------------------------------------------------------------------------------------
NppCrypt 1.0.1.0 (unicode)
------------------------------------------------------------------------------------------------------
bin: http://www.cerberus-design.de/nppcrypt/nppcryptv1010.zip
------------------------------------------------------------------------------------------------------
Sourcecode: http://www.cerberus-design.de/nppcrypt/nppcryptv1010.src.zip
------------------------------------------------------------------------------------------------------
email: kontakt@cerberus-design.de
------------------------------------------------------------------------------------------------------
Plugin for Notepad++ [ Copyright (C)2003 Don HO <don.h@free.fr> ]
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
------------------------------------------------------------------------------------------------------

encrypt/decrypt/hash your notepad++ files using symmetric ciphers like aes.

<<< IMPORTANT: nppcrypt-files created by older versions may not work any more!  >>>
<<< this is true for 1.007 files and 1.008/9 files that use pbkdf1.             >>>
<<< it's annoying but you get a much more powerful plugin and safer encryption. >>>

------------------------------------------------------------------------------------------------------
this software uses the following third party stuff:
-> openssl 1.0.1i  ( http://www.openssl.org )
-> tinyxml2 ( http://www.grinninglizard.com/tinyxml2 )
-> keccak/sha3 ( http://keccak.noekeon.org )
-> crypt_blowfish ( http://www.openwall.com/crypt )
-> scrypt ( https://www.tarsnap.com/scrypt.html )
-> libb64 project ( sourceforge.net/projects/libb64 )

------------------------------------------------------------------------------------------------------
A few words about this new release (1.0.1.0)
------------------------------------------------------------------------------------------------------

the main aims of this update are:

1. better encryption: for example by providing bcrypt ( https://en.wikipedia.org/wiki/Bcrypt ) and
   scrypt ( https://en.wikipedia.org/wiki/Scrypt ) as alternatives to pbkdf2 key derivation.

2. transparency: "security by obscurity" is stupid. nppcrypt aims to make it absolutly clear how
   it encrypts something. therefore a human-readable header is added to every ciphertext.
   (of course you can also just delete it afterwards)

3. data authentication: nppcrypts now includes two options to authenticate encrypted data. You can
   either choose the gcm or ccm mode for aes, which creates a tag to authenticate the encrypted data
   plus the iv and salt-strings of the header. or you add an additional hmac to validate the
   encrypted data plus the whole body of the header (everything between <nppcrypt> and </nppcrypt>).

------------------------------------------------------------------------------------------------------

some implementation details: 
 - every user input is converted to utf8.
 - to get the needed key-length etc. from bcrypt's fixed 24 byte output, keccak shake128 hash is used.
 - authentication keys entered by the user are also hashed by keccak shake128 with an output 
   length of 16 bytes.

