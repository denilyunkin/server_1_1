//mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    connect(ui->pushButton, &QPushButton::toggled, this, &MainWindow::on_pushButton_toggled);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::onTreeViewClicked);

    ui->pushButton->setCheckable(true); // Начальное состояние
    ui->pushButton->setText(">>");
    while (ui->tabWidget->count() > 0) {
        ui->tabWidget->removeTab(0);
    }
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setMovable(true);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        ui->tabWidget->removeTab(index);
    });

    ui->tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabWidget, &QTabWidget::customContextMenuRequested, this, [this](const QPoint &pos){
        int index = ui->tabWidget->tabBar()->tabAt(pos);
        if(index == -1) return;

        QMenu menu;
        menu.addAction("Закрыть", [this,index](){
            QWidget *w = ui->tabWidget->widget(index);
            ui->tabWidget->removeTab(index);
            delete w;
        });
        menu.addAction("Закрыть все", [this](){
            while(ui->tabWidget->count() > 0){
                QWidget *w = ui->tabWidget->widget(0);
                ui->tabWidget->removeTab(0);
                delete w;
            }
        });
        menu.addAction("Закрыть остальные", [this,index](){
            for(int i = ui->tabWidget->count()-1; i>=0; --i){
                if(i != index){
                    QWidget *w = ui->tabWidget->widget(i);
                    ui->tabWidget->removeTab(i);
                    delete w;
                }
            }
        });
        menu.exec(ui->tabWidget->mapToGlobal(pos));
    });

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processJsonMessage(const QJsonObject &json) {
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Имя"});

    std::function<void(QStandardItem*, const QJsonObject&, int)> buildTree;
    buildTree = [&buildTree](QStandardItem *parent, const QJsonObject &node, int level) {
        if (node.contains("folders")) {
            for (const QJsonValue &folder : node["folders"].toArray()) {
                QJsonObject folderObj = folder.toObject();
                QStandardItem *folderItem = new QStandardItem(folderObj["name"].toString());
                folderItem->setData(folderObj["path"].toString(), Qt::UserRole);
                parent->appendRow(folderItem);
                buildTree(folderItem, folderObj, level + 1);
            }
        }

        if (node.contains("files")) {
            for (const QJsonValue &file : node["files"].toArray()) {
                QStandardItem *fileItem = new QStandardItem(file["name"].toString());
                parent->appendRow(fileItem);
            }
        }
    };

    QStandardItem *rootItem = new QStandardItem(json["name"].toString());
    rootItem->setData(json["path"].toString(), Qt::UserRole);
    model->appendRow(rootItem);
    buildTree(rootItem, json, 0);

    ui->treeView->setModel(model);

    // Раскрываем только корень и первый уровень
    ui->treeView->expandAll();  // Сначала раскрываем всё
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex parentIndex = model->index(i, 0);
        for (int j = 0; j < model->rowCount(parentIndex); ++j) {
            ui->treeView->collapse(model->index(j, 0, parentIndex));  // Сворачиваем второй уровень
        }
    }
}

void MainWindow::processNoJsonMessage(const QString &message) {
    ui->textBrowser->append(message);
}


void MainWindow::on_pushButton_toggled(bool checked) {
    if (checked) {
        ui->treeView->setVisible(false);
        ui->textBrowser->setVisible(false);
        ui->pushButton->setText(">>");
    } else {
        ui->treeView->setVisible(true);
        ui->textBrowser->setVisible(true);
        ui->pushButton->setText("<<");
    }
}

void MainWindow::onTreeViewClicked(const QModelIndex &index) {
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->treeView->model());
    if (!model) return;

    QStandardItem *item = model->itemFromIndex(index);

    // Получаем полный путь из данных элемента (Qt::UserRole + 1)
    QString filePath = item->data(Qt::UserRole + 1).toString();  // Используем +1, чтобы не пересекаться с другими данными

    // Если путь не задан, пытаемся собрать его из иерархии (запасной вариант)
    if (filePath.isEmpty()) {
        QStandardItem *currentItem = item;
        QStringList pathParts;
        while (currentItem) {
            pathParts.prepend(currentItem->text());
            currentItem = currentItem->parent();
        }
        filePath = "C:/" + pathParts.join("/");  // Базовый путь + относительный
        filePath = QDir::toNativeSeparators(filePath);  // Не "/"!! Проблема? - "\"
    }

    // Проверяем существование файла
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "Файл не найден:" << filePath;
        return;
    }

    // Открываем файл и добавляем вкладку
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextEdit *textEdit = new QTextEdit();
        textEdit->setPlainText(file.readAll());
        file.close();

        int newTabIndex = ui->tabWidget->addTab(textEdit, fileInfo.fileName());
        ui->tabWidget->setCurrentIndex(newTabIndex);
    } else {
        qWarning() << "Ошибка при открытии файла:" << filePath;
    }
}
