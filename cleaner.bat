@echo off 
@title BDK Cleaner

echo 开始删除编译产生的文件...
set target=*.exe *.ncb *.pdb *.ilk *.suo *.user *.obj *.res *.htm *.manifest *.dep *.idb *.lib
for /r %%i in (%target%) do ( 
@echo delete %%i
del %%i
)
echo 完成删除编译产生的文件

pause