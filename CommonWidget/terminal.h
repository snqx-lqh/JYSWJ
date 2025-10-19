#ifndef TERMINAL_H
#define TERMINAL_H

#include <QPlainTextEdit>
#include <QIODevice>
#include <QDebug>
#include <QTextCodec>
#include <QDateTime>

class Terminal : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Terminal(  QWidget *parent = nullptr);
    void setEncoding(const QByteArray &encodingName);   // 新增：设置编码
    void appendData(const QByteArray &ba);
    void appendSendData(const QByteArray &ba);
    void setShowHexState(bool state);
    void setShowDateState(bool state);

private:
    QTextCodec *m_codec = nullptr;                      // 当前使用的编码
    bool mShowHexState  = false;
    bool mShowDateState = false;

signals:
    void sendBytes(QByteArray bytes);

public slots:
    void onReadBytes(QByteArray bytes);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void insertFromMimeData(const QMimeData *source) override;

private slots:

};

#endif // TERMINAL_H
