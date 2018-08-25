@echo off
REM 2010-09-02 .. 2012-10-09
REM set COPY=echo (copy)
set COPY=copy

set PROJECT=avrdude
REM コピー先のディレクトリをセットする
set TARGET=C:\bin

REM OSのチェック
if "%comspec%"=="" goto unknown
if "%comspec%"=="c:\command.com" goto Win9x
if "%comspec%"=="%systemroot%\system32\cmd.exe" goto WinNT

:unknown
echo OSの種類が分かりません。
goto END

:Win9x
echo 手作業で、ファイル群のコピーを行ってください。
goto END

:WinNT

if "%1"=="" (
REM 引数なし（標準設定）
) else (
REM HELPのチェック
  if "%1"=="HELP" goto HELP
  if "%1"=="?" goto HELP
  if "%1"=="help" goto HELP
  set TARGET=%1
)

echo ================= %PROJECT% (YCIT版) をセットアップします =================
echo avrdude.confは上書きしません。avrdude.confがある時は、配布のavrdude.confを
echo avrdude-dist.conf　というファイル名でコピーします。
echo picspx.exe, picspx.ini　は上書きでコピーします。
echo \
echo %TARGET%フォルダ にコピーを希望する場合は、[Y]を大文字で入力してください。
SET ANS=n
SET /P ANS="↑にセットアップをしますか[Y/n]"
IF %ANS%==Y (
  GOTO START
) ELSE (
  GOTO WHERE
)

:WHERE
SET TARGET=
SET /P TARGET="希望するフォルダ名を入力してください:"
SET ANS=n
SET /P ANS="%TARGET%フォルダにセットアップをしますか？ [Y/n]"
IF %ANS%==Y (
  GOTO START
) ELSE (
  echo ================= %PROJECT% のセットアップを中断 ====================
  GOTO END
)

:START
echo 必要なファイル群を%TARGET%にコピーします。

REM Check
IF NOT EXIST %TARGET% ( 
  mkdir %TARGET%
)

REM Check
IF NOT EXIST %TARGET%\avrdude-html ( 
  mkdir %TARGET%\avrdude-html
  echo %COPY% Help files
  %COPY% avrdude-html\*.* %TARGET%\avrdude-html > NUL
)

echo ==== [COPY to %TARGET%] ====
echo %COPY% avrdude.exe %TARGET%
%COPY% avrdude.exe %TARGET% > NUL

echo %COPY% avrdude_orig.exe %TARGET%
%COPY% avrdude_orig.exe %TARGET% > NUL

echo %COPY% avrdude_orig.conf %TARGET%
%COPY% avrdude_orig.conf %TARGET% > NUL

echo %COPY% avrdude.rc %TARGET%
%COPY% avrdude.rc %TARGET% > NUL

REM Check
IF NOT EXIST %TARGET%\avrdude.conf (
  echo %COPY% avrdude.conf %TARGET%
  %COPY% avrdude.conf %TARGET% > NUL
) ELSE (
  echo %COPY% avrdude-dist.conf %TARGET%
  %COPY% avrdude.conf %TARGET%\avrdude-dist.conf > NUL
)

REM Check (fuse.txtが無い時にコピーする)
IF NOT EXIST %TARGET%\fuse.txt (
  echo %COPY% fuse.txt %TARGET%
  %COPY% fuse.txt %TARGET% > NUL
)

REM Check (avrdude-GUI.exe.configが無い時にコピーする)
IF NOT EXIST %TARGET%\avrdude-GUI.exe.config (
  echo %COPY% avrdude-GUI.exe.config %TARGET%
  %COPY% avrdude-GUI.exe.config %TARGET% > NUL
)

echo %COPY% avrdude-GUI.exe %TARGET%
%COPY% avrdude-GUI.exe %TARGET% > NUL

echo %COPY% avrdude-GUI.exe.config %TARGET%
%COPY% avrdude-GUI.exe.config %TARGET% > NUL

echo %COPY% bootloadHID.exe %TARGET%
%COPY% bootloadHID.exe %TARGET% > NUL

echo %COPY% srec_cat.exe %TARGET%
%COPY% srec_cat.exe %TARGET% > NUL

echo %COPY% srec_info.exe %TARGET%
%COPY% srec_info.exe %TARGET% > NUL

echo %COPY% srec_cmp.exe %TARGET%
%COPY% srec_cmp.exe %TARGET% > NUL

echo %COPY% pthreadGC2.dll %TARGET%
%COPY% pthreadGC2.dll %TARGET% > NUL

echo %COPY% usbtool.exe %TARGET%
%COPY% usbtool.exe %TARGET% > NUL

echo %COPY% picspx.exe %TARGET%
%COPY% picspx.exe %TARGET% > NUL

echo %COPY% picspx.ini %TARGET%
%COPY% picspx.ini %TARGET% > NUL

echo %COPY% hidspx.exe %TARGET%
%COPY% hidspx.exe %TARGET% > NUL

echo %COPY% hidspx.ini %TARGET%
%COPY% hidspx.ini %TARGET% > NUL

echo ==== %PROJECT% setup finished ====
goto END

:HELP
  echo これは %PROJECT%のセットアッププログラムであり、更新時にも利用できます。
  echo   setup [希望するフォルダ名]
  echo のように使います。
  echo [希望するフォルダ名]を省略すると標準とする%TARGET%にコピーします。

:END
pause
