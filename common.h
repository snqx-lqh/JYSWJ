#ifndef COMMON_H
#define COMMON_H

typedef enum{
    IOSETTING_VISIBLE = 0, ///< IOSetting 窗口的显示和关闭 0 关闭 1 显示
    MULTISEND_VISIBLE,     ///< MultiSend 窗口的显示和关闭 0 关闭 1 显示
    IOConnect_State,       ///< IO连接状态 0 连接失败 1 连接成功
    IOSendNewLine_State,   ///< IO发送新行状态
    RecvShowHex_State,     ///< Recv 框显示HEX状态
    RecvShowDate_State,    ///< Recv 框显示时间戳状态
    RecvShowSend_State,    ///< Recv 框显示发送回显
    RecvClear_State,    ///< Recv 框清空
    RecvCount,          ///< Recv 计数
    SendCount,          ///< Send 计数
    TextCodec,          ///< TextCodec 编码格式

}STATE_CHANGE_TYPE_T;

#endif // COMMON_H
