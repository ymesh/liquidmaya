@echo Runing %0
@rem export PYTHONHOME=/usr:/usr
@rem export PYTHONPATH=/usr/lib64/python2.6/:/usr/lib64/python2.6/lib-dynload:$PYTHONPATH

@echo LIQUIDHOME=%LIQUIDHOME%
@echo PYTHONHOME=%PYTHONHOME%
@echo PYTHONPATH=%PYTHONPATH%
@echo PATH=%PATH%

xml2afanasy.py %1 %2

pause
