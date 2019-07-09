#ifndef SYNTAXDEF_COMLIB_H_H_
#define SYNTAXDEF_COMLIB_H_H_

#define LABEL_DEFAULT       "Default"
#define LABEL_LOG_CONTENT   "LogContent"
#define LABEL_DBG_CONTENT   "DbgContent"
#define LABEL_CALLSTACK     "CallStack"
#define LABEL_TCP_PIPE1     "TcpPipe1"
#define LABEL_TCP_PIPE2     "TcpPipe2"

/*
20190618
STYLE 用于设置字体的颜色等属性，需要注意,最大的STYLE值是256,
超过这个值会导致设置的颜色属性无效，STYLE_DEFAULT = 32
我们的范围从101开始设置
*/
#define STYLE_CONTENT           101
#define STYLE_FILTER            102
#define STYLE_SELECT            103
#define STYLE_ERROR             104
#define STYLE_TCP_PIPE1         105    //tcp流样式1
#define STYLE_TCP_PIPE2         106    //tcp流样式2
#define STYLE_LOG_KEYWORD_BASE  160    //日志关键字
#define STYLE_LOG_WARN          251    //日志警告
#define STYLE_LOG_ERROR         252    //日志错误

#define NOTE_KEYWORD    SCE_UNIVERSAL_FOUND_STYLE_EXT1      //关键字高亮
#define NOTE_SELECT     SCE_UNIVERSAL_FOUND_STYLE_EXT2      //选择高亮
#endif //SYNTAXDEF_COMLIB_H_H_
