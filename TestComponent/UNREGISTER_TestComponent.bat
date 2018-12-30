@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Fix issue with "Run as Administrator" current dir
setlocal enableextensions
cd /d "%~dp0"


:: Remove almost all traces of TestComponent from registry
for %%P in (32 64) do (
  for %%R in (HKEY_LOCAL_MACHINE HKEY_CURRENT_USER) do (
    :: TypeLib & AppID
    reg delete "%%R\SOFTWARE\Classes\TypeLib\{8E42F33F-E5E3-46B4-8B39-0A03DCF338CC}"   /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\AppID\{CDD196FE-70ED-46F4-BED7-57615CB78F9B}"     /f /reg:%%P 2> NUL
  
    :: class & interface
    reg delete "%%R\SOFTWARE\Classes\CLSID\{B7B84F73-8638-4EE5-A486-2C9455C0CD2B}"     /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{54E049DB-C39C-4025-B1EF-2FC0E5246277}" /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\CLSID\{C8767698-8297-4BE1-904E-FC25034851D2}"     /f /reg:%%P 2> NUL
    reg delete "%%R\SOFTWARE\Classes\Interface\{7D278258-3B05-45D3-AD60-CF65497B56B6}" /f /reg:%%P 2> NUL
  )
)

::pause
