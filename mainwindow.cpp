#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "expr.h"
#include <vector>
#include "solve.h"
#include <iostream>
#include <QApplication>
#include <QProcess>
#include <QMessageBox>

#include "common.h"
#include "matrix.h"

#include <cmath>
#include <stdexcept>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tableFuncs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableAns->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableEval->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void showError(const std::exception& err) {
	QMessageBox box;
	box.critical(0, "Error", err.what());
	box.setFixedSize(500, 200);
}

void MainWindow::on_addButton_clicked()
{
	QString fx = ui->plainTextFunctionEdit->toPlainText();
	std::string stringFx = fx.toStdString();
	Expr expFx;
	try {
		expFx = Expr::parse(stringFx);
	}
	catch (const ParseError& err) {
		showError(err);
		return;
	}
	std::unordered_set<std::string> present;
	for (int i = 0; i < ui->initialValuesTable->columnCount(); ++i) {
		present.insert(ui->initialValuesTable->item(0, i)->text().toStdString());
	}
	for (const auto& exp : expFx.variables()) {
		if (present.count(exp) == 0) {
			size_t idx = ui->initialValuesTable->columnCount();
			ui->initialValuesTable->insertColumn(idx);
			ui->initialValuesTable->setItem(0, idx, new QTableWidgetItem(QString::fromStdString(exp)));
		}
	}
	ui->tableFuncs->insertRow(ui->tableFuncs->rowCount());
	auto item = new QTableWidgetItem(fx);
	item->setToolTip(QString::fromStdString(expFx.show()));
	ui->tableFuncs->setItem(ui->tableFuncs->rowCount()-1, 0, item);
	ui->plainTextFunctionEdit->clear();
}

void MainWindow::on_tableFuncs_cellChanged(int row, int col) {
	auto str = ui->tableFuncs->item(row, col)->text().toStdString();
	try {
		auto exp = Expr::parse(str);
		ui->tableFuncs->item(row, col)->setToolTip(QString::fromStdString(exp.show()));
	}
	catch (const ParseError& err) {
		showError(err);
	}
}

void MainWindow::on_solveButton_clicked()
{
	ui->iterationsAns->clear();
	ui->maxDiffAns->clear();
	ui->tableAns->setRowCount(0);
	ui->tableEval->setRowCount(0);

    std::vector<Expr> funcs;
    std::vector<Binding> inits;

    for(int i = 0; i < ui->tableFuncs->rowCount(); i++){
		auto str = ui->tableFuncs->item(i, 0)->text().toStdString();
		try {
			funcs.push_back(Expr::parse(str));
		}
		catch (const ParseError& err) {
			showError(err);
			return;
		}
    }

    for(int i = 0; i < ui->initialValuesTable->columnCount(); i++){
		auto item = ui->initialValuesTable->item(1, i);
		if (item == nullptr) {
			continue;
		}
		double value;
		try {
			value = std::stod(item->text().toStdString());
		}
		catch (const std::exception& err) {
			showError(err);
			return;
		}
		inits.emplace_back(ui->initialValuesTable->item(0, i)->text().toStdString(), value);
    }

    struct Constraints constr;
    constr.min_iters = ui->minIterationsSpinBox->value();
    constr.max_iters = ui->maxIterationsSpinBox->value();;
    constr.abs_epsilon = ui->absSpinBox->value() *
		std::pow(10.0, ui->absMagSpinBox->value());
    constr.rel_epsilon = ui->relSpinBox->value() *
		std::pow(10.0, ui->relMagSpinBox->value());

	Solution solution;
	try {
        solution = solve(funcs, inits, constr);
	}
	catch (const MathError& err) {
		showError(err);
		return;
	}

	ui->iterationsAns->setText(QString::number(solution.iters));
	ui->maxDiffAns->setText(QString::number(solution.max_diff));
	for (const auto& p : solution.vars) {
		int i =  ui->tableAns->rowCount();
		ui->tableAns->insertRow(i);
		ui->tableAns->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p.first)));
		ui->tableAns->setItem(i, 1, new QTableWidgetItem(QString::number(p.second)));
	}
	auto env = Expr::Env(solution.vars.begin(), solution.vars.end());
	for (const auto& f : funcs) {
		int i = ui->tableEval->rowCount();
		ui->tableEval->insertRow(i);
		ui->tableEval->setItem(i, 0, new QTableWidgetItem(QString::number(f.eval(env))));
	}
}

void MainWindow::on_pushButton_clicked()
{
	ui->initialValuesTable->setColumnCount(0);
	ui->tableFuncs->setRowCount(0);
	ui->iterationsAns->clear();
	ui->maxDiffAns->clear();
	ui->tableAns->setRowCount(0);
	ui->tableEval->setRowCount(0);
}
