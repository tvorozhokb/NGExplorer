#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QTableView>
#include <QLineEdit>
#include <QSplitter>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QDirIterator>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QFileIconProvider>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDesktopServices>
#include <QUrl>
#include <QMenu>
#include <QInputDialog>


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showContextMenu(const QPoint &pos);
    void deleteSelectedFile();
    void renameSelectedFile();

private:
    QTreeView *treeView;
    QTableView *tableView;
    QLineEdit *pathEdit;

    QLineEdit *extensionEdit;
    QCheckBox *recursiveCheckBox;
    QPushButton *searchButton;
    QListWidget *resultsList;

    QCheckBox *includeNoExtensionCheckBox;

    QWidget *searchPanel;
    QPushButton *toggleSearchButton;

    QLineEdit *contentEdit;

    QLineEdit *nameEdit;

    QProgressBar* progressBar;

    QFileIconProvider iconProvider;

    QFileSystemModel *model;

};

#endif // MAINWINDOW_H
