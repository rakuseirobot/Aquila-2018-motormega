@echo off
REM 2010-09-02 .. 2012-10-09
REM set COPY=echo (copy)
set COPY=copy

set PROJECT=avrdude
REM �R�s�[��̃f�B���N�g�����Z�b�g����
set TARGET=C:\bin

REM OS�̃`�F�b�N
if "%comspec%"=="" goto unknown
if "%comspec%"=="c:\command.com" goto Win9x
if "%comspec%"=="%systemroot%\system32\cmd.exe" goto WinNT

:unknown
echo OS�̎�ނ�������܂���B
goto END

:Win9x
echo ���ƂŁA�t�@�C���Q�̃R�s�[���s���Ă��������B
goto END

:WinNT

if "%1"=="" (
REM �����Ȃ��i�W���ݒ�j
) else (
REM HELP�̃`�F�b�N
  if "%1"=="HELP" goto HELP
  if "%1"=="?" goto HELP
  if "%1"=="help" goto HELP
  set TARGET=%1
)

echo ================= %PROJECT% (YCIT��) ���Z�b�g�A�b�v���܂� =================
echo avrdude.conf�͏㏑�����܂���Bavrdude.conf�����鎞�́A�z�z��avrdude.conf��
echo avrdude-dist.conf�@�Ƃ����t�@�C�����ŃR�s�[���܂��B
echo picspx.exe, picspx.ini�@�͏㏑���ŃR�s�[���܂��B
echo \
echo %TARGET%�t�H���_ �ɃR�s�[����]����ꍇ�́A[Y]��啶���œ��͂��Ă��������B
SET ANS=n
SET /P ANS="���ɃZ�b�g�A�b�v�����܂���[Y/n]"
IF %ANS%==Y (
  GOTO START
) ELSE (
  GOTO WHERE
)

:WHERE
SET TARGET=
SET /P TARGET="��]����t�H���_������͂��Ă�������:"
SET ANS=n
SET /P ANS="%TARGET%�t�H���_�ɃZ�b�g�A�b�v�����܂����H [Y/n]"
IF %ANS%==Y (
  GOTO START
) ELSE (
  echo ================= %PROJECT% �̃Z�b�g�A�b�v�𒆒f ====================
  GOTO END
)

:START
echo �K�v�ȃt�@�C���Q��%TARGET%�ɃR�s�[���܂��B

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

REM Check (fuse.txt���������ɃR�s�[����)
IF NOT EXIST %TARGET%\fuse.txt (
  echo %COPY% fuse.txt %TARGET%
  %COPY% fuse.txt %TARGET% > NUL
)

REM Check (avrdude-GUI.exe.config���������ɃR�s�[����)
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
  echo ����� %PROJECT%�̃Z�b�g�A�b�v�v���O�����ł���A�X�V���ɂ����p�ł��܂��B
  echo   setup [��]����t�H���_��]
  echo �̂悤�Ɏg���܂��B
  echo [��]����t�H���_��]���ȗ�����ƕW���Ƃ���%TARGET%�ɃR�s�[���܂��B

:END
pause
