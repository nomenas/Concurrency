#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filesearchengineitem.h"

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

class PDFSearch : public FileSearchEngineItem
{
public:
    using FileSearchEngineItem::FileSearchEngineItem;

    bool open() override
    {
        return true;
    }
    bool findNext() const override
    {
        return true;
    }

    void close() override
    {
    }

    QVariant result() const override
    {
        return QVariant();
    }
};

void MainWindow::on_pushButton_clicked()
{
    QApplication* app = dynamic_cast<QApplication*>(QCoreApplication::instance());
    ui->btnCancel->setCursor(Qt::ArrowCursor);
    app->setOverrideCursor(Qt::WaitCursor);
    m_searchEngine.reset(new FileSearchEngine("/Users/nomenas/Downloads", QStringList() << "*.pdf",
        [](const QString& filepath, const QString& text, int options) -> FileSearchEngineItem*
        {
            return new PDFSearch(filepath, text, options);
        }
    ));
    m_searchEngine->find("Hello", 0);
}

void MainWindow::on_btnCancel_clicked()
{
    QApplication* app = dynamic_cast<QApplication*>(QCoreApplication::instance());
    if (m_searchEngine)
    {
        m_searchEngine->cancel();
        m_searchEngine.reset();
    }
    app->restoreOverrideCursor();
}
