#ifndef SYNTAXDEF_COMLIB_H_H_
#define SYNTAXDEF_COMLIB_H_H_

#define LABEL_DEFAULT       "Default"
#define LABEL_LOG_CONTENT   "LogContent"
#define LABEL_DBG_CONTENT   "DbgContent"

//LogView �ļ����ݼ�����ǩ
#define LABEL_SEARCH_FILE   "SearchFile"
#define LABEL_SEARCH_LOG    "SearchLog"

#define LABEL_CMD_SEND      "CmdSend"
#define LABEL_CMD_RECV      "CmdRecv"
#define LABEL_CMD_HIGHT     "cmdHight"


#define LABEL_CALLSTACK     "CallStack"
#define LABEL_TCP_PIPE1     "TcpPipe1"
#define LABEL_TCP_PIPE2     "TcpPipe2"

//����Demo��ر�ǩ
#define LABEL_DEMO_TEST1     "DemoTest1"
#define LABEL_DEMO_TEST2     "DemoTest2"
#define LABEL_DEMO_TEST3     "DemoTest3"
#define LABEL_DEMO_TEST4     "DemoTest4"

/*
20190618
STYLE ���������������ɫ�����ԣ���Ҫע��,����STYLEֵ��256,
�������ֵ�ᵼ�����õ���ɫ������Ч��STYLE_DEFAULT = 32
���ǵķ�Χ��101��ʼ����
*/
#define STYLE_CONTENT           101
#define STYLE_FILTER            102
#define STYLE_SELECT            103
#define STYLE_ERROR             104
#define STYLE_TCP_PIPE1         105    //tcp����ʽ1
#define STYLE_TCP_PIPE2         106    //tcp����ʽ2

#define COLOUR_DEFAULT_TEXT     RGB(0x00, 0xff, 0x00)
#define COLOUR_DEFAULT_BACK     RGB(0x00, 0x00, 0x00)

//cmd������ʽ
#define STYLE_CMD_DEFAULT       107    //cmd default��ʽ
#define STYLE_CMD_SEND          108    //cmd send��ʽ
#define STYLE_CMD_RECV          109    //cmd recv��ʽ
#define STYLE_CMD_HIGHT         110    //cmd hight��ʽ

//����Demo�����ʽ
#define STYLE_DEMO_DEFAULT      119    //Ĭ����ʽ
#define STYLE_DEMO_CALL         120    //callָ��
#define STYLE_DEMO_JMP          121    //jmpָ��
#define STYLE_DEMO_HEX1         122    //hex addr
#define STYLE_DEMO_HEX2         123    //hex str
#define STYLE_DEMO_HEX3         124    //hex byte
#define STYLE_DEMO_PROC         125    //ģ�麯��

//File Search Style
#define STYLE_SEARCH_FILE       126    //File Path
#define STYLE_SEARCH_LOG        127    //File Log

#define STYLE_LOG_KEYWORD_BASE  160    //��־�ؼ���
#define STYLE_LOG_WARN          251    //��־����
#define STYLE_LOG_ERROR         252    //��־����

#define NOTE_KEYWORD    SCE_UNIVERSAL_FOUND_STYLE_EXT1      //�ؼ��ָ���
#define NOTE_SELECT     SCE_UNIVERSAL_FOUND_STYLE_EXT2      //ѡ�����
#endif //SYNTAXDEF_COMLIB_H_H_
