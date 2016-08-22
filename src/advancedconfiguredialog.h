#ifndef ADVANCEDCONFIGUREDIALOG_H
#define ADVANCEDCONFIGUREDIALOG_H

#include "problem.h"
#include "testcasetable.h"

#include <QDialog>
#include <QListWidget>

namespace Ui
{
class AdvancedConfigureDialog;
}

class AdvancedConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdvancedConfigureDialog(const std::vector<Problem*>& problems, QWidget *parent = 0);
    ~AdvancedConfigureDialog();

    std::vector<Problem*> Problems() const { return problems; }

public slots:
    void accept() override;

private:
    Ui::AdvancedConfigureDialog *ui;
    TestCaseTable* test_case_table;
    Problem* current_problem;
    std::vector<Problem*> old_problems, problems;

    void loadFromProblem(Problem *problem);

private slots:
    void onListWidgetCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onTestCaseSelectionChanged();

    void on_radioButton_internal_clicked();
    void on_radioButton_custom_clicked();
    void on_pushButton_resetSubmit_clicked();
    void on_pushButton_resetRun_clicked();
    void on_pushButton_resetChecker_clicked();
    void on_pushButton_addCompiler_clicked();
    void on_pushButton_removeCompiler_clicked();
    void on_pushButton_addTestCase_clicked();
    void on_pushButton_addSubTestCase_clicked();
    void on_pushButton_removeTestCase_clicked();
    void on_tableWidget_compiler_itemSelectionChanged();
};

#endif // ADVANCEDCONFIGUREDIALOG_H
