@echo off 
@title BDK Cleaner

echo ��ʼɾ������������ļ�...
set target=*.exe *.ncb *.pdb *.ilk *.suo *.user *.obj *.res *.htm *.manifest *.dep *.idb *.lib
for /r %%i in (%target%) do ( 
@echo delete %%i
del %%i
)
echo ���ɾ������������ļ�

pause