========================================================================
                    Base Development Kit
========================================================================

1. BDK�����ĵ�������˵����
	- BOOST
	  * ֻʹ���������Ŀ�
	  * ��Ҫʹ�õĿ����£�
		** noncopyable -> ���ÿ������캯���͸�ֵ�����
		** function    -> �ص�����
		** bind        -> �ص�����
	- PTHREAD [win32]
	  * ��Ҫʹ�õĿ����£�
		** thread
		** mutex
		** condition
		
	[Note] 
	- ������ֻ��Ҫ��Visual Studio 2005�н��м����ü���
	  * Tool -> Options -> Projects and Solutions -> VC++ Directories
		** Include files : ����BOOST��PTHREADͷ�ļ�·��
		** Library files : ����PTHREAD���ļ�·��
	  
