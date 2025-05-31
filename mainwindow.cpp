#include "mainwindow.h"
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    treeView(new QTreeView(this)),
    tableView(new QTableView(this)),
    pathEdit(new QLineEdit(this)),
    model(new QFileSystemModel(this)),

    drivesView(nullptr),
    drivesModel(nullptr),

    extensionEdit(nullptr),
    recursiveCheckBox(nullptr),
    searchButton(nullptr),
    resultsList(nullptr),
    includeNoExtensionCheckBox(nullptr),
    searchPanel(nullptr),
    toggleSearchButton(nullptr),
    contentEdit(nullptr),
    nameEdit(nullptr),
    progressBar(nullptr)
{
    initModel();
    initDrivesView();
    initTreeView();
    initTableView();
    initSearchWidgets();
    initLayout();
    initConnections();

    setWindowTitle("NGExplorer Alpha 2.0");
    resize(800, 600);
}

MainWindow::~MainWindow() = default;

void MainWindow::initModel()
{
    model->setRootPath("");
    model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    model->setReadOnly(false);
}

void MainWindow::initDrivesView()
{
    drivesModel = new QFileSystemModel(this);
    drivesModel->setFilter(QDir::Drives | QDir::NoDotAndDotDot);
    drivesModel->setRootPath("");

    drivesView = new QTreeView(this);
    drivesView->setModel(drivesModel);

    for (int i = 1; i < drivesModel->columnCount(); ++i) {
        drivesView->hideColumn(i);
    }
    drivesView->setHeaderHidden(true);
    drivesView->setMinimumWidth(40);

    drivesView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::initTreeView()
{
    treeView->setModel(model);

    for (int i = 1; i < model->columnCount(); ++i) {
        treeView->hideColumn(i);
    }
    treeView->setHeaderHidden(true);
    treeView->setMinimumWidth(200);

    QModelIndex rootIndex = model->index("");
    treeView->setRootIndex(rootIndex);
    treeView->setColumnWidth(0, 250);

    treeView->setDragDropMode(QAbstractItemView::DragDrop);
    treeView->setDefaultDropAction(Qt::CopyAction);
    treeView->setAcceptDrops(true);
    treeView->setDragEnabled(true);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::initTableView()
{
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->setDragDropMode(QAbstractItemView::DragDrop);
    tableView->setDefaultDropAction(Qt::CopyAction);
    tableView->setAcceptDrops(true);
    tableView->setDragEnabled(true);

    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::initSearchWidgets()
{
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Имя файла (частично)");

    extensionEdit = new QLineEdit(this);
    extensionEdit->setPlaceholderText("Например: .txt");

    contentEdit = new QLineEdit(this);
    contentEdit->setPlaceholderText("Поиск по содержимому (текст)");

    recursiveCheckBox = new QCheckBox("Рекурсивно", this);

    includeNoExtensionCheckBox = new QCheckBox("Включать файлы без расширения", this);

    searchButton = new QPushButton("Найти", this);

    resultsList = new QListWidget(this);
    resultsList->setMinimumHeight(100);

    toggleSearchButton = new QPushButton("🔍 Поиск", this);
    toggleSearchButton->setCheckable(true);
    toggleSearchButton->setToolTip("Показать или скрыть панель поиска");

    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 0);
    progressBar->setTextVisible(false);
    progressBar->setVisible(false);

    searchPanel = new QWidget(this);
    searchPanel->setVisible(false);
}

void MainWindow::initLayout()
{
    QWidget    *central    = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    QSplitter *topSplitter = new QSplitter(Qt::Horizontal, this);
    topSplitter->addWidget(drivesView);
    topSplitter->addWidget(treeView);
    topSplitter->addWidget(tableView);

    topSplitter->setStretchFactor(0, 0);
    topSplitter->setStretchFactor(1, 1);
    topSplitter->setStretchFactor(2, 2);

    QSplitter *searchSplitter = new QSplitter(Qt::Horizontal, this);

    QWidget   *filtersWidget  = new QWidget(this);
    QVBoxLayout *filtersLayout = new QVBoxLayout(filtersWidget);

    {
        QHBoxLayout *h = new QHBoxLayout;
        h->addWidget(new QLabel("Имя:", this));
        h->addWidget(nameEdit);
        filtersLayout->addLayout(h);
    }

    {
        QHBoxLayout *h = new QHBoxLayout;
        h->addWidget(new QLabel("Расширение:", this));
        h->addWidget(extensionEdit);
        filtersLayout->addLayout(h);
    }

    {
        QHBoxLayout *h = new QHBoxLayout;
        h->addWidget(new QLabel("Содержимое:", this));
        h->addWidget(contentEdit);
        filtersLayout->addLayout(h);
    }

    filtersLayout->addWidget(includeNoExtensionCheckBox);
    filtersLayout->addWidget(recursiveCheckBox);

    filtersLayout->addWidget(searchButton);
    filtersLayout->addStretch();


    QWidget   *resultsWidget = new QWidget(this);
    QVBoxLayout *resultsLayout = new QVBoxLayout(resultsWidget);
    resultsLayout->addWidget(resultsList);

    searchSplitter->addWidget(filtersWidget);
    searchSplitter->addWidget(resultsWidget);
    searchSplitter->setStretchFactor(0, 0);
    searchSplitter->setStretchFactor(1, 1);

    QVBoxLayout *searchPanelLayout = new QVBoxLayout(searchPanel);
    searchPanelLayout->addWidget(searchSplitter);

    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(topSplitter);
    mainSplitter->addWidget(searchPanel);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);

    mainLayout->addWidget(pathEdit);
    mainLayout->addWidget(toggleSearchButton);
    mainLayout->addWidget(mainSplitter);
    mainLayout->addWidget(progressBar);

    setCentralWidget(central);
}

void MainWindow::initConnections()
{
    connect(drivesView, &QTreeView::clicked, this, [this](const QModelIndex &idx) {
        QString diskPath = drivesModel->fileInfo(idx).absoluteFilePath();
        if (!diskPath.isEmpty()) {
            QModelIndex treeIndex = model->index(diskPath);
            treeView->setRootIndex(treeIndex);
            treeView->setCurrentIndex(treeIndex);

            tableView->setRootIndex(model->setRootPath(diskPath));

            pathEdit->setText(diskPath);
        }
    });

    connect(treeView, &QTreeView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    connect(tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    connect(treeView, &QTreeView::clicked, this, [this](const QModelIndex &index) {
        QString path = model->fileInfo(index).absoluteFilePath();
        if (!path.isEmpty()) {
            pathEdit->setText(path);
            tableView->setRootIndex(model->setRootPath(path));
        }
    });

    connect(pathEdit, &QLineEdit::returnPressed, this, [this]() {
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

    connect(searchButton, &QPushButton::clicked, this, [this]() {
        const QString name       = nameEdit->text().trimmed();
        const QString extension  = extensionEdit->text().trimmed();
        const QString content    = contentEdit->text().trimmed();
        const bool useName       = !name.isEmpty();
        const bool useExtension  = !extension.isEmpty();
        const bool useContent    = !content.isEmpty();
        const bool includeNoExt  = includeNoExtensionCheckBox->isChecked();
        const bool recursive     = recursiveCheckBox->isChecked();

        QString rootPath = model->fileInfo(treeView->currentIndex()).absoluteFilePath();
        if (rootPath.isEmpty()) {
            rootPath = QDir::rootPath();
        }

        resultsList->clear();
        searchButton->setEnabled(false);
        progressBar->setRange(0, 0);
        progressBar->setVisible(true);

        std::thread([=]() {
            std::vector<QString> foundPaths;
            QDirIterator::IteratorFlags flags = recursive
                                                    ? QDirIterator::Subdirectories
                                                    : QDirIterator::NoIteratorFlags;

            QDirIterator it(rootPath, QDir::Files | QDir::NoDotAndDotDot, flags);
            while (it.hasNext()) {
                const QString filePath = it.next();
                QFileInfo fileInfo(filePath);

                if (!fileInfo.isFile())
                    continue;

                if (useName && !fileInfo.fileName().contains(name, Qt::CaseInsensitive))
                    continue;

                if (useExtension) {
                    if (fileInfo.suffix().isEmpty()) {
                        if (!includeNoExt)
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

                foundPaths.push_back(filePath);
            }

            QMetaObject::invokeMethod(this, [=]() {
                if (foundPaths.empty()) {
                    resultsList->addItem("❌ Ничего не найдено");
                } else {
                    for (const QString &path : foundPaths) {
                        QFileInfo fi(path);
                        QIcon icon = iconProvider.icon(fi);
                        QListWidgetItem *item = new QListWidgetItem(icon, fi.fileName());
                        item->setToolTip(path);
                        item->setData(Qt::UserRole, path);
                        resultsList->addItem(item);
                    }
                }
                progressBar->setVisible(false);
                progressBar->setRange(0, 100);
                searchButton->setEnabled(true);
            }, Qt::QueuedConnection);

        }).detach();
    });

    connect(treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        QString path = model->filePath(index);
        QFileInfo info(path);
        if (info.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        QString path = model->filePath(index);
        QFileInfo info(path);
        if (info.isFile()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });

    connect(toggleSearchButton, &QPushButton::toggled, this, [this](bool checked) {
        searchPanel->setVisible(checked);
    });
    connect(toggleSearchButton, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            searchPanel->setMaximumHeight(QWIDGETSIZE_MAX);
            searchPanel->setMinimumHeight(100);
        } else {
            searchPanel->setMaximumHeight(0);
            searchPanel->setMinimumHeight(0);
        }
    });
}


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
