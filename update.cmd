echo off

rem build
call build.cmd

rem copy to local arduino project libraries folders

del C:\Users\Jumangee\Documents\Arduino\libraries\IniConfig\*.* /y
copy IniConfig\*.* C:\Users\Jumangee\Documents\Arduino\libraries\IniConfig\ /y

del e:\Projects\!DIY\ESP32\TelegramBot\lib\IniConfig\*.* /y
copy IniConfig\*.* e:\Projects\!DIY\ESP32\TelegramBot\lib\IniConfig\ /y

del e:\Projects\!DIY\ESP32\SecurGramCam32\lib\IniConfig\*.* /y
copy IniConfig\*.* e:\Projects\!DIY\ESP32\SecurGramCam32\lib\IniConfig\ /y
