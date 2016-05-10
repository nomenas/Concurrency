#include "filesearchengine.h"

#include <QRunnable>
#include <QThreadPool>
#include <QMetaObject>
#include <QDir>
#include <QElapsedTimer>
#include <QPointer>
#include <QCoreApplication>
#include <QDebug>

class FindFilesTask : public QRunnable
{
public:
    FindFilesTask(QPointer<FileSearchEngine> engine, const QString& path, const QStringList& nameFilters)
        : m_path(path)
        , m_engine(engine)
        , m_nameFilter(nameFilters)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        filterDocuments(m_path);

        if (m_engine)
        {
            QMetaObject::invokeMethod(m_engine, "taskDone", Q_ARG(QRunnable*, this));
        }
    }

    void filterDocuments(const QString& path)
    {
        if (m_engine && m_engine->isCanceled()) return;

        QDir currentDir(path);
        for (auto item : currentDir.entryList(m_nameFilter, QDir::Files))
        {
            m_result << currentDir.filePath(item);
        }

        for (auto item : currentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            filterDocuments(currentDir.filePath(item));
        }
    }

    QStringList result() const
    {
        return m_result;
    }

private:
    QString m_path;
    QPointer<FileSearchEngine> m_engine;
    QStringList m_result;
    QStringList m_nameFilter;
};

class SearchFileTask : public QRunnable
{
public:
    SearchFileTask(QPointer<FileSearchEngine> engine, FileSearchEngineItem* item)
        : m_engine(engine)
        , m_pItem(item)
    {
        setAutoDelete(false);
    }

    void run() override
    {
        if (m_engine && m_engine->isCanceled()) return;

        if (m_pItem && m_pItem->open())
        {
            while (m_pItem->findNext())
            {
                if (m_engine && m_engine->isCanceled()) break;
                m_results << m_pItem->result();
            }
            m_pItem->close();
        }

        if (m_engine)
        {
            QMetaObject::invokeMethod(m_engine, "taskDone", Q_ARG(QRunnable*, this));
        }
    }

    FileSearchEngineItem* item() const
    {
        return m_pItem;
    }

    QList<QVariant> results() const
    {
        return m_results;
    }

private:
    QList<QVariant> m_results;
    QPointer<FileSearchEngine> m_engine;
    FileSearchEngineItem* m_pItem;
};

FileSearchEngine::FileSearchEngine(const QString& path, const QStringList& nameFilters, ItemFactory itemFactory)
    : m_path(path)
    , m_nameFilters(nameFilters)
    , m_itemFactory(itemFactory)
{
    qRegisterMetaType<QRunnable*>("QRunnable*");
}

FileSearchEngine::~FileSearchEngine()
{
    cancel();
}

void FileSearchEngine::startTask(QRunnable* task)
{
    ++m_activeTasks;
    QThreadPool::globalInstance()->start(task);
}

void FileSearchEngine::find(const QString& text, int options /*= 0*/)
{
    startTask(new FindFilesTask(this, m_path, m_nameFilters));
    m_text = text;
    m_options = options;
}

void FileSearchEngine::taskDone(QRunnable* task)
{
    if (!isCanceled())
    {
        if (FindFilesTask* item = dynamic_cast<FindFilesTask*>(task))
        {
            for (auto filepath : item->result())
            {
                startTask(new SearchFileTask(this, m_itemFactory(filepath, m_text, m_options)));
            }
        }
        else if (SearchFileTask* item = dynamic_cast<SearchFileTask*>(task))
        {
            m_results << item->results();
        }
    }

    --m_activeTasks;
    delete task;
}

void FileSearchEngine::cancel(int msecs /*= -1*/)
{
    m_bIsCanceled = true;

    QElapsedTimer timer;
    timer.start();
    while (m_activeTasks > 0 && (msecs == -1 || timer.elapsed() < msecs))
    {
        QCoreApplication::processEvents();
    }
}

bool FileSearchEngine::isCanceled() const
{
    return m_bIsCanceled;
}

FileSearchEngine::Results FileSearchEngine::results() const
{
    return m_results;
}
