MKDIR CuteMaze
STRIP release\CuteMaze.exe
COPY release\CuteMaze.exe CuteMaze
COPY %QTDIR%\bin\mingwm10.dll CuteMaze
COPY %QTDIR%\bin\QtCore4.dll CuteMaze
COPY %QTDIR%\bin\QtGui4.dll CuteMaze
COPY %QTDIR%\bin\QtXml4.dll CuteMaze
COPY %QTDIR%\bin\QtSvg4.dll CuteMaze
