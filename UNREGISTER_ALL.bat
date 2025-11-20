@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"

:: TestServerProxy typelib
reg delete "HKCR\TypeLib\{8E42F33F-E5E3-46B4-8B39-0A03DCF338CC}"   /f 2> NUL

:: remove interfaces
reg delete "HKCR\Interface\{54E049DB-C39C-4025-B1EF-2FC0E5246277}" /f 2> NUL
reg delete "HKCR\Interface\{7D278258-3B05-45D3-AD60-CF65497B56B6}" /f 2> NUL

:: remove HandleMgr class
reg delete "HKCR\CLSID\{C8767698-8297-4BE1-904E-FC25034851D2}"     /f 2> NUL

:: remove DataHandleProxy class
reg delete "HKCR\CLSID\{D84F768B-55F6-4B3A-AF68-9276C9488696}"     /f 2> NUL

:: TestServer typelib
reg delete "HKCR\TypeLib\{ECC2EB22-C8CB-4034-993C-158F68FCD39E}"   /f 2> NUL

::pause
