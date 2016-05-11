#include "filesearchengine.h"

#include <QRunnable>
#include <QThreadPool>
#include <QMetaObject>
#include <QDir>
#include <QElapsedTimer>
#include <QPointer>
#include <QCoreApplication>

static const int MatchFileFoundID = 0;
static const int FileSearchItemFoundID = 1;

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
            QString filepath = currentDir.filePath(item);
            QMetaObject::invokeMethod(m_engine, "taskProgress",
                Q_ARG(QRunnable*, this), Q_ARG(int, MatchFileFoundID), Q_ARG(QVariant, filepath));
        }

        for (auto item : currentDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            filterDocuments(currentDir.filePath(item));
        }
    }

private:
    QString m_path;
    QPointer<FileSearchEngine> m_engine;
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

    ~SearchFileTask()
    {
        if (m_pItem)
        {
            delete m_pItem;
            m_pItem = nullptr;
        }
    }

    void run() override
    {
        if (m_engine)
        {
            if (m_engine->isCanceled())
            {
                QMetaObject::invokeMethod(m_engine, "taskDone", Q_ARG(QRunnable*, this));
            }
            else if (m_pItem && m_pItem->open())
            {
                while (m_pItem->findNext())
                {
                    if (m_engine && m_engine->isCanceled()) break;
                    QMetaObject::invokeMethod(m_engine, "taskProgress",
                        Q_ARG(QRunnable*, this), Q_ARG(int, FileSearchItemFoundID), Q_ARG(QVariant, m_pItem->result()));
                }

                m_pItem->close();

                if (m_engine)
                {
                    QMetaObject::invokeMethod(m_engine, "taskDone", Q_ARG(QRunnable*, this));
                }
            }
        }
    }

    FileSearchEngineItem* item() const
    {
        return m_pItem;
    }

private:
    QPointer<FileSearchEngine> m_engine;
    FileSearchEngineItem* m_pItem = nullptr;
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
    --m_activeTasks;
    delete task;

    if (m_activeTasks == 0)
    {
        emit searchDone();
    }
}

void FileSearchEngine::taskProgress(QRunnable *task, int code, QVariant arg)
{
    if (!isCanceled())
    {
        if (code == MatchFileFoundID)
        {
            QString filename = arg.toString();
            startTask(new SearchFileTask(this, m_itemFactory(filename, m_text, m_options)));
            emit matchFileFound(filename);
        }
        else if (code == FileSearchItemFoundID)
        {
            SearchFileTask* searchTask = dynamic_cast<SearchFileTask*>(task);
            if (searchTask && searchTask->item())
            {
                emit fileSearchItemFound(searchTask->item()->filepath(), arg);
            }
        }
    }
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
