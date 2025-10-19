#include "mainwindow.h"

#include <QApplication>
#include <windows.h>
#include <dbghelp.h>
#include <QDateTime>
#include <QMessageBox>
#include <QTextCodec>

LONG CreateCrashHander(EXCEPTION_POINTERS *pException)
{
    if (!pException) {  // 防止空指针崩溃
        QMessageBox::warning(NULL, "错误", "异常指针为空！", QMessageBox::Ok);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 获取应用程序工作目录路径（程序运行时所在的目录）
    QString appWorkPath = QCoreApplication::applicationDirPath();

    // 1. 生成合法的文件名（不含非法字符）
    QDateTime CurrentDTime = QDateTime::currentDateTime();
    QString CurrentDtimeStr = CurrentDTime.toString("yyyy-MM-dd hh-mm-ss");  // 用-替代:
    // 获取用户文档目录（确保有写入权限）
    QString dumpText = appWorkPath + "/Dump_" + CurrentDtimeStr + ".dmp";  // 完整路径

    // 2. 创建文件（显式使用宽字符版本CreateFileW）
    HANDLE DumpHandle = CreateFileW(
        (LPCWSTR)dumpText.utf16(),  // 宽字符路径
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    // 3. 处理文件创建失败的情况
    if (DumpHandle == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();  // 获取系统错误代码
        QMessageBox::critical(NULL, "创建dump失败",
                            QString("无法创建dump文件！\n路径：%1\n错误代码：%2")
                            .arg(dumpText).arg(errorCode),
                            QMessageBox::Ok);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // 4. 生成minidump（使用更完整的类型）
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;

    // 调用MiniDumpWriteDump生成dump
    BOOL success = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        DumpHandle,
        MiniDumpWithFullMemory,  // 包含完整内存信息
        &dumpInfo,
        NULL,
        NULL
    );

    if (!success) {
        QMessageBox::warning(NULL, "生成dump失败",
                            QString("MiniDumpWriteDump调用失败！错误代码：%1")
                            .arg(GetLastError()),
                            QMessageBox::Ok);
    }

    CloseHandle(DumpHandle);  // 关闭文件句柄

    // 5. 显示崩溃信息
    EXCEPTION_RECORD *record = pException->ExceptionRecord;
    QString errCode = QString::number(record->ExceptionCode, 16).toUpper();
    QString errAddr = QString::number((quint64)record->ExceptionAddress, 16).toUpper();
    QString errFlag = QString::number(record->ExceptionFlags, 16).toUpper();
    QString errPara = QString::number(record->NumberParameters, 16).toUpper();

    QMessageBox::warning(NULL, "程序崩溃",
                        QString("错误代码：0x%1\n错误地址：0x%2\n错误标志：0x%3\n参数数量：0x%4\nDump文件已保存至：%5")
                        .arg(errCode).arg(errAddr).arg(errFlag).arg(errPara).arg(dumpText),
                        QMessageBox::Ok);

    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)CreateCrashHander);


    MainWindow w;
    w.show();
    return a.exec();
}
