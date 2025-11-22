@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"

:: TestServerProxy typelib
reg delete "HKCR\TypeLib\{8E42F33F-E5E3-46B4-8B39-0A03DCF338CC}"   /f 2> NUL

:: remove interfaces
reg delete "HKCR\Interface\{3CBD54F2-C9A5-4E64-A582-B7D6AD14CE3D}" /f 2> NUL
reg delete "HKCR\Interface\{7D278258-3B05-45D3-AD60-CF65497B56B6}" /f 2> NUL

:: remove HandleMgr class
reg delete "HKCR\CLSID\{C8767698-8297-4BE1-904E-FC25034851D2}"     /f 2> NUL

:: remove DataHandleProxy class
reg delete "HKCR\CLSID\{9A7D97EB-1491-438E-B88C-234390D727F2}"     /f 2> NUL

:: TestServer typelib
reg delete "HKCR\TypeLib\{ECC2EB22-C8CB-4034-993C-158F68FCD39E}"   /f 2> NUL

::pause
