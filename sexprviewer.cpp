#include "sexprviewer.h"
#include "ui_sexprviewer.h"

#include "sexpr/sexpr_parser.h"

#include <QDebug>
#include <QFileDialog>

#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMimeData>
#include <QStyle>
#include <QScreen>

using namespace sexpr;

QDebug operator<<(QDebug out, const std::string &str)
{
    out << str.c_str();
    return out;
}


SexprViewer::SexprViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SexprViewer)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 2);
    ui->plainTextEdit->setReadOnly(true);

    setMinimumSize(QSize(640, 450));
    setGeometry(QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    size(),
                    screen()->availableGeometry()
                ));
    setAcceptDrops(true);

    if(QSysInfo::productType() == "ubuntu") {
        QTextDocument *doc = ui->plainTextEdit->document();
        QFont font = doc->defaultFont();
        font.setFamily("Ubuntu Mono");
        ui->fontComboBox->setCurrentFont(font);
    }

    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, this, [&](QTreeWidgetItem * item) {
        if(item) {
            ui->plainTextEdit->setPlainText(item->data(0, Qt::UserRole).toString());
            ui->sexprPath->setText(getSexprPath(item));
        }
    });
}

SexprViewer::~SexprViewer()
{
    delete ui;
}

void SexprViewer::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls()) {
        event->accept();
    }
}

void SexprViewer::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();
    foreach(auto url, urls) {
        fileName = url.toLocalFile();
        break;
    }
    loadSexpr();
}


void SexprViewer::on_pushButtonOpen_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, "Open s-expression file", lastPath);
    if(!fileName.isEmpty()) {
        loadSexpr();
    }
}

void SexprViewer::on_pushButtonReload_clicked()
{
    loadSexpr();
}

QTreeWidgetItem *SexprViewer::buildTree(SEXPR *child, QTreeWidgetItem *tree)
{

    // s-expression must be a list and the first item must be symbol type
    std::string symbol = "";
    if(child->GetChild(0)->IsSymbol()) {
        symbol = child->GetChild(0)->GetSymbol();
    } else if(child->GetChild(0)->IsInteger()) {
        symbol = child->GetChild(0)->AsString();
    }

    QTreeWidgetItem *subTree = new QTreeWidgetItem({QString::fromStdString(symbol)});
    subTree->setData(0, Qt::UserRole, QString::fromStdString(child->AsString()));

    if(tree == nullptr) {
        tree =  subTree;
    } else {
        tree->addChild(subTree);
    }

    const SEXPR_VECTOR *list = child->GetChildren();
    for(SEXPR *it : *list) {
        if(it->IsList()) {
            buildTree(it, subTree);
        }
    }
//    for( std::vector<SEXPR *>::const_iterator it = list->begin(); it != list->end(); ++it ) {
//        if ((*it)->IsList()) {
//            buildTree(*it,subTree);
//        }
//    }
    return tree;
}

void SexprViewer::loadSexpr()
{
    PARSER parser;
    sexpr = parser.ParseFromFile(fileName.toStdString());

    root  = buildTree(sexpr.get());
    QTreeWidgetItem *top = ui->treeWidget->takeTopLevelItem(0);
    ui->treeWidget->addTopLevelItem(root);
    ui->treeWidget->setCurrentItem(root);
    ui->filePath->setText(fileName);
    if (!ui->pushButtonFold->isEnabled()) {
        ui->pushButtonFold->setEnabled(true);
    }

    if(top) {
        delete top;
    }
    sexpr.reset();
    lastPath = QFileInfo(fileName).path();
}

const QString SexprViewer::getSexprPath(QTreeWidgetItem *item, QString path)
{
    QString ss = item->text(0);
    if(!path.isEmpty()) {
        ss.append(path);
    }
    QTreeWidgetItem *parent = item->parent();
    if(parent) {
        ss.prepend(" >> ");
        ss = getSexprPath(parent, ss);
    }
    return ss;
}

void SexprViewer::on_fontComboBox_currentFontChanged(const QFont &f)
{
    QTextDocument *doc = ui->plainTextEdit->document();
    doc->setDefaultFont(f);
}

void SexprViewer::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);

    QAction *actionSymbol = new QAction("Copy Symbol", this);
    connect(actionSymbol, &QAction::triggered, [&]() {
        QClipboard *board = QApplication::clipboard();
        board->setText(item->text(0));
    });
    QAction *actionNode = new QAction("Copy Node", this);
    connect(actionNode, &QAction::triggered, [&]() {
        QClipboard *board = QApplication::clipboard();
        board->setText(item->data(0, Qt::UserRole).toString());
    });
    QAction *actionPath = new QAction("Copy Path", this);
    connect(actionPath, &QAction::triggered, [&]() {
        QClipboard *board = QApplication::clipboard();
        board->setText(ui->sexprPath->text());
    });

    QMenu menu(this);
    menu.addAction(actionSymbol);
    menu.addAction(actionNode);
    menu.addAction(actionPath);

    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

void SexprViewer::on_pushButtonFold_clicked()
{
    if (ui->pushButtonFold->text().startsWith("Expand")) {
        ui->treeWidget->expandAll();
        ui->pushButtonFold->setText("Collapse All");
    } else {
        ui->treeWidget->collapseAll();
        ui->pushButtonFold->setText("Expand All");
    }
}
