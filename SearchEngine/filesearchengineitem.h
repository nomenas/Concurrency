#ifndef FILESEARCHENGINEITEM_H
#define FILESEARCHENGINEITEM_H

#include <QVariant>
#include <QString>

class FileSearchEngineItem
{
public:
    FileSearchEngineItem(const QString& filepath, const QString& text, int options)
        : m_filepath(filepath), m_text(text), m_options(options) {}
    virtual ~FileSearchEngineItem() = default;

    QString filepath() const
    {
        return m_filepath;
    }

    QString text() const
    {
        return m_text;
    }

    int options() const
    {
        return m_options;
    }

    virtual bool open() = 0;
    virtual bool findNext() = 0;
    virtual void close() = 0;
    virtual QVariant result() const = 0;

protected:
    QString m_filepath;
    QString m_text;
    int m_options = 0;
};

#endif // FILESEARCHENGINEITEM_H

