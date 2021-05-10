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
    ui->tableWidget_2->insertRow ( ui->tableWidget_2->rowCount() );
    ui->tableWidget_2->setItem   ( ui->tableWidget_2->rowCount()-1, 0, new QTableWidgetItem(fx));
    ui->plainTextFunctionEdit->clear();
}

void MainWindow::on_solveButton_clicked()
{
    std::vector<Expr> funcs;
    std::vector<Binding> inits;

    for(int i = 0; i < ui->tableWidget_2->rowCount(); i++){
		auto str = ui->tableWidget_2->item(i, 0)->text().toStdString();
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
	// ui->Iterations->setStyleSheet("font-weight: bold; color: red");
	// ui->Max_difference->setStyleSheet("font-weight: bold; color: red");
	// ui->maxDiffAns->setStyleSheet("font-weight: bold; color: red");
	// ui->iterationsAns->setStyleSheet("font-weight: bold; color: red");

	ui->tableAns->insertRow ( ui->tableAns->rowCount() );
	ui->tableAns->insertRow ( ui->tableAns->rowCount() );
	for(unsigned int i = 0; i < solution.vars.size(); i++){
		ui->tableAns->insertColumn(i);
		ui->tableAns->setItem(0,i,new QTableWidgetItem(QString::fromStdString(solution.vars[i].first)));
		ui->tableAns->setItem(1,i, new QTableWidgetItem(QString::number(solution.vars[i].second)));
	}

	ui->maxDiffAns->setText(QString::number(solution.max_diff));
	ui->iterationsAns->setText(QString::number(solution.iters));
}

void MainWindow::on_pushButton_clicked()
{
	ui->initialValuesTable->setColumnCount(0);
	ui->tableWidget_2->setRowCount(0);
	ui->iterationsAns->clear();
	ui->maxDiffAns->clear();
	ui->tableAns->setRowCount(0);
	ui->tableAns->setColumnCount(0);
}
