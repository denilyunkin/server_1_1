//mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processJsonMessage(const QJsonObject &json) {
    // Здесь реализуйте processJsonTree или перенесите его код сюда
    // Например:
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Имя", "Размер", "Дата изменения"});

    // Рекурсивная функция для заполнения модели
    std::function<void(QStandardItem*, const QJsonObject&)> buildTree;
    buildTree = [&buildTree](QStandardItem *parent, const QJsonObject &node) {
        // Добавляем подпапки
        if (node.contains("folders")) {
            for (const QJsonValue &folder : node["folders"].toArray()) {
                QJsonObject folderObj = folder.toObject();
                QStandardItem *folderItem = new QStandardItem(folderObj["name"].toString());
                folderItem->setData(folderObj["path"].toString(), Qt::UserRole); // Сохраняем путь
                parent->appendRow(folderItem);
                buildTree(folderItem, folderObj);
            }
        }

        // Добавляем файлы
        if (node.contains("files")) {
            for (const QJsonValue &file : node["files"].toArray()) {
                QJsonObject fileObj = file.toObject();
                QList<QStandardItem*> items;
                items << new QStandardItem(fileObj["name"].toString());
                items << new QStandardItem(QString::number(fileObj["size"].toInt()));
                items << new QStandardItem(fileObj["lastModified"].toString());
                parent->appendRow(items);
            }
        }
    };

    // Заполняем корень
    QStandardItem *rootItem = new QStandardItem(json["name"].toString());
    rootItem->setData(json["path"].toString(), Qt::UserRole);
    model->appendRow(rootItem);
    buildTree(rootItem, json);

    // Устанавливаем модель в treeView
    ui->treeView->setModel(model);
    ui->treeView->expandAll(); // Раскрываем все узлы
}

void MainWindow::processNoJsonMessage(const QString &message) {
    ui->textBrowser->append(message);
}
