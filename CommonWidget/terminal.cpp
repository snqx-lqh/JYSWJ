#include "terminal.h"
#include <QScrollBar>
#include <QMimeData>

Terminal::Terminal( QWidget *parent)
    : QPlainTextEdit(parent)
{
    setFont(QFont("Consolas", 10));
    setStyleSheet("background:#222;color:#ddd;");
    setContentsMargins(0,0,0,0);

    m_codec = QTextCodec::codecForName("UTF-8");
    m_fixedCursor = textCursor();

    m_fmtDefault = QTextCharFormat();            // 黑底白字/系统默认
    m_fmtCurrent = m_fmtDefault;

    m_customSelection.start = 0;
    m_customSelection.end   = 0;
    m_customSelection.active = false;

    QTextOption opt = document()->defaultTextOption();
    opt.setWrapMode(QTextOption::WrapAnywhere);   // 纯字符截断
    document()->setDefaultTextOption(opt);
}


void Terminal::setEncoding(const QByteArray &encodingName)
{
    QTextCodec *codec = QTextCodec::codecForName(encodingName);
    if (codec)
        m_codec = codec;
}


void Terminal::appendData(const QByteArray &ba)
{
    if (ba.isEmpty()) return;

    qDebug() << "Terminal::appendData current thread:" << QThread::currentThread();

    // 使用常量引用，避免不必要的复制
    const QByteArray &data = ba;
    QTextCursor cur = textCursor();

    // ===== 时间戳显示 =====
    if (mShowDateState) {
        QString dateTime = QString("[%1]收<-")
                .arg(QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss.zzz"));
        cur.insertText("\n" + dateTime);
    }

    // ===== HEX 模式 =====
    if (mShowHexState) {
        QString hexText = data.toHex(' ');
        cur.insertText(hexText + ' ');
        setTextCursor(cur);
        return;
    }

    // ===== 文本模式 =====
    QString text = m_codec ? m_codec->toUnicode(data) : QString::fromUtf8(data);

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text.at(i);

        /****************  ANSI 颜色解析  ****************/
        if (m_inEscape) {
            m_ansiBuf += ch;
            // 序列以字母结尾
            if (ch.isLetter()) {
                m_inEscape = false;
                applyAnsi(m_ansiBuf);     // 真正设置 m_fmtCurrent
                m_ansiBuf.clear();
            }
            continue;                     // 颜色码本身不插入文本
        }
        if (ch == QChar(0x1B)) {          // ESC
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            m_inEscape = true;
            m_ansiBuf = ch;
            setTextCursor(cur);
            continue;
        }

        // --- 判断是否为控制字符 ---
        if( ch == '\b' ){
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            if (cur.positionInBlock() > 0)
                cur.movePosition(QTextCursor::Left);
            if(m_expectUpEcho) {
                keyState = KEY_UP_STATE;
            }else if(m_expectDownEcho){
                keyState = KEY_DOWN_STATE;
            }else if(m_expectLeftEcho){
                keyState = KEY_LEFT_STATE;
            }else if(m_expectRightEcho){
                keyState = KEY_RIGHT_STATE;
            }else{
                keyState = KEY_BACK_STATE;
            }
            setTextCursor(cur);
            continue;
        }

        if( ch == '\r' ){
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            // 移到行首
            cur.movePosition(QTextCursor::StartOfBlock);
            setTextCursor(cur);
            continue;
        }

        if( ch == '\n' ){
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            cur.movePosition(QTextCursor::EndOfBlock);
            cur.insertBlock();
            setTextCursor(cur);
            continue;
        }

        textData.append(ch);

        if (keyState == KEY_BACK_STATE) {
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            // 如果上一个是退格+空格组合，擦除并重写
            cur.deleteChar();
            if(ch == 0x20){
                keyState = KEY_NORMAL_STATE;
            }
            setTextCursor(cur);
            continue;
        }
        if (keyState == KEY_UP_STATE || keyState == KEY_DOWN_STATE) {
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            if(ch == 0x20){
                cur.deleteChar();
            }else{
                keyState = KEY_NORMAL_STATE;
            }
            setTextCursor(cur);
            continue;
        }

        if (keyState == KEY_LEFT_STATE) {
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            cur.deleteChar();
            setTextCursor(cur);
            continue;
        }

        if(keyState == KEY_RIGHT_STATE){
            cur.insertText(textData, m_fmtCurrent);
            textData.clear();
            cur.deleteChar();
            setTextCursor(cur);
            continue;
        }

    }

    cur.insertText(textData, m_fmtCurrent);
    textData.clear();
    setTextCursor(cur);
}


void Terminal::appendSendData(const QByteArray &ba)
{
    QByteArray data = ba;
    QString text = m_codec ? m_codec->toUnicode(data) : QString::fromUtf8(data);

    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::End);

    if(mShowDateState){
        QString dateTime = '\n' + QDateTime::currentDateTime()
                .toString("[yy-MM-dd hh:mm:ss.zzz]发->");
        cur.insertText(dateTime);
    }

    if(mShowHexState){
        QString hexShowTemp = data.toHex(' ')+" ";
        cur.insertText(hexShowTemp);
    }else{
        cur.insertText(text);
    }

    setTextCursor(cur);
}

void Terminal::setShowHexState(bool state)
{
    mShowHexState = state;
}

void Terminal::setShowDateState(bool state)
{
    mShowDateState = state;
}

void Terminal::onReadBytes(QByteArray bytes)
{
    appendData(bytes);
}

void Terminal::delData()
{
    if(isDeal) return;
    isDeal = true;
    while(1)
    {
        char tempBuf[1024];
        for(int i = 0;i<1024;i++){
            tempBuf[i] = m_ringBuffer.getData(i);
        }
        QByteArray bytes;
        //m_ringBuffer.read(bytes,m_ringBuffer.getLen());
        m_ringBuffer.pushRd(1024);
        appendData(tempBuf);  // 你的解析器
        if(m_ringBuffer.getLen() == 0)
        {
            break;
        }
    }

    isDeal = false;
}

bool Terminal::event(QEvent *ev)
{
    if (ev->type() == QEvent::InputMethod) {
        QInputMethodEvent *ime = static_cast<QInputMethodEvent *>(ev);
        if (!ime->commitString().isEmpty()) {
            // 中文已上屏，直接发串口
            QByteArray bytes;
            if (m_codec->name() == "GBK") {
                // GB18030 向下兼容 GBK， Qt5/6 都认
                static QTextCodec *gbk = QTextCodec::codecForName("GB18030");
                if (gbk) bytes = gbk->fromUnicode(ime->commitString());
                else     bytes = ime->commitString().toLocal8Bit();   // 兜底
            } else if (m_codec->name() == "UTF-8"){
                bytes = ime->commitString().toUtf8();                 // UTF-8
            }
            emit sendBytes(bytes);
        }
        return true;   // 自己处理完
    }
    return QPlainTextEdit::event(ev);
}

void Terminal::keyPressEvent(QKeyEvent *ev)
{
    QByteArray bytes;

    switch (ev->key()) {
    case Qt::Key_Up:
        bytes = "\x1B[A";
        m_expectUpEcho    = true;
        m_expectDownEcho  = false;
        m_expectRightEcho = false;
        m_expectLeftEcho  = false;
        break;
    case Qt::Key_Down:
        bytes = "\x1B[B";
        m_expectUpEcho    = false;
        m_expectDownEcho  = true;
        m_expectRightEcho = false;
        m_expectLeftEcho  = false;
        break;
    case Qt::Key_Right:
        bytes = "\x1B[C";
        m_expectUpEcho    = false;
        m_expectDownEcho  = false;
        m_expectRightEcho = true;
        m_expectLeftEcho  = false;
        break;
    case Qt::Key_Left:
        bytes = "\x1B[D";
        m_expectUpEcho    = false;
        m_expectDownEcho  = false;
        m_expectRightEcho = false;
        m_expectLeftEcho  = true;
        break;
    case Qt::Key_Backspace: bytes = QByteArray(1, '\b');m_expectEnterEcho = true;break;  // 有些系统是 '\b'
    case Qt::Key_Tab:       bytes = QByteArray(1, '\t'); break;    // Tab
    case Qt::Key_Return:
    case Qt::Key_Enter:     bytes = QByteArray(1, '\r'); break;    // 回车
    default:
        // 普通字符
        if (!ev->text().isEmpty())
            bytes = ev->text().toUtf8();
        break;
    }

    // Ctrl 组合键
    if (ev->modifiers() & Qt::ControlModifier) {
        switch (ev->key()) {
        case Qt::Key_C: bytes = QByteArray(1, 3); break;   // Ctrl+C
        case Qt::Key_Z: bytes = QByteArray(1, 26); break;  // Ctrl+Z
        case Qt::Key_D: bytes = QByteArray(1, 4); break;   // Ctrl+D
        default: break;
        }
    }

    if (!bytes.isEmpty()){
        emit sendBytes(bytes);
        key_down = true;
    }

}

void Terminal::insertFromMimeData(const QMimeData *source)
{
    if (source->hasText())
        emit sendBytes(source->text().toUtf8());
}


void Terminal::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        int pos = cursorForPosition(e->pos()).position();
        m_customSelection.start = m_customSelection.end = pos;
        m_customSelection.active = true;
        viewport()->update();          // 触发重绘
    }
}

void Terminal::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_customSelection.active) return;

    // 更新选中结束点
    m_customSelection.end = cursorForPosition(e->pos()).position();

    // ===== 自动滚动逻辑 =====
    const int margin = 10;  // 距离上下边界多少像素开始滚动
    auto* vBar = verticalScrollBar();

    if (e->pos().y() < margin)
        vBar->setValue(vBar->value() - 1);
    else if (e->pos().y() > viewport()->height() - margin)
        vBar->setValue(vBar->value() + 1);

    viewport()->update();
}

void Terminal::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_customSelection.active = false;
        viewport()->update();
    }
}

void Terminal::paintEvent(QPaintEvent *e)
{
    QPlainTextEdit::paintEvent(e);
    if (m_customSelection.start < 0 || m_customSelection.end < 0)
        return;

    int start = qMin(m_customSelection.start, m_customSelection.end);
    int end   = qMax(m_customSelection.start, m_customSelection.end);

    if (start == end)
        return;  // 无选区，不绘制

    QPainter p(viewport());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0,120,215,80));

    QTextCursor cur(document());
    cur.setPosition(start);

    int lastY = -1;
    QRect lineRect;

    for (int pos = start; pos <= end; ++pos) {
        cur.setPosition(pos);
        QRect r = cursorRect(cur);

        int charWidth = fontMetrics().horizontalAdvance(cur.document()->characterAt(pos));

        if (lastY == -1) {
            lastY = r.y();
            lineRect = r;
            lineRect.setRight(r.right() + charWidth);
        }
        else if (r.y() == lastY) {
            lineRect.setRight(r.right() + charWidth);
        }
        else {
            p.drawRect(lineRect);
            lastY = r.y();
            lineRect = r;
            lineRect.setRight(r.right() + charWidth);
        }
    }
    p.drawRect(lineRect);
}

void Terminal::copy()
{
    qDebug()<<"in copy";
    if (m_customSelection.start < 0 || m_customSelection.end < 0) return;
    int s = qMin(m_customSelection.start, m_customSelection.end);
    int e = qMax(m_customSelection.start, m_customSelection.end);
    QTextCursor cur(document());
    cur.setPosition(s);
    cur.setPosition(e, QTextCursor::KeepAnchor);

    // 获取选中文本
    QString sel = cur.selectedText();

    // ⭕️关键：把段落分隔符替换成标准换行
    sel.replace(QChar::ParagraphSeparator, "\r\n"); // Windows 风格

    // 或者 Linux 风格
    // sel.replace(QChar::ParagraphSeparator, "\n");

    QApplication::clipboard()->setText(sel);
}

void Terminal::paste()
{
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) return;

    const QMimeData *md = QApplication::clipboard()->mimeData();
    insertFromMimeData(md);           // ← 最终只会调这里
}

void Terminal::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e);
    QMenu menu(this);
    menu.addAction("复制", this, &Terminal::copy);
    menu.addAction("粘贴", this, &Terminal::paste);
    menu.exec(QCursor::pos());
}

void Terminal::applyAnsi(const QString &seq)
{
    if (!seq.startsWith("\x1B[")) return;
    QString body = seq.mid(2, seq.length() - 3);   // 去掉 ESC[ 和末尾字母

    if(m_convert_start){
        m_fmtCurrent = QTextCharFormat();   // 先全部清空
        m_fmtCurrent.setForeground(m_fmtDefault.foreground());
        m_fmtCurrent.setBackground(m_fmtDefault.background());
        // 如果有默认字体也拷回来
        m_fmtCurrent.setFont(m_fmtDefault.font());
        m_convert_start = false;
        return;
    }

    QStringList codes = body.split(';', QString::SkipEmptyParts);

    for (const QString &c : codes) {
        if (c == "0" || c.isEmpty()) {          // 真正 reset
            m_fmtCurrent = QTextCharFormat();   // 先全部清空
            m_fmtCurrent.setForeground(m_fmtDefault.foreground());
            m_fmtCurrent.setBackground(m_fmtDefault.background());
            // 如果有默认字体也拷回来
            m_fmtCurrent.setFont(m_fmtDefault.font());
            continue;
        }
        if (c == "1") {                         // 高亮/加粗
            m_fmtCurrent.setFontWeight(QFont::Bold);
            continue;
        }
        if (ansiColor.contains(c)) {            // 前景色 30-37
            m_fmtCurrent.setForeground(ansiColor[c]);
            continue;
        }
        // 还可以扩展 40-47 背景色、90-97 高亮前景色
    }

    m_convert_start = true;
}



