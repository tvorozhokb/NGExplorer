#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    treeView(new QTreeView(this)),
    tableView(new QTableView(this)),
    pathEdit(new QLineEdit(this)),
    model(new QFileSystemModel(this))
{
    model->setRootPath(QDir::rootPath());
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    model->setReadOnly(false);

    treeView->setModel(model);
    for (int i = 1; i < model->columnCount(); ++i)
        treeView->hideColumn(i);
    treeView->setHeaderHidden(true);
    treeView->setMinimumWidth(200);
    treeView->setRootIndex(model->index(QDir::rootPath()));
    treeView->setColumnWidth(0, 250);
    treeView->setDragDropMode(QAbstractItemView::DragDrop);
    treeView->setDefaultDropAction(Qt::CopyAction);
    treeView->setAcceptDrops(true);
    treeView->setDragEnabled(true);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);


    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setDragDropMode(QAbstractItemView::DragDrop);
    tableView->setDefaultDropAction(Qt::CopyAction);
    tableView->setAcceptDrops(true);
    tableView->setDragEnabled(true);
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    extensionEdit = new QLineEdit(this);
    extensionEdit->setPlaceholderText("Например: .txt");
    recursiveCheckBox = new QCheckBox("Рекурсивно", this);
    searchButton = new QPushButton("Найти", this);
    resultsList = new QListWidget(this);
    resultsList->setMinimumHeight(100);
    includeNoExtensionCheckBox = new QCheckBox("Включать файлы без расширения", this);

    toggleSearchButton = new QPushButton("🔍 Поиск", this);
    toggleSearchButton->setCheckable(true);
    toggleSearchButton->setToolTip("Показать или скрыть панель поиска");

    contentEdit = new QLineEdit(this);
    contentEdit->setPlaceholderText("Поиск по содержимому (текст)");

    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Имя файла (частично)");

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setTextVisible(false);
    progressBar->setVisible(false);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    QSplitter *splitter = new QSplitter(this);

    splitter->addWidget(treeView);
    splitter->addWidget(tableView);

    searchPanel = new QWidget(this);
    searchPanel->setVisible(false);

    QSplitter *searchSplitter = new QSplitter(Qt::Horizontal, this);

    QWidget *filtersWidget = new QWidget(this);
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersWidget);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(nameEdit);
    filtersLayout->addLayout(nameLayout);

    QHBoxLayout *extensionLayout = new QHBoxLayout;
    extensionLayout->addWidget(new QLabel("Расширение:"));
    extensionLayout->addWidget(extensionEdit);
    filtersLayout->addLayout(extensionLayout);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(new QLabel("Содержимое:"));
    contentLayout->addWidget(contentEdit);
    filtersLayout->addLayout(contentLayout);

    filtersLayout->addWidget(includeNoExtensionCheckBox);
    filtersLayout->addWidget(recursiveCheckBox);
    filtersLayout->addWidget(searchButton);
    filtersLayout->addStretch();

    QWidget *resultsWidget = new QWidget(this);
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsWidget);
    resultsLayout->addWidget(resultsList);

    searchSplitter->addWidget(filtersWidget);
    searchSplitter->addWidget(resultsWidget);
    searchSplitter->setStretchFactor(0, 0);
    searchSplitter->setStretchFactor(1, 1);

    QVBoxLayout *searchPanelLayout = new QVBoxLayout(searchPanel);
    searchPanelLayout->addWidget(searchSplitter);

    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(splitter);
    mainSplitter->addWidget(searchPanel);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);

    mainLayout->addWidget(pathEdit);
    mainLayout->addWidget(toggleSearchButton);
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(pathEdit);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(progressBar);

    setCentralWidget(central);
    setWindowTitle("NGExplorer Alpha 1.0");
    resize(800, 600);

    connect(treeView, &QTreeView::clicked, this, [=](const QModelIndex &index) {
        QString path = model->fileInfo(index).absoluteFilePath();
        pathEdit->setText(path);
        tableView->setRootIndex(model->setRootPath(path));
    });

    connect(pathEdit, &QLineEdit::returnPressed, this, [=]() {
        QString path = pathEdit->text();
        QFileInfo info(path);
        if (info.exists() && info.isDir()) {
            QModelIndex dirIndex = model->index(path);
            treeView->setCurrentIndex(dirIndex);
            treeView->scrollTo(dirIndex);
            tableView->setRootIndex(model->setRootPath(path));
        } else {
            QMessageBox::warning(this, "Ошибка", "Путь не существует или не является директорией.");

            QModelIndex currentIndex = treeView->currentIndex();
            QString currentPath = model->fileInfo(currentIndex).absoluteFilePath();
            pathEdit->setText(currentPath);
        }
    });

    connect(searchButton, &QPushButton::clicked, this, [=]() {
        QString name = nameEdit->text().trimmed();
        QString extension = extensionEdit->text().trimmed();
        QString content = contentEdit->text().trimmed();

        bool useName = !name.isEmpty();
        bool useExtension = !extension.isEmpty();
        bool useContent = !content.isEmpty();
        bool includeNoExtension = includeNoExtensionCheckBox->isChecked();

        resultsList->clear();
        searchButton->setEnabled(false);
        progressBar->setRange(0, 0);
        progressBar->setVisible(true);

        QTimer::singleShot(100, this, [=]() {
            QString rootPath = model->fileInfo(treeView->currentIndex()).absoluteFilePath();
            QDirIterator::IteratorFlags flags = recursiveCheckBox->isChecked()
                                                    ? QDirIterator::Subdirectories
                                                    : QDirIterator::NoIteratorFlags;

            QDirIterator it(rootPath, QDir::Files | QDir::NoDotAndDotDot, flags);
            while (it.hasNext()) {
                QString filePath = it.next();
                QFileInfo fileInfo(filePath);

                if (!fileInfo.isFile())
                    continue;

                if (useName && !fileInfo.fileName().contains(name, Qt::CaseInsensitive))
                    continue;

                if (useExtension) {
                    if (fileInfo.suffix().isEmpty()) {
                        if (!includeNoExtension)
                            continue;
                    } else {
                        QString extClean = extension;
                        if (extClean.startsWith('.'))
                            extClean.remove(0, 1);

                        if (fileInfo.suffix().compare(extClean, Qt::CaseInsensitive) != 0)
                            continue;
                    }
                }

                if (useContent) {
                    QFile file(filePath);
                    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                        continue;

                    QTextStream in(&file);
                    bool found = false;
                    while (!in.atEnd()) {
                        QString line = in.readLine();
                        if (line.contains(content, Qt::CaseInsensitive)) {
                            found = true;
                            break;
                        }
                    }
                    file.close();

                    if (!found)
                        continue;
                }

                QIcon icon = iconProvider.icon(fileInfo);

                QListWidgetItem *item = new QListWidgetItem(icon, fileInfo.fileName());
                item->setToolTip(fileInfo.absoluteFilePath());
                item->setData(Qt::UserRole, fileInfo.absoluteFilePath());

                resultsList->addItem(item);

            }

            if (resultsList->count() == 0) {
                resultsList->addItem("❌ Ничего не найдено");
            }

            progressBar->setVisible(false);
            progressBar->setRange(0, 100);
            searchButton->setEnabled(true);

        });
    });
    connect(resultsList, &QListWidget::itemActivated, this, [=](QListWidgetItem *item) {
        QString filePath = item->data(Qt::UserRole).toString();
        if (!filePath.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    });


    connect(treeView, &QTreeView::doubleClicked, this, [=](const QModelIndex &index) {
        QString path = model->filePath(index);
        QFileInfo info(path);
        if (info.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(tableView, &QTableView::doubleClicked, this, [=](const QModelIndex &index) {
        QString path = model->filePath(index);
        QFileInfo info(path);
        if (info.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(toggleSearchButton, &QPushButton::toggled, this, [=](bool checked) {
        searchPanel->setVisible(checked);
    });

    connect(toggleSearchButton, &QPushButton::toggled, this, [=](bool checked) {
        if (checked) {
            searchPanel->setMaximumHeight(16777215);
            searchPanel->setMinimumHeight(100);
        } else {
            searchPanel->setMaximumHeight(0);
            searchPanel->setMinimumHeight(0);
        }
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::showContextMenu(const QPoint &pos)
{
    QAbstractItemView *view = qobject_cast<QAbstractItemView *>(sender());
    if (!view) return;

    QModelIndex index = view->indexAt(pos);
    if (!index.isValid()) {
        QMessageBox::information(this, "Контекстное меню", "Ничего не выбрано.");
        return;
    }

    view->setCurrentIndex(index);

    QMenu contextMenu(this);

    QAction *deleteAction = new QAction("Удалить", this);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedFile);
    contextMenu.addAction(deleteAction);

    QAction *renameAction = new QAction("Переименовать", this);
    connect(renameAction, &QAction::triggered, this, &MainWindow::renameSelectedFile);
    contextMenu.addAction(renameAction);

    contextMenu.exec(view->viewport()->mapToGlobal(pos));
}

void MainWindow::renameSelectedFile() {
    QModelIndex index;

    QModelIndex treeIndex = treeView->currentIndex();
    QModelIndex tableIndex = tableView->currentIndex();

    if (treeIndex.isValid()) {
        index = treeView->currentIndex();
    } else if (tableIndex.isValid()) {
        index = tableView->currentIndex();
    }

    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Элемент не выбран.");
        return;
    }

    QFileSystemModel *model = qobject_cast<QFileSystemModel *>(tableView->model());

    QString oldPath = model->filePath(index);
    QFileInfo fileInfo(oldPath);

    QString baseName = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();

    QString initialText = baseName;

    bool ok;
    QString newBaseName = QInputDialog::getText(this, "Переименование",
                                                "Новое имя:", QLineEdit::Normal,
                                                initialText, &ok);

    if (ok && !newBaseName.isEmpty() && newBaseName != baseName) {
        QString newName = extension.isEmpty() ? newBaseName : newBaseName + "." + extension;
        QString newPath = fileInfo.dir().absoluteFilePath(newName);

        if (!QFile::rename(oldPath, newPath)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось переименовать файл.");
        }
    }
}

void MainWindow::deleteSelectedFile()
{
    QModelIndex treeIndex = treeView->currentIndex();
    QModelIndex tableIndex = tableView->currentIndex();

    QModelIndex selectedIndex;
    if (treeIndex.isValid()) {
        selectedIndex = treeIndex;

    } else if (tableIndex.isValid()) {
        selectedIndex = tableIndex;

    } else {
        QMessageBox::information(this, "Удаление", "Ничего не выбрано.");
        return;
    }

    QFileSystemModel *model = qobject_cast<QFileSystemModel *>(tableView->model());
    QString filePath = model->filePath(selectedIndex);

    QFileInfo info(filePath);
    QString msg = QString("Вы уверены, что хотите удалить \"%1\"?").arg(info.fileName());

    if (QMessageBox::question(this, "Подтверждение удаления", msg) == QMessageBox::Yes)
    {
        bool success = false;
        if (info.isDir())
            success = QDir(filePath).removeRecursively();
        else
            success = QFile::remove(filePath);

        if (!success)
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить файл или папку.");
    }
}
