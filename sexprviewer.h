#ifndef SEXPRVIEWER_H
#define SEXPRVIEWER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SexprViewer; }
namespace sexpr { class SEXPR; }
QT_END_NAMESPACE

class QTreeWidgetItem;

class SexprViewer : public QWidget
{
    Q_OBJECT

public:
    SexprViewer(QWidget *parent = nullptr);
    ~SexprViewer();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
private slots:
    void on_pushButtonOpen_clicked();

    void on_pushButtonReload_clicked();

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

    void on_pushButtonFold_clicked();

private:
    Ui::SexprViewer *ui;

    std::unique_ptr<sexpr::SEXPR> sexpr;
    QString fileName;
    QString lastPath = "";

    QTreeWidgetItem *root;
    QTreeWidgetItem * buildTree(sexpr::SEXPR *child,QTreeWidgetItem *tree = nullptr);
    void loadSexpr();

    const QString getSexprPath(QTreeWidgetItem *item,QString path = "");

};
#endif // SEXPRVIEWER_H
