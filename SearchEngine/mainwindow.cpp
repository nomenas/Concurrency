#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filesearchengineitem.h"

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

class ConcreteSearch : public FileSearchEngineItem
{
public:
    using FileSearchEngineItem::FileSearchEngineItem;

    bool open() override
    {
        std::srand(std::time(0));
        m_matches = qAbs(std::rand() % 10000);
        return true;
    }

    bool findNext() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return ++m_current < m_matches;
    }

    void close() override
    {
    }

    QVariant result() const override
    {
        return QVariant(m_current);
    }

private:
    int m_matches = 0;
    int m_current = 0;
};

void MainWindow::on_pushButton_clicked()
{
    if (ui->pushButton->text() == "Search")
    {
        ui->pushButton->setText("Cancel");
        m_searchEngine.reset(new FileSearchEngine("/Users/nomenas/Downloads", QStringList() << "*.pdf",
            [](const QString& filepath, const QString& text, int options) -> FileSearchEngineItem*
            {
                return new ConcreteSearch(filepath, text, options);
            }
        ));
        connect(m_searchEngine.data(), &FileSearchEngine::matchFileFound, this, &MainWindow::onMatchFileFound);
        connect(m_searchEngine.data(), &FileSearchEngine::fileSearchItemFound, this, &MainWindow::onFileSearchItemFound);
        connect(m_searchEngine.data(), &FileSearchEngine::searchDone, this, &MainWindow::onSearchDone);

        m_searchEngine->find("Hello", 0);
    }
    else if (m_searchEngine)
    {
        m_searchEngine->cancel();
        m_searchEngine.reset();
    }
}

void MainWindow::onMatchFileFound(const QString& filepath)
{
    ui->textBrowser->append(filepath + " found!");
}

void MainWindow::onFileSearchItemFound(const QString& filepath, const QVariant& result)
{
    ui->textBrowser->append(filepath + QString(": %1").arg(result.toInt()));
}

void MainWindow::onSearchDone()
{
    ui->textBrowser->append("search done!");
    ui->pushButton->setText("Search");
}
