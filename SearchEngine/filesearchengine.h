#ifndef FILESEARCHENGINE_H
#define FILESEARCHENGINE_H

#include "filesearchengineitem.h"

#include <QObject>
#include <QStringList>

class QRunnable;

class FileSearchEngine : public QObject
{
    Q_OBJECT

public:
    using ItemFactory = std::function<FileSearchEngineItem*(const QString& filepath, const QString& text, int options)>;
    using Results = QList<QList<QVariant> >;

    explicit FileSearchEngine(const QString& path, const QStringList& nameFilters, ItemFactory itemFactory);
    ~FileSearchEngine();

    void find(const QString& text, int options = 0);

    void cancel(int msecs = 10000);
    bool isCanceled() const;

    Results results() const;

private slots:
    void taskDone(QRunnable* task);

private:
    void startTask(QRunnable* task);

    int m_activeTasks = 0;
    QString m_path;
    QStringList m_nameFilters;
    QString m_text;
    int m_options = 0;
    bool m_bIsCanceled = false;
    ItemFactory m_itemFactory;
    Results m_results;
};

#endif // FILESEARCHENGINE_H
