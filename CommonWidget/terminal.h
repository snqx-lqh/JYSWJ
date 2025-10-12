#ifndef TERMINAL_H
#define TERMINAL_H

#include <QPlainTextEdit>
#include <QIODevice>
#include <QDebug>
#include <QTextCodec>

class Terminal : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit Terminal(  QWidget *parent = nullptr);
    void setIODevice(QIODevice *io);
    void setEncoding(const QByteArray &encodingName);   // 新增：设置编码

private:
    QIODevice *m_io;
    QTextCodec *m_codec = nullptr;                      // 当前使用的编码

    void appendData(const QByteArray &ba);

public slots:
    void onReadBytes(QByteArray bytes);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void insertFromMimeData(const QMimeData *source) override;

private slots:
    void onReadyRead();
};

#endif // TERMINAL_H
